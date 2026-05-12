#include "client/AppClient.h"

#include "shared/MessageDispatcher.h"
#include "shared/CryptoHelper.h"

#include "client/engines/DemoPingEngine.h"

namespace gs::client {

AppClient::AppClient(const CredsStorage & credentials, QObject* parent)
    : m_credentials(credentials)
    , QObject(parent)
{
    m_socket = new QTcpSocket(this);

    // Try to reconnect each RECONNECT_INTERVAL ms if no online
    m_reconnect_timer = new QTimer(this);
    m_reconnect_timer->setInterval(RECONNECT_INTERVAL);
    connect(m_reconnect_timer, &QTimer::timeout, this, &AppClient::TryConnect);

    // Heartbeating each HEARTBEAT_INTERVAL ms if online
    m_heartbeat_timer = new QTimer(this);
    m_heartbeat_timer->setInterval(HEARTBEAT_INTERVAL);
    connect(m_heartbeat_timer, &QTimer::timeout, this, &AppClient::SendHeartbeat);

    // Bind socket signals
    connect(m_socket, &QTcpSocket::connected, this, &AppClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &AppClient::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError err)
    {
        qWarning() << "[Network] Socket error:" << m_socket->errorString();
        // This error -> disconnected -> reconnect will run by itself
    });

    m_telemetry_timer = new QTimer(this);
    m_telemetry_timer->setInterval(10000);
    connect(m_telemetry_timer, &QTimer::timeout, this, &AppClient::SendTelemetryReport);

    TryConnect();
}

void AppClient::TryConnect() {
    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        qInfo() << "[Network] Attempting to connect to server...";
        m_socket->connectToHost(ADDRESS, PORT);
    }
}

void AppClient::onConnected() {
    qInfo() << "[Network] TCP Connected! Initializing streamer...";
    m_reconnect_timer->stop(); // Already connected, stop dudos

    // Reset seqnums at each reconnect
    m_seq_nums.Reset();

    // Cleanup previous streamer
    if (m_streamer) {
        m_streamer->deleteLater();
    }
    m_streamer = new gs::network::JsonPacketStreamer(m_socket, this);

    connect(m_streamer, &gs::network::JsonPacketStreamer::jsonReceived,
            this, [this](const QJsonObject& json)
    {
        helpers::MessageDispatcher::Dispatch(json, [this](const traits::WithHeader auto& msg)
        {
            // SeqNum validation for all but Heartbeat
            if (msg.header.type != gs::enums::MsgType::Heartbeat)
            {
                if (msg.header.seq_num != m_seq_nums.expected_rx_seq) {
                    qCritical() << "[Desync] Expected seq:" << m_seq_nums.expected_rx_seq
                                << "but got:" << msg.header.seq_num;
                    m_socket->disconnectFromHost(); // Disconnect!
                    return;
                }
                ++m_seq_nums.expected_rx_seq;
            }

            this->HandleMessage(msg); // Бизнес-логика
        });
    });

    // Start auth
    protocol::LoginRequest req;
    SendMessage(req);
}

void AppClient::onDisconnected() {
    qWarning() << "[Network] Disconnected! Starting reconnect timer...";
    m_heartbeat_timer->stop();
    m_reconnect_timer->start(); // Снова пытаемся пробиться к серверу
}

void AppClient::HandleMessage(const protocol::ChallengeRequest& msg) {
    qDebug() << "[Auth] Received Challenge! Solving...";

    protocol::ChallengeResponse resp;
    resp.hash = gs::helpers::CryptoHelper::SolveChallenge(m_credentials.Token(), msg.nonce);
    SendMessage(resp);
}

void AppClient::HandleMessage(const protocol::LoginResponse& msg) {
    if (msg.conn_state == gs::enums::ConnectionState::Online) {
        qInfo() << "[Auth] SUCCESS! We are online.";
        m_heartbeat_timer->start();
    } else {
        qCritical() << "[Auth] REJECTED by server. Invalid id/token?";
        m_socket->disconnectFromHost();
    }
}

