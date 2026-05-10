#include <QCoreApplication>
#include <QTcpServer>
#include <QDebug>
#include <QTimer>

#include "shared/JsonPacketStreamer.h"
#include "shared/MessageDispatcher.h"
#include "shared/protocol/SessionLayer.h"

using namespace gs::protocol;

// Separate for each client
class ClientSession : public QObject {
    Q_OBJECT
public:
    explicit ClientSession(QTcpSocket* socket, QObject* parent = nullptr)
        : QObject(parent), m_socket(socket) 
    {
        m_streamer = new gs::network::JsonPacketStreamer(socket, this);

        connect(m_streamer, &gs::network::JsonPacketStreamer::jsonReceived,
                this, [this](const QJsonObject& json)
        {
            qDebug().noquote() << "\n[RAW JSON] ->" << QJsonDocument(json).toJson(QJsonDocument::Compact);

            gs::helpers::MessageDispatcher::Dispatch(json, [this](const auto& msg)
            {
                this->HandleMessage(msg);
            });
        });

        connect(socket, &QTcpSocket::disconnected, this, [this]()
        {
            qDebug() << "[Server] Client disconnected.";
            this->deleteLater();
        });
    }

private:
    template <typename T>
    void send(const T& msg) {
        m_streamer->SendJson(gs::helpers::JsonSerializer::ToJson(msg));
    }

    // --- Some blah-blah logic ---
    void HandleMessage(const LoginRequest& msg) {
        qDebug() << "[Server] LoginRequest received! Sending Challenge...";
        
        ChallengeRequest challenge;
        challenge.nonce = "SuperSecretNonce123";
        send(challenge);
    }

    void HandleMessage(const Heartbeat& msg) {
        qDebug() << "[Server] Heartbeat received from client_id:" << msg.header.client_id;
    }

    template <typename T>
    void HandleMessage(const T& msg) {
        qWarning() << "[Server] Unhandled message type!";
    }

    QTcpSocket* m_socket;
    gs::network::JsonPacketStreamer* m_streamer;
};



class AppServer : public QObject {
    Q_OBJECT
public:
    explicit AppServer(QObject* parent = nullptr) : QObject(parent) {
        m_server = new QTcpServer(this);
        connect(m_server, &QTcpServer::newConnection, this, &AppServer::onNewConnection);
        
        if (m_server->listen(QHostAddress::Any, 12345)) {
            qInfo() << "[Server] Listening on port 12345...";
        } else {
            qFatal("[Server] Failed to bind to port.");
        }
    }

private slots:
    void onNewConnection() {
        qDebug() << "[Server] New client connected!";
        QTcpSocket* socket = m_server->nextPendingConnection();
        new ClientSession(socket, this); // Sessio will be self-destroying at disconnect (deleteLater)
    }

private:
    QTcpServer* m_server;
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    AppServer server;
    return a.exec();
}

// CMake workaround for Q_OBJECT in main.cpp
#include "main.moc"
