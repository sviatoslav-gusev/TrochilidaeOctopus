#include "server/ClientSession.h"
#include "server/DatabaseManager.h"

#include "shared/CryptoHelper.h"
#include "shared/MsgDispatcher.h"

namespace gs::server {

ClientSession::ClientSession(QTcpSocket* socket, models::ClientsModel* model, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_model(model)
{
    qInfo() << "[Session] New connection from" << m_socket->peerAddress().toString();

    m_streamer = new network::JsonPacketStreamer(m_socket, this);

    connect(m_socket, &QTcpSocket::disconnected, this, &ClientSession::onDisconnected);
    connect(m_streamer, &network::JsonPacketStreamer::jsonReceived,
            this, &ClientSession::ProcessJson);
}

void ClientSession::PushRunningState(gs::enums::RunningState new_state) {
    if (!m_authorized) return;

    const std::optional<const models::ClientItem> client_item = m_model->GetClient(m_client_id);
    if (client_item.has_value()) {
        protocol::SettingsSetRequest set_req;
        set_req.client_settings = client_item->ToClientSettings();
        // Set new state!
        set_req.client_settings.running_state = new_state;

        SendMessage(set_req);
        qInfo().nospace()
            << "[Session " << m_client_id << "] Pushed new state" << new_state;
    }
}

void ClientSession::onDisconnected() {
    qInfo() << QString("[Session %1] Client disconnected.").arg(m_client_id);
    emit sessionClosed(this); // Ask AppServer delete us
}

void ClientSession::ProcessJson(const QJsonObject& json) {
    helpers::MsgDispatcher::Dispatch(json, [this](const traits::WithHeader auto& msg) {

        // seq_num validation
        if (msg.header.type != enums::MsgType::Heartbeat)
        {
            if (msg.header.seq_num != m_seq_nums.expected_rx_seq) {
                qCritical() << QString("[Session %1] Desync! "
                                       "Expected: %2; Got: %3")
                                       .arg(m_client_id)
                                       .arg(m_seq_nums.expected_rx_seq)
                                       .arg(msg.header.seq_num);
                m_socket->disconnectFromHost();
                return;
            }
            m_seq_nums.expected_rx_seq++;
        }

        this->HandleMessage(msg);
    });
}

void ClientSession::HandleMessage(const protocol::LoginRequest& msg) {

    const QString token = DatabaseManager::instance().GetTokenForClient(msg.header.client_id);

    if (token.isEmpty()) {
        qWarning() << "[Auth] Drop LoginRequest from unknown ID:" << msg.header.client_id;
        emit sessionClosed(this);
        return;
    }

    // Such ID exists in DB.
    m_client_id = msg.header.client_id;
    qInfo() << "[Auth] LoginRequest from ID:" << m_client_id;

    // Gen nonce
    const QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);

    m_expected_hash = helpers::CryptoHelper::SolveChallenge(token, nonce);

    protocol::ChallengeRequest challenge_request;
    challenge_request.nonce = nonce;
    SendMessage(challenge_request);
}

void ClientSession::HandleMessage(const protocol::SettingsSetResponse& msg)
{
    std::optional<models::ClientItem> client_item = m_model->GetClient(m_client_id);
    if (!client_item.has_value()) {
        qWarning().nospace() << "[Client "<< msg.header.client_id << "] not exists (SettingsSetResponse)";
        return;
    }

    client_item->UpdateClientSettings(msg.client_settings);

    qInfo().nospace()
        << "[Client "<< msg.header.client_id << "] applied new settings. "
        << msg.client_settings.ToString();
}

void ClientSession::HandleMessage(const protocol::ChallengeResponse& msg) {
    protocol::LoginResponse resp;

    if (msg.hash == m_expected_hash) {
        qInfo() << "[Auth] SUCCESS! Client ID:" << m_client_id << "is Online.";
        m_authorized = true;
        resp.conn_state = enums::ConnectionState::Online;

        m_model->SetConnectionState(m_client_id, enums::ConnectionState::Online);

        // Send initial setup for client
        const std::optional<const models::ClientItem> client_item = m_model->GetClient(m_client_id);
        if (client_item.has_value()) {
            protocol::SettingsSetRequest set_req;
            set_req.client_settings = client_item->ToClientSettings();
            SendMessage(set_req);
            qInfo().noquote()
                << QString("[Session %1] Command sent: SettingsSetRequest. %2")
                           .arg(m_client_id)
                           .arg(set_req.client_settings.ToString());
        }
    } else {
        qCritical() << "[Auth] REJECTED! Client ID:" << m_client_id << "sent wrong hash.";
        m_authorized = false;
        resp.conn_state = enums::ConnectionState::Offline;
    }

    SendMessage(resp);

    if (!m_authorized) {
        m_socket->disconnectFromHost(); // kick unauthorized guy
    }
}

void ClientSession::HandleMessage(const protocol::Heartbeat& msg) {
    // TODO: Connection monitoring
    // qDebug() << "Ba-dump... ID:" << m_client_id;
}

void ClientSession::HandleMessage(const protocol::NetworkMetricsReport& msg) {
    qInfo().nospace().noquote()
        << "[Metrics " << m_client_id << "] " << msg.metrics.ToString();
}

} // namespace gs::server
