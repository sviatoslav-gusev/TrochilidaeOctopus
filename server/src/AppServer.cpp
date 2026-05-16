#include "server/AppServer.h"

#include "server/DatabaseManager.h"

namespace gs::server {

AppServer::AppServer(QObject* parent)
    : QTcpServer(parent)
{
    LoadClientsFromDB();
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
    for (ClientSession* const session : std::as_const(m_sessions)) {
        session->PushRunningState(gs::enums::RunningState::Running);
    }
}

void AppServer::massStop() {
    qInfo() << "[AppServer] UI command: MASS STOP triggered!";
    for (ClientSession* const session : std::as_const(m_sessions)) {
        session->PushRunningState(gs::enums::RunningState::Stopped);
    }
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

} // namespace gs::server
