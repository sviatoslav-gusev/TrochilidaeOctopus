#include "client/AppClient.h"

#include "shared/MsgDispatcher.h"
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
    m_reconnect_timer->setInterval(RECONNECT_INTERVAL_MS);
    connect(m_reconnect_timer, &QTimer::timeout, this, &AppClient::TryConnect);

    // Heartbeating each HEARTBEAT_INTERVAL ms if online
    m_heartbeat_timer = new QTimer(this);
    m_heartbeat_timer->setInterval(HEARTBEAT_INTERVAL_MS);
    connect(m_heartbeat_timer, &QTimer::timeout, this, &AppClient::SendHeartbeat);

    // Bind socket signals
    connect(m_socket, &QTcpSocket::connected, this, &AppClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &AppClient::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError err)
    {
        qWarning() << "[Network] Socket error:" << m_socket->errorString();

        if (m_socket->state() == QAbstractSocket::UnconnectedState) {
            if (!m_reconnect_timer->isActive()) {
                m_reconnect_timer->start();
            }
        }
    });

    m_metrics_report_timer = new QTimer(this);
    m_metrics_report_timer->setInterval(FALLBACK_METRICS_REPORT_INTERVAL_MS);
    connect(m_metrics_report_timer, &QTimer::timeout, this, &AppClient::SendMetricsReport);

    m_reconnect_timer->start();
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
        helpers::MsgDispatcher::Dispatch(json, [this](const traits::WithHeader auto& msg)
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

            this->HandleMessage(msg); // Functional logic
        });
    });

    // Start auth
    protocol::LoginRequest req;
    SendMessage(req);
}

void AppClient::onDisconnected() {
    qWarning() << "[Network] Disconnected! Starting reconnect timer...";

    if (m_engine) {
        m_engine->Stop();
        m_engine->deleteLater();
        m_engine = nullptr;
    }

    m_metrics_report_timer->stop();
    m_heartbeat_timer->stop();
    m_reconnect_timer->start(); // Try reconnect again
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
        if (m_settings.running_state == enums::RunningState::Running) {
            qInfo() << "[Auth] RunningState::Running. Exploring metrics...";
        }
        else {
            qInfo() << "[Auth] RunningState::Stopped. "
                       "Waiting for server instructions (SettingsSetRequest)...";
        }
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

    if (m_settings != msg.client_settings) {
        qInfo().noquote() << "[Settings] Server updated settings." << msg.client_settings.ToString();

        m_settings = msg.client_settings;
        if (m_settings.ping_timeout_ms > 5000) { m_settings.ping_timeout_ms = 5000; }

        ApplySettings();
    }

    // Send actual condition
    protocol::SettingsSetResponse resp;
    resp.client_settings = m_settings;
    SendMessage(resp);
}

void AppClient::ApplySettings() {
    qInfo().noquote() << "[Settings] Applying new settings." << m_settings.ToString();

    m_metrics_report_timer->stop();

    // Prepare to use updated logic
    if (m_engine) {
        m_engine->Stop();
        m_engine->deleteLater();
        m_engine = nullptr;
    }

    if (m_settings.running_state == gs::enums::RunningState::Stopped) {
        qInfo() << "[Settings] State is Stopped. Halting engine.";
        return;
    }

    if (!m_engine || m_settings.exec_mode != m_engine->ExecMode())
    {
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

        connect(m_engine, &engines::AbstractPingEngine::pingResult,
                this, &AppClient::onPingResult);
    }

    m_engine->SetTarget(m_settings.target);
    m_engine->SetTimeout(m_settings.ping_timeout_ms);
    m_engine->Start();

    m_metrics_report_timer->setInterval(m_settings.metrics_report_interval_s * 1000);
    m_metrics_report_timer->start();
}

void AppClient::SendMetricsReport() {
    protocol::NetworkMetricsReport report;
    report.metrics = m_current_metrics;
    SendMessage(report);

    // Just some beauty for cli output
    qInfo().noquote() << "[Metrics] Sent." << m_current_metrics.ToString();

    // Reset for next metrics report window
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
