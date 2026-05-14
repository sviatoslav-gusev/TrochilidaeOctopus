#include "server/AppServer.h"

namespace gs::server {

AppServer::AppServer(QObject* parent)
    : QTcpServer(parent)
{}

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
    ClientSession* const session = new ClientSession(socket, this);
    m_sessions.append(session);

    // Handle session closure: deleting it from list and from memory
    connect(session, &ClientSession::sessionClosed, this, &AppServer::onSessionClosed);
}

void AppServer::onSessionClosed(ClientSession* session) {
    m_sessions.removeOne(session);
    session->deleteLater();
    qInfo() << "[AppServer] Session destroyed. Total active:" << m_sessions.size();
}

} // namespace gs::server
