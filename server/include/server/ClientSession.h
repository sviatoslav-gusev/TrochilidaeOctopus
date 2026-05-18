#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QUuid>
#include <QDebug>

#include "shared/JsonPacketStreamer.h"
#include "shared/JsonSerializer.h"
#include "shared/ConceptTraits.h"
#include "shared/SeqNums.h"
#include "shared/protocol/SessionLayer.h"
#include "shared/protocol/FunctionalLayer.h"
#include "server/models/ClientsModel.h"

namespace gs::server {

class ClientSession final : public QObject {
    Q_OBJECT
public:
    explicit ClientSession(QTcpSocket* socket, models::ClientsModel* model, QObject* parent = nullptr);

    uint32_t ClientID() const { return m_client_id; }
    bool IsAuthorized() const { return m_authorized; }

    void PushRunningState(gs::enums::RunningState new_state);
    void PushCurrentSettings();
signals:
    // Signal for AppServer to session removal
    void sessionClosed(gs::server::ClientSession* session);

private slots:
    void onDisconnected();
    void ProcessJson(const QJsonObject& json);

private:
    template <traits::WithHeader T>
    void SendMessage(T& msg) {
        msg.header.client_id = m_client_id;
        msg.header.seq_num = m_seq_nums.tx_seq++;
        msg.header.timestamp = QDateTime::currentMSecsSinceEpoch();
        m_streamer->SendJson(helpers::JsonSerializer::ToJson(msg));
    }

    void HandleMessage(const protocol::LoginRequest& msg);
    void HandleMessage(const protocol::ChallengeResponse& msg);
    void HandleMessage(const protocol::SettingsSetResponse& msg);
    void HandleMessage(const protocol::Heartbeat& msg);
    void HandleMessage(const protocol::NetworkMetricsReport& msg);

    template <typename T>
    void HandleMessage(const T& msg) {
        qWarning() << "[Session] Unhandled message type from Client ID:"
                   << m_client_id;
    }

    QTcpSocket* m_socket;
    network::JsonPacketStreamer* m_streamer;

    models::ClientsModel* m_model;

    uint32_t m_client_id{0};
    bool m_authorized{false};
    QString m_expected_hash; // Challenge answer
    network::SeqNums m_seq_nums;
};

} // namespace gs::server
