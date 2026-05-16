#include "server/DatabaseManager.h"

#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>

namespace gs::server {

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

bool DatabaseManager::OpenDB(const QString& db_name) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // AppData path for
    const QString app_data_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir dir(app_data_path);
    if (!dir.exists()) {
        dir.mkpath("."); // Create all cascade if not yet
    }

    const QString full_db_path = dir.filePath(db_name);
    m_db.setDatabaseName(full_db_path);

    if (!m_db.open()) {
        qCritical() << "[DBManager] Cannot open database:" << m_db.lastError().text();
        return false;
    }

    return ValidateTables();
}

QString DatabaseManager::GetTokenForClient(uint32_t client_id) {
    QSqlQuery query;
    query.prepare("SELECT token FROM clients WHERE client_id = :client_id");
    query.bindValue(":client_id", client_id);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

QList<ClientInfo> DatabaseManager::GetAllClients() {
    QList<ClientInfo> result;
    QSqlQuery query("SELECT client_id, token, "
                           "client_name, target, "
                           "exec_mode, run_state, "
                           "ping_timeout_ms, metrics_report_interval_s "
                    "FROM clients");

    // Run across all lines in DB
    while (query.next()) {
        ClientInfo info;
        // query.value(0) means first col from SELECT (client_id)
        info.client_id = query.value(0).toUInt();
        info.token = query.value(1).toString();
        info.client_name = query.value(2).toString();
        info.target = query.value(3).toString();

        info.exec_mode = static_cast<enums::ExecMode>(query.value(4).toInt()),
        info.run_state = static_cast<enums::RunningState>(query.value(5).toInt());

        info.ping_timeout_ms = query.value(6).toUInt();
        info.metrics_report_interval_s = query.value(7).toUInt();

        result.append(info);
    }

    if (query.lastError().isValid()) {
        qWarning() << "[DB] Error fetching clients:" << query.lastError().text();
    }

    return result;
}

bool DatabaseManager::ValidateTables() {
    QSqlQuery query;
    // Clients
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS clients ("
        "client_id INTEGER PRIMARY KEY, "
        "token TEXT NOT NULL, "
        "client_name TEXT NOT NULL, "
        "exec_mode INTEGER DEFAULT 0, " // 0 means ExecMode::Demo
        "target TEXT, "
        "run_state INTEGER DEFAULT 0, " // 0 means RunningState::Stopped
        "ping_timeout_ms INTEGER DEFAULT 500, "
        "metrics_report_interval_s INTEGER DEFAULT 10)"
        );

    // Metrics storage
    ok &= query.exec(
        "CREATE TABLE IF NOT EXISTS metrics ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "client_id INTEGER, "
        "exec_mode INTEGER, "
        "roundtrip_ms REAL, "
        "jitter_ms REAL, "
        "timestamp INTEGER, "

        "FOREIGN KEY(client_id) REFERENCES clients(client_id))"
        );

    return ok;
}

bool DatabaseManager::AddClient(const ClientInfo & client_info)
{
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO "
                  "clients (client_id, token, client_name, "
                           "exec_mode, target, run_state, "
                           "ping_timeout_ms, metrics_report_interval_s) "
                  "VALUES (:client_id, :token, :client_name, "
                          ":exec_mode, :target, :run_state, "
                          ":ping_timeout_ms, :metrics_report_interval_s)");
    query.bindValue(":client_id", client_info.client_id);
    query.bindValue(":token", client_info.token);
    query.bindValue(":client_name", client_info.client_name);
    query.bindValue(":target", client_info.target);

    query.bindValue(":exec_mode", static_cast<int>(client_info.exec_mode));
    query.bindValue(":run_state", static_cast<int>(client_info.run_state));

    query.bindValue(":ping_timeout_ms", client_info.ping_timeout_ms);
    query.bindValue(":metrics_report_interval_s", client_info.metrics_report_interval_s);

    if (!query.exec()) {
        qCritical() << "[DB] Failed to add client:" << query.lastError().text();
        return false;
    }
    qInfo() << "[DB] Client added/updated. ID:" << client_info.client_id;
    return true;
}

bool DatabaseManager::ClearAllData() {
    QSqlQuery query;
    bool ok = true;

    // Remove all from tables, tables exists.
    ok &= query.exec("DELETE FROM metrics");
    ok &= query.exec("DELETE FROM clients");

    if (ok) {
        qInfo() << "[DB] All data cleared successfully.";
    } else {
        qCritical() << "[DB] Failed to clear data:" << query.lastError().text();
    }
    return ok;
}

} // namespace gs::server