void AppClient::HandleMessage(const protocol::SettingsGetRequest& msg) {
    qInfo() << "[Settings] Server requested settings.";

    protocol::SettingsSetResponse resp;
    resp.client_settings = m_settings;
    SendMessage(resp);
}

void AppClient::HandleMessage(const protocol::SettingsSetRequest& msg) {
    qInfo() << "[Settings] Server updated settings. Target:" << msg.client_settings.ping_target;

    m_settings = msg.client_settings;
    if (m_settings.ping_timeout_ms > 5000) { m_settings.ping_timeout_ms = 5000; }

    // Send actual condition
    protocol::SettingsSetResponse resp;
    resp.client_settings = m_settings;
    SendMessage(resp);
}

void AppClient::ApplySettings() {
    // Server required to stop activity
    if (m_settings.running_state == enums::RunningState::Stopped) {
        if (m_engine) { m_engine->Stop(); }
        m_telemetry_timer->stop();
        return;
    }

    // --- Пересоздаем движок, если сменился режим (Demo / Real) ---
    // Пока у нас есть только DemoPingEngine, но архитектура уже готова для Real
    if (!m_engine || m_settings.exec_mode != m_engine->ExecMode()) {

        if (m_engine) { m_engine->deleteLater(); }

        switch (m_settings.exec_mode) {
        // TODO: Uncomment at RealPingEngine
        // case enums::ExecMode::Ping: {
        //     m_engine = new engines::RealPingEngine(this);
        //     break;
        // }
        case enums::ExecMode::Demo:{
            m_engine = new engines::DemoPingEngine(this);
            break;
        }
        default: {
            qWarning() << "[Engine Factory] Unknown ExecMode! Falling back to Demo.";
            m_engine = new engines::DemoPingEngine(this);
            break;
        }
        }

        connect(m_engine, &engines::AbstractPingEngine::pingResult, this, &AppClient::onPingResult);
    }

    m_engine->SetTarget(m_settings.ping_target);
    m_engine->SetTimeout(m_settings.ping_timeout_ms);

    m_engine->Start();
    m_telemetry_timer->start();
}

void AppClient::SendTelemetryReport() {
    protocol::NetworkMetricsReport report;
    report.metrics = m_current_metrics;
    SendMessage(report);


    // Just some beauty for cli output
    double avg_ping = 0.0;
    double avg_jitter = 0.0;

    if (m_current_metrics.received > 0) {
        avg_ping = m_current_metrics.rcvd_total_ms / m_current_metrics.received;
    }

    if (m_current_metrics.n_jitters > 0) {
        avg_jitter = m_current_metrics.total_jitter_ms / m_current_metrics.n_jitters;
    }

    qInfo().noquote() << QString("[Telemetry] Sent. Pings: %1/%2 | Avg RTT: %3 ms | Jitter: %4 ms")
                             .arg(m_current_metrics.received)
                             .arg(m_current_metrics.sent)
                             .arg(avg_ping, 0, 'f', 1)
                             .arg(avg_jitter, 0, 'f', 1);

    // Reset for next report window
    m_current_metrics = protocol::NetworkMetrics();

    // Still keep m_last_roundtrip_ms to make reports seamless
}

void AppClient::onPingResult(bool success, double roundtrip_ms) {
    ++m_current_metrics.sent;

    if (success) {
        ++m_current_metrics.received;
        m_current_metrics.rcvd_total_ms += roundtrip_ms;

        // Outlier: If ping is greater than 50% from limit
        if (roundtrip_ms > (m_settings.ping_timeout_ms * 0.5)) {
            m_current_metrics.outliers++;
        }

        // Jitter (delta between prev and current roundtrip)
        if (m_last_roundtrip_ms >= 0.0) {
            // Берем модуль разницы между текущим и прошлым пингом
            const double diff = std::abs(roundtrip_ms - m_last_roundtrip_ms);
            m_current_metrics.total_jitter_ms += diff;
            ++m_current_metrics.n_jitters;
        }
        m_last_roundtrip_ms = roundtrip_ms;
    }
    else {
        // It was lost ping, so break the combo for keeping jitter actual
        m_last_roundtrip_ms = -1.0;
    }
}

} // namespace gs::client
