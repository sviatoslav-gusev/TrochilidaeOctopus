#pragma once

#include "server/ClientSession.h"
#include "server/models/ClientsModel.h"

#include <QTcpServer>
#include <QList>

namespace gs::server {

class AppServer final : public QTcpServer {
    Q_OBJECT
public:
    explicit AppServer(QObject* parent = nullptr);
    bool StartServer(uint16_t port);

    models::ClientsModel* model() { return &m_model; }

public:
    Q_INVOKABLE void massStart();
    Q_INVOKABLE void massStop();
    Q_INVOKABLE void addDummyClient(); // For testing Add Client
    Q_INVOKABLE QVariantMap getClientSummaryMetrics(uint32_t client_id);
    Q_INVOKABLE static QString generateToken();
    Q_INVOKABLE void saveClient(const QVariantMap& clientData);
    Q_INVOKABLE void deleteClient(uint32_t client_id);

private:
    // Override QTcpServer's reaction
    void incomingConnection(qintptr socket_descriptor) override;

    void LoadClientsFromDB();

private slots:
    void onSessionClosed(gs::server::ClientSession* session);

private:
    QList<ClientSession*> m_sessions;
    models::ClientsModel m_model;
};

} // namespace gs::server
