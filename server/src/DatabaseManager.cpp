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
        "resolution INTEGER DEFAULT 0, " // 0 means MetricsResolution::Raw
        "sent_count INTEGER DEFAULT 1, "
        "received_count INTEGER DEFAULT 1, "
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

uint32_t DatabaseManager::UpsertClient(const ClientInfo & client_info)
{
    QSqlQuery query;

    if (client_info.client_id == 0) {
        // CREATE: Do not set client_id - let db make it via autoincrement
        query.prepare(R"(
            INSERT INTO clients (
                token, client_name, exec_mode, target,
                run_state, ping_timeout_ms, metrics_report_interval_s
            )
            VALUES (
                :token, :client_name, :exec_mode, :target,
                :run_state, :ping_timeout_ms, :metrics_report_interval_s
            )
        )");
    } else {
        // UPDATE: Edit current fields without deletion (there are bindings)
        query.prepare(R"(
            UPDATE clients
            SET token = :token,
                client_name = :client_name,
                exec_mode = :exec_mode,
                target = :target,
                run_state = :run_state,
                ping_timeout_ms = :ping_timeout_ms,
                metrics_report_interval_s = :metrics_report_interval_s
            WHERE client_id = :client_id
        )");
        query.bindValue(":client_id", client_info.client_id);
    }

    // Общие биндинги для обоих запросов
    query.bindValue(":token", client_info.token);
    query.bindValue(":client_name", client_info.client_name);
    query.bindValue(":target", client_info.target);
    query.bindValue(":exec_mode", static_cast<int>(client_info.exec_mode));
    query.bindValue(":run_state", static_cast<int>(client_info.run_state));
    query.bindValue(":ping_timeout_ms", client_info.ping_timeout_ms);
    query.bindValue(":metrics_report_interval_s", client_info.metrics_report_interval_s);

    if (!query.exec()) {
        qCritical() << "[DB] Failed to upsert client:" << query.lastError().text();
        return 0; // Return 0 in case of error
    }

    // Determine used client id
    uint32_t final_id = client_info.client_id;

    if (final_id == 0) {
        // If INSERT - override via lastInsertId
        final_id = query.lastInsertId().toUInt();
        qInfo() << "[DB] New client created. Assigned ID:" << final_id;
    } else {
        qInfo() << "[DB] Client updated. ID:" << final_id;
    }

    return final_id;
}

void DatabaseManager::DeleteClient(uint32_t client_id)
{
    m_db.transaction();

    QSqlQuery query;

    // 1. delete all related metrics
    query.prepare("DELETE FROM metrics WHERE client_id = :client_id");
    query.bindValue(":client_id", client_id);

    if (!query.exec()) {
        qCritical() << "[DB] Cancelled. Failed to delete client metrics:"
                    << query.lastError().text();
        m_db.rollback();
        return;
    }

    // 2. delete client itself
    query.prepare("DELETE FROM clients WHERE client_id = :client_id");
    query.bindValue(":client_id", client_id);

    if (!query.exec()) {
        qCritical() << "[DB] Cancelled. Failed to delete client record:"
                    << query.lastError().text();
        m_db.rollback();
        return;
    }

    // if both commands are okay, finalize transaction
    m_db.commit();

    qInfo() << "[DB] Client and his metrics manually deleted via transaction. ID:"
            << client_id;
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

void DatabaseManager::CompactMetrics() {
    using enum enums::MetricsResolution;

    const qint64 now_s = QDateTime::currentSecsSinceEpoch();

    // Запускаем транзакцию для максимальной скорости записи!
    QSqlDatabase::database().transaction();

    CompactResolutionLevel(Raw, TenMins, now_s);
    CompactResolutionLevel(TenMins, Hour, now_s);
    CompactResolutionLevel(Hour, Day, now_s);
    CompactResolutionLevel(Day, Week, now_s);

    // Подтверждаем транзакцию
    QSqlDatabase::database().commit();
}

void DatabaseManager::CompactResolutionLevel(const enums::MetricsResolution from_res,
                                             const enums::MetricsResolution to_res,
                                             const qint64 current_time_s)
{
    const int from_res_int = static_cast<int>(from_res);
    const int to_res_int = static_cast<int>(to_res);

    const qint64 to_interval = enums::ToIntervalSec(to_res);
    const qint64 current_interval_start_ts_s = (current_time_s / to_interval) * to_interval;

    QSqlQuery query;
    const QString insert_sql = R"(
        INSERT INTO metrics (client_id, exec_mode, roundtrip_ms,
                             jitter_ms, timestamp, resolution,
                             sent_count, received_count)
        SELECT client_id,
               exec_mode,
               SUM(roundtrip_ms * received_count) / SUM(received_count),
               SUM(jitter_ms * received_count) / SUM(received_count),
               (timestamp / :to_interval) * :to_interval,
               :to_res,
               SUM(sent_count),
               SUM(received_count)
        FROM metrics
        WHERE resolution = :from_res AND timestamp < :level_ts_s
        GROUP BY client_id, exec_mode, (timestamp / :to_interval) * :to_interval
    )";
    query.prepare(insert_sql);
    query.bindValue(":to_interval", to_interval);
    query.bindValue(":to_res", to_res_int);
    query.bindValue(":from_res", from_res_int);
    query.bindValue(":level_ts_s", current_interval_start_ts_s);

    if (!query.exec()) {
        qCritical() << "[DB] Failed to insert compacted metrics ("
                    << from_res_int << "->" << to_res_int << "):" << query.lastError().text();
        return;
    }

    //  INSERT successful -> remove sources
    const QString delete_sql = R"(
        DELETE FROM metrics
        WHERE resolution = :from_res AND timestamp < :level_ts_s
    )";

    query.prepare(delete_sql);
    query.bindValue(":from_res", from_res_int);
    query.bindValue(":level_ts_s", current_interval_start_ts_s);

    if (!query.exec()) {
        qCritical() << "[DB] Failed to delete old metrics during compaction:"
                    << query.lastError().text();
    }
}

