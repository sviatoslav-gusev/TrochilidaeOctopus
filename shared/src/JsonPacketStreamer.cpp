#include "shared/JsonPacketStreamer.h"

#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QtEndian>

namespace gs::network {

void JsonPacketStreamer::SendJson(const QJsonObject& json)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState)
    {
        qWarning() << "Error: socket is not ready to send Json";
        return;
    }

    // Pack Json
    const QByteArray json_data = QJsonDocument(json).toJson(QJsonDocument::Compact);

    // Size of packed Json
    const quint32 json_data_size = qToBigEndian<quint32>(static_cast<quint32>(json_data.size()));

    // Send
    m_socket->write(reinterpret_cast<const char*>(&json_data_size), sizeof(json_data_size));
    m_socket->write(json_data);
}

void JsonPacketStreamer::onReadyRead()
{
    static constexpr quint32 MAX_PACKET_SIZE = 1024 * 1024; // 1MB max

    m_buffer.append(m_socket->readAll());

    while (true)
    {
        // Currently unreaded
        int unread_size = m_buffer.size() - m_read_offset;

        // 1. Reading packet size
        if (m_expected_size == 0) {
            if (unread_size < sizeof(quint32)) {
                break; // Awaiting more bytes
            }

            m_expected_size = qFromBigEndian<quint32>(m_buffer.constData() + m_read_offset);
            m_read_offset += sizeof(quint32);
            unread_size -= sizeof(quint32);

            if (m_expected_size == 0) {
                qWarning() << "Error: Zero packet size? Wtf?? Skip packet.";
                continue;
            }

            // Flood (and total_packet_size overflow) protection
            if (m_expected_size > MAX_PACKET_SIZE) {
                qWarning() << "Critical: Packet size exceeds limit:" << m_expected_size;
                m_buffer.clear(); // Drop that buffer
                m_expected_size = 0;
                m_read_offset = 0;
                m_socket->disconnectFromHost(); // Disconnect
                return;
            }
        }


        if (unread_size < m_expected_size) {
            // There is no any of some full packet. We need another data update.
            break;
        }

        // Skip size_prefix, get packed Json
        const QByteArray jsonData = m_buffer.mid(m_read_offset, m_expected_size);
        m_read_offset += m_expected_size;
        // unread_size -= m_expected_size
        m_expected_size = 0; // Ready to next packet

        // Parse
        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
        if (!doc.isObject()) {
            qWarning() << "Error: Cannot unpack json. Skip that packet with size" << jsonData.size();
        }
        else {
            emit jsonReceived(doc.object());
        }
    }


    //     --- GARBAGE CLEANING ---
    // Scenario 1: We read buffer completely. Clean it.
    if (m_read_offset == m_buffer.size()) {
        m_buffer.clear();
        m_read_offset = 0;
    }
    // Scenario 2: We have lot of grbage. Remove used part.
    else if (m_read_offset > MAX_PACKET_SIZE) {
        m_buffer.remove(0, m_read_offset);
        m_read_offset = 0;
    }
}

} // namespace gs::network
