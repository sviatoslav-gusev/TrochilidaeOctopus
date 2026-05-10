#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QByteArray>

/**     --- SEND & RECEIVE PACKETS --
 *  Logic which connects:
 *      jsons <-> wrapping <-> network
 *
 *  1. SendJson(json) - method to send json
 *  2. jsonReceived(json) - signal about ready json built by slot onReadyRead()
 */

namespace gs::network {

class JsonPacketStreamer : public QObject {
    Q_OBJECT

public:
    explicit JsonPacketStreamer(QTcpSocket* socket, QObject* parent = nullptr)
        : QObject(parent), m_socket(socket)
    {
        // Socket is sender of signal readyRead,
        // JsonPacketStreamer is receiver handling with slot onReadyRead
        connect(m_socket, &QTcpSocket::readyRead, this, &JsonPacketStreamer::onReadyRead);
    }

    void SendJson(const QJsonObject& json);

signals:
    void jsonReceived(const QJsonObject& json);

private slots:
    void onReadyRead();

private:
    QTcpSocket* m_socket;
    QByteArray m_buffer;
    quint32 m_expected_size{0}; // size of awaited JSON
    int m_read_offset{0}; // cursor
};

} // namespace gs::network
