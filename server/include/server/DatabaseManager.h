#pragma once
#include <QSqlDatabase>
#include "shared/Enums.h"

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
    bool AddClient(const ClientInfo & client_info);
    bool ClearAllData();
private:
    DatabaseManager() = default;

    bool ValidateTables();

private:
    QSqlDatabase m_db;
};

} // namespace gs::server
