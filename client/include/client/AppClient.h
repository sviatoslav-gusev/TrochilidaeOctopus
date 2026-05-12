#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>

#include "client/CredsStorage.h"
#include "shared/JsonPacketStreamer.h"
#include "shared/JsonSerializer.h"
#include "shared/ConceptTraits.h"
#include "shared/SeqNums.h"
#include "shared/protocol/FunctionalLayer.h"
#include "shared/protocol/SessionLayer.h"
#include "client/engines/AbstractPingEngine.h"

namespace gs::client {

class AppClient : public QObject {
    Q_OBJECT
public:
    explicit AppClient(const CredsStorage & credentials, QObject* parent = nullptr);

private slots:
    void TryConnect();
    void SendTelemetryReport();
    void ApplySettings();

    void onConnected();
    void onDisconnected();
    void onPingResult(bool success, double roundtrip_ms);

private:
    void SendHeartbeat() {
        protocol::Heartbeat hb;
        hb.header.client_id = m_credentials.ClientID();
        hb.header.timestamp = QDateTime::currentMSecsSinceEpoch();
        m_streamer->SendJson(gs::helpers::JsonSerializer::ToJson(hb));
    }

    // Unisersal sender
    template <traits::WithHeader T>
    void SendMessage(T& msg) {
        msg.header.client_id = m_credentials.ClientID();
        msg.header.seq_num = m_seq_nums.tx_seq++;
        msg.header.timestamp = QDateTime::currentMSecsSinceEpoch();
        m_streamer->SendJson(gs::helpers::JsonSerializer::ToJson(msg));
    }

    // ==========================================
    //    --- Packets handling ---
    // ==========================================

    void HandleMessage(const protocol::ChallengeRequest& msg);
    void HandleMessage(const protocol::LoginResponse& msg);
    void HandleMessage(const protocol::SettingsGetRequest& msg);
    void HandleMessage(const protocol::SettingsSetRequest& msg);

    template <typename T>
    void HandleMessage(const T& msg) {
        qWarning() << "Unhandled logical message type!";
    }

private:

    const QString ADDRESS = "127.0.0.1";
    static constexpr int PORT = 12345;
    static constexpr int RECONNECT_INTERVAL = 5000;
    static constexpr int HEARTBEAT_INTERVAL = 2000;

    QTcpSocket* m_socket;
    network::JsonPacketStreamer* m_streamer{nullptr};

    QTimer* m_reconnect_timer;
    QTimer* m_heartbeat_timer;

    const client::CredsStorage & m_credentials;
    protocol::ClientSettings m_settings;

    network::SeqNums m_seq_nums;

    engines::AbstractPingEngine* m_engine{nullptr};
    QTimer* m_telemetry_timer;
    protocol::NetworkMetrics m_current_metrics;
    double m_last_roundtrip_ms{-1.0};
};

} // namespace gs
