#pragma once

#include <QTcpServer>
#include <QList>
#include "server/ClientSession.h"

namespace gs::server {

class AppServer : public QTcpServer {
    Q_OBJECT
public:
    explicit AppServer(QObject* parent = nullptr);
    bool StartServer(uint16_t port);

protected:
    // Override QTcpServer's reaction
    void incomingConnection(qintptr socket_descriptor) override;

private slots:
    void onSessionClosed(ClientSession* session);

private:
    QList<ClientSession*> m_sessions;
};

} // namespace gs::server
