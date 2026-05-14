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
    query.prepare("SELECT token FROM clients WHERE id = :id");
    query.bindValue(":id", client_id);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

bool DatabaseManager::ValidateTables() {
    QSqlQuery query;
    // Clients
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS clients ("
        "id INTEGER PRIMARY KEY, "
        "token TEXT NOT NULL, "
        "name TEXT)"
        );

    // Metrics storage
    ok &= query.exec(
        "CREATE TABLE IF NOT EXISTS metrics ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "client_id INTEGER, "
        "roundtrip_ms REAL, "
        "jitter_ms REAL, "
        "timestamp INTEGER, "

        "FOREIGN KEY(client_id) REFERENCES clients(id))"
        );

    return ok;
}

bool DatabaseManager::AddClient(uint32_t client_id, const QString& token, const QString& name) {
    QSqlQuery query;

    query.prepare("INSERT OR REPLACE INTO clients (id, token, name) VALUES (:id, :token, :name)");
    query.bindValue(":id", client_id);
    query.bindValue(":token", token);
    query.bindValue(":name", name);

    if (!query.exec()) {
        qCritical() << "[DB] Failed to add client:" << query.lastError().text();
        return false;
    }
    qInfo() << "[DB] Client added/updated. ID:" << client_id;
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