bool DatabaseManager::InsertRawMetrics(uint32_t client_id, enums::ExecMode mode,
                                       double roundtrip_ms, double jitter_ms,
                                       int sent_count, int received_count)
{
    QSqlQuery query;
    // resolution = 0 (Raw packet of measurements)
    const QString insert_sql = R"(
        INSERT INTO metrics (client_id, exec_mode,
                             roundtrip_ms, jitter_ms, timestamp,
                             resolution,
                             sent_count, received_count)
        VALUES (:client_id, :exec_mode,
                :roundtrip_ms, :jitter_ms, :ts,
                0,
                :sent_count, :received_count)
    )";

    query.prepare(insert_sql);
    query.bindValue(":client_id", client_id);
    query.bindValue(":exec_mode", static_cast<int>(mode));
    query.bindValue(":roundtrip_ms", roundtrip_ms);
    query.bindValue(":jitter_ms", jitter_ms);
    query.bindValue(":ts", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":sent_count", sent_count);
    query.bindValue(":received_count", received_count);

    if (!query.exec()) {
        qCritical() << "[DB] Failed to insert raw metrics:"
                    << query.lastError().text();
        return false;
    }
    return true;
}

QVariantMap DatabaseManager::GetClientSummaryMetrics(uint32_t client_id) {
    QVariantMap result;
    QSqlQuery query;

    // Desired resolutions (excepting Raw)
    const QList<std::pair<QString, int>> resolutions = {
        {"10mins", static_cast<int>(enums::MetricsResolution::TenMins)},
        {"hour", static_cast<int>(enums::MetricsResolution::Hour)},
        {"day", static_cast<int>(enums::MetricsResolution::Day)},
        {"week", static_cast<int>(enums::MetricsResolution::Week)}
    };

    for (const auto& [key, res_enum] : resolutions) {
        // Get last aggregated block
        query.prepare(R"(
            SELECT roundtrip_ms, jitter_ms, sent_count, received_count
            FROM metrics
            WHERE client_id = :client_id AND resolution = :res
            ORDER BY timestamp DESC LIMIT 1
        )");
        query.bindValue(":client_id", client_id);
        query.bindValue(":res", res_enum);

        if (query.exec() && query.next()) {
            QVariantMap res_data;
            res_data["rtt"] = query.value(0).toDouble();
            res_data["jitter"] = query.value(1).toDouble();

            const int sent = query.value(2).toInt();
            const int recv = query.value(3).toInt();

            double loss = 0.0;
            if (sent > 0) {
                loss = (static_cast<double>(sent - recv) / sent) * 100.0;
            }
            res_data["sent"] = sent;
            res_data["loss"] = loss;

            result[key] = res_data;
        } else {
            // If no data
            QVariantMap empty;
            empty["rtt"] = 0.0;
            empty["jitter"] = 0.0;
            empty["sent"] = 0;
            empty["loss"] = 0.0;

            result[key] = empty;
        }
    }
    return result;
}

} // namespace gs::server
