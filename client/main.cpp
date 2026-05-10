#include <QCoreApplication>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>

#include "shared/JsonPacketStreamer.h"
#include "shared/MessageDispatcher.h"
#include "shared/protocol/SessionLayer.h"

using namespace gs::protocol;

class AppClient : public QObject {
    Q_OBJECT
public:
    explicit AppClient(QObject* parent = nullptr) : QObject(parent) {
        m_socket = new QTcpSocket(this);
        
        // New TcpConnection: setup streamer and send LoginRequest
        connect(m_socket, &QTcpSocket::connected,
                this, [this]()
        {
            qInfo() << "[Client] Connected to server! Sending LoginRequest...";
            
            m_streamer = new gs::network::JsonPacketStreamer(m_socket, this);
            
            connect(m_streamer, &gs::network::JsonPacketStreamer::jsonReceived,
                    this, [this](const QJsonObject& json)
            {
                gs::helpers::MessageDispatcher::Dispatch(json, [this](const auto& msg)
                {
                    this->HandleMessage(msg);
                });
            });

            // Send login request
            LoginRequest req;
            send(req);
        });

        connect(m_socket, &QTcpSocket::disconnected, this, []()
        {
            qWarning() << "[Client] Disconnected from server.";
            QCoreApplication::quit();
        });

        m_socket->connectToHost("127.0.0.1", 12345);
    }

private:
    template <typename T>
    void send(const T& msg) {
        m_streamer->SendJson(gs::helpers::JsonSerializer::ToJson(msg));
    }

    // --- Some blah-blah logic ---
    void HandleMessage(const ChallengeRequest& msg) {
        qDebug() << "[Client] ChallengeRequest received! Nonce is:" << msg.nonce;
        
        // Run timer to send Heartbeat every 2 sec
        auto* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]()
        {
            Heartbeat hb;
            hb.header.client_id = 42;
            qDebug() << "[Client] Sending Heartbeat...";
            send(hb);
        });
        timer->start(2000);
    }

    template <typename T>
    void HandleMessage(const T& msg) {
        qWarning() << "[Client] Unhandled message type!";
    }

    QTcpSocket* m_socket;
    gs::network::JsonPacketStreamer* m_streamer{nullptr};
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    AppClient client;
    return a.exec();
}

#include "main.moc"
