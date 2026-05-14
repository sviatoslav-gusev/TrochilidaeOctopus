#pragma once
#include <QSqlDatabase>

namespace gs::server {

class DatabaseManager {
public:
    // Singletoning
    static DatabaseManager& instance();

    bool OpenDB(const QString& db_name = "server_data.db");

    QString GetTokenForClient(uint32_t client_id);
    bool AddClient(uint32_t client_id, const QString& token, const QString& name = "");
    bool ClearAllData();
private:
    DatabaseManager() = default;

    bool ValidateTables();

private:
    QSqlDatabase m_db;
};

} // namespace gs::server
