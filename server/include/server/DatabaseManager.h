#pragma once

#include "server/Enums.h"
#include "shared/Enums.h"

#include <QSqlDatabase>

namespace gs::server {

struct ClientInfo {
    uint32_t client_id;
    QString token;

    QString client_name = "";
    QString target = "";

    enums::ExecMode exec_mode = enums::ExecMode::Demo;
    enums::RunningState run_state = enums::RunningState::Stopped;

    uint32_t ping_timeout_ms = 500;
    uint32_t metrics_report_interval_s = 10;
};

class DatabaseManager {
public:
    // Singletoning
    static DatabaseManager& instance();

    bool OpenDB(const QString& db_name = "server_data.db");
    QList<ClientInfo> GetAllClients();

    QString GetTokenForClient(uint32_t client_id);
    [[deprecated]] bool AddClient(const ClientInfo & client_info);
    uint32_t UpsertClient(const ClientInfo & client_info);
    void DeleteClient(uint32_t client_id);
    [[deprecated]] bool ClearAllData();

    void CompactMetrics();
    bool InsertRawMetrics(uint32_t client_id, enums::ExecMode mode,
                          double roundtrip_ms, double jitter_ms,
                          int sent_count, int received_count);
    QVariantMap GetClientSummaryMetrics(uint32_t client_id);

private:
    void CompactResolutionLevel(enums::MetricsResolution from_res,
                                enums::MetricsResolution to_res,
                                qint64 current_time_s);
    DatabaseManager() = default;

    bool ValidateTables();

private:
    QSqlDatabase m_db;
};

} // namespace gs::server
