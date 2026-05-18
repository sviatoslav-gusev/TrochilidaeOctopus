#include "server/AppServer.h"

#include "server/DatabaseManager.h"

#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>

namespace gs::server {

AppServer::AppServer(QObject* parent)
    : QTcpServer(parent)
{
    LoadClientsFromDB();


    QTimer* const compactor_timer = new QTimer(this);
    connect(compactor_timer, &QTimer::timeout, this, []()
    {
        DatabaseManager::instance().CompactMetrics();
        qInfo() << "[DB] Metrics compaction loop executed.";
    });
    compactor_timer->start(30 * 1000);
}

bool AppServer::StartServer(uint16_t port) {
    if (this->listen(QHostAddress::Any, port)) {
        qInfo() << "[AppServer] Listening on port:" << port;
        return true;
    } else {
        qCritical() << "[AppServer] Failed to start:" << this->errorString();
        return false;
    }
}

void AppServer::incomingConnection(qintptr socket_descriptor) {
    // Make raw socket with OS descriptor
    QTcpSocket* const socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socket_descriptor);

    // Wrap to session
    ClientSession* const session = new ClientSession(socket, &m_model, this);
    m_sessions.append(session);

    // Handle session closure: deleting it from list and from memory
    connect(session, &ClientSession::sessionClosed, this, &AppServer::onSessionClosed);
}

void AppServer::LoadClientsFromDB(){
    // 1. Get all client data from DB
    const QList<ClientInfo> clients = DatabaseManager::instance().GetAllClients();

    // 2. Move to the model
    for (const ClientInfo& info : clients)
    {
        models::ClientItem item;
        item.client_id = info.client_id;
        item.client_name = info.client_name;
        item.target = info.target;
        item.token = info.token;

        item.exec_mode = info.exec_mode;
        item.running_state = info.run_state;
        item.conn_state = enums::ConnectionState::Offline;

        item.ping_timeout_ms = info.ping_timeout_ms;
        item.metrics_report_interval_s = info.metrics_report_interval_s;

        m_model.AddOrUpdateClient(item);
    }
}

void AppServer::onSessionClosed(ClientSession* session) {
    m_sessions.removeOne(session);
    session->deleteLater();
    qInfo() << "[AppServer] Session destroyed. Total active:" << m_sessions.size();
}

void AppServer::massStart() {
    qInfo() << "[AppServer] UI command: MASS START triggered!";

    QSqlQuery query;
    query.prepare("UPDATE clients SET run_state = :state");
    query.bindValue(":state", static_cast<int>(gs::enums::RunningState::Running));
    if (!query.exec()) {
        qCritical() << "[DB] Mass start update failed:" << query.lastError().text();
    }

    for (ClientSession* const session : std::as_const(m_sessions)) {
        session->PushRunningState(gs::enums::RunningState::Running);
    }

    m_model.MassUpdateRunningState(gs::enums::RunningState::Running);
}

void AppServer::massStop() {
    qInfo() << "[AppServer] UI command: MASS STOP triggered!";

    // DB
    QSqlQuery query;
    query.prepare("UPDATE clients SET run_state = :state");
    query.bindValue(":state", static_cast<int>(gs::enums::RunningState::Stopped));
    query.exec();

    // Network/client
    for (ClientSession* const session : std::as_const(m_sessions)) {
        session->PushRunningState(gs::enums::RunningState::Stopped);
    }

    // UI
    m_model.MassUpdateRunningState(gs::enums::RunningState::Stopped);
}

void AppServer::addDummyClient() {
    qInfo() << "[AppServer] UI command: Add Client triggered!";
    // Генерим случайного клиента для наглядности в UI
    static uint32_t fake_id = 100;
    models::ClientItem item;
    item.client_id = ++fake_id;
    item.client_name = QString("Agent_%1").arg(fake_id);
    item.target = "8.8.8.8";
    item.conn_state = enums::ConnectionState::Offline;

    m_model.AddOrUpdateClient(item);
}

QVariantMap AppServer::getClientSummaryMetrics(uint32_t client_id) {
    return DatabaseManager::instance().GetClientSummaryMetrics(client_id);
}

QString AppServer::generateToken() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void AppServer::saveClient(const QVariantMap& clientData) {

    // Parse data from QML
    gs::server::ClientInfo info;
    info.client_id = clientData["client_id"].toUInt(); // 0 если новый
    info.client_name = clientData["client_name"].toString();
    info.target = clientData["target"].toString();
    info.token = clientData["token"].toString();
    info.exec_mode = static_cast<gs::enums::ExecMode>(clientData["exec_mode"].toInt());
    info.run_state = static_cast<gs::enums::RunningState>(clientData["run_state"].toInt());
    info.ping_timeout_ms = clientData["ping_timeout_ms"].toUInt();
    info.metrics_report_interval_s = clientData["metrics_report_interval_s"].toUInt();

    // 1. Save to DB. If id == 0, there is new client. UpsertClient handle both
    uint32_t final_id = DatabaseManager::instance().UpsertClient(info);

    // 2. Update model for UI
    models::ClientItem item;
    item.client_id = final_id;
    item.client_name = info.client_name;
    item.target = info.target;
    item.token = info.token;
    item.exec_mode = info.exec_mode;
    item.running_state = info.run_state;
    item.ping_timeout_ms = info.ping_timeout_ms;
    item.metrics_report_interval_s = info.metrics_report_interval_s;

    // Keep metrics if existing
    if (const std::optional<models::ClientItem> existing = m_model.GetClient(final_id))
    {
        item.conn_state = existing->conn_state;
        item.avg_roundtrip_ms = existing->avg_roundtrip_ms;
        item.avg_jitter_ms = existing->avg_jitter_ms;
        item.loss_percentage = existing->loss_percentage;
        item.sent_count = existing->sent_count;

    }

    m_model.AddOrUpdateClient(item);

    // 3. Send fresh setup to client if online
    if (item.conn_state == enums::ConnectionState::Offline) {
        return;
    }

    protocol::SettingsSetRequest set_req;
    set_req.client_settings = item.ToClientSettings();
    for (ClientSession* const session : std::as_const(m_sessions))
    {
        if (session->ClientID() == final_id) {
            session->PushCurrentSettings();
            break;
        }
    }
}

void AppServer::deleteClient(uint32_t client_id) {
    DatabaseManager::instance().DeleteClient(client_id);
    m_model.RemoveClient(client_id);

    for (ClientSession* const session : std::as_const(m_sessions))
    {
        if (session->ClientID() == client_id) {
            onSessionClosed(session);
            break;
        }
    }
}

} // namespace gs::server
