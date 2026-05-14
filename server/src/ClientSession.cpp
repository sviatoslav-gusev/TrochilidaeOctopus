#include "server/ClientSession.h"

namespace gs::server {

ClientSession::ClientSession(QTcpSocket* socket, QObject* parent)
    : QObject(parent), m_socket(socket)
{
    qInfo() << "[Session] New connection from" << m_socket->peerAddress().toString();

    m_streamer = new network::JsonPacketStreamer(m_socket, this);

    connect(m_socket, &QTcpSocket::disconnected, this, &ClientSession::onDisconnected);
    connect(m_streamer, &network::JsonPacketStreamer::jsonReceived,
            this, &ClientSession::ProcessJson);
}

void ClientSession::onDisconnected() {
    qInfo() << "[Session] Client disconnected. ID:" << m_client_id;
    emit sessionClosed(this); // Ask AppServer delete us
}

void ClientSession::ProcessJson(const QJsonObject& json) {
    helpers::MsgDispatcher::Dispatch(json, [this](const traits::WithHeader auto& msg) {

        // Валидация seq_num
        if (msg.header.type != enums::MsgType::Heartbeat)
        {
            if (msg.header.seq_num != m_seq_nums.expected_rx_seq) {
                qCritical() << "[Session] Desync! ID:" << m_client_id
                            << "Expected:" << m_seq_nums.expected_rx_seq
                            << "Got:" << msg.header.seq_num;
                m_socket->disconnectFromHost();
                return;
            }
            m_seq_nums.expected_rx_seq++;
        }

        this->HandleMessage(msg);
    });
}

void ClientSession::HandleMessage(const protocol::LoginRequest& msg) {
    // Todo: such ID should exist
    m_client_id = msg.header.client_id;
    qInfo() << "[Auth] LoginRequest from ID:" << m_client_id;

    // Token shared with client
    // TODO: remove hardcode
    const QString token = "WonderToken";

    // Gen nonce
    const QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);

    m_expected_hash = helpers::CryptoHelper::SolveChallenge(token, nonce);

    protocol::ChallengeRequest challenge;
    challenge.nonce = nonce;
    SendMessage(challenge);
}

void ClientSession::HandleMessage(const protocol::ChallengeResponse& msg) {
    protocol::LoginResponse resp;

    if (msg.hash == m_expected_hash) {
        qInfo() << "[Auth] SUCCESS! Client ID:" << m_client_id << "is Online.";
        m_authorized = true;
        resp.conn_state = enums::ConnectionState::Online;
    } else {
        qCritical() << "[Auth] REJECTED! Client ID:" << m_client_id << "sent wrong hash.";
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
    qInfo().noquote() << QString("[Metrics] ID: %1 | Pings: %2/%3 | RTT: %4 ms | Jitter: %5 ms")
                         .arg(m_client_id)
                         .arg(msg.metrics.received)
                         .arg(msg.metrics.sent)
                         .arg(msg.metrics.received
                              ? msg.metrics.rcvd_total_ms / msg.metrics.received
                              : 0.0, 0, 'f', 1)
                         .arg(msg.metrics.n_jitters
                              ? msg.metrics.total_jitter_ms / msg.metrics.n_jitters
                              : 0.0, 0, 'f', 1);
}

} // namespace gs::server
