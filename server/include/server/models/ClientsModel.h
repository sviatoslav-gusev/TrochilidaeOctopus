#pragma once

#include "shared/Enums.h"
#include "shared/protocol/ClientSettings.h"

#include <QAbstractListModel>
#include <QList>
#include <QString>

namespace gs::server::models {

// Line of table
struct ClientItem {
    uint32_t client_id{0};
    QString client_name;
    QString target;
    QString token;

    enums::ExecMode exec_mode{enums::ExecMode::Demo};
    enums::ConnectionState conn_state{enums::ConnectionState::Offline};
    enums::RunningState running_state{enums::RunningState::Stopped};

    uint32_t ping_timeout_ms{500};
    uint32_t metrics_report_interval_s{10};

    double avg_roundtrip_ms{0.0};
    double avg_jitter_ms{0.0};
    double loss_percentage{0.0};
    uint32_t sent_count{0};

    gs::protocol::ClientSettings ToClientSettings() const;
    void UpdateClientSettings(const gs::protocol::ClientSettings & settings);
};

class ClientsModel : public QAbstractListModel {
    Q_OBJECT
public:
    // Roles - "id"s of cols for QML
    enum ClientRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TargetRole,
        TokenRole,

        ExecModeRole,
        ConnRole,
        RunRole,

        TimeoutRole,
        MetricsIntervalRole,

        RoundtripRole,
        JitterRole,
        LossPercRole,
        SentRole
    };

    explicit ClientsModel(QObject *parent = nullptr);

    // Overridings for QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    std::optional<models::ClientItem> GetClient(uint32_t client_id) const;
    void AddOrUpdateClient(const ClientItem& client);
    void SetConnectionState(uint32_t client_id, gs::enums::ConnectionState conn_state);
    void UpdateMetrics(uint32_t client_id, double roundtrip_ms,
                       double jitter_ms, double loss_percentage, uint32_t sent_count);
    void RemoveClient(uint32_t client_id);
    void MassUpdateRunningState(gs::enums::RunningState state);
private:
    QList<ClientItem> m_clients;
    QHash<uint32_t, int> m_client_id_to_index;

    // Helper to find idx
    int FindClientIndex(uint32_t client_id) const;
};

} // namespace gs::server::models
