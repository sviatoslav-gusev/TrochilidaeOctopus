#pragma once

#include "shared/Enums.h"
#include "shared/protocol/MsgHeader.h"
#include "shared/protocol/ClientStatus.h"
#include "shared/protocol/NetworkMetrics.h"

#include <QJsonObject>
#include <QString>

namespace gs::protocol {

// Client -> Server
struct Log {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::Log;

    Q_PROPERTY(MsgHeader header MEMBER header)
    Q_PROPERTY(QString   msg    MEMBER msg)

    MsgHeader header{enums::MsgType::Log};
    QString   msg;
};

// Client -> Server
struct NetworkMetricsReport {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::NetworkMetricsReport;

    Q_PROPERTY(MsgHeader header        MEMBER header)
    Q_PROPERTY(NetworkMetrics metrics MEMBER metrics)

    MsgHeader      header{enums::MsgType::NetworkMetricsReport};
    NetworkMetrics metrics;
};

// Server -> Client
struct StatusGetRequest {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::StatusGetRequest;

    Q_PROPERTY(MsgHeader header MEMBER header)

    MsgHeader header{enums::MsgType::StatusGetRequest};
};

// Client -> Server
struct StatusGetResponse {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::StatusGetResponse;

    Q_PROPERTY(MsgHeader      header         MEMBER header)
    Q_PROPERTY(ClientStatus   client_status  MEMBER client_status)

    MsgHeader     header{enums::MsgType::StatusGetResponse};
    ClientStatus  client_status;
};

// Server -> Client
struct StatusSetRequest {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::StatusSetRequest;

    Q_PROPERTY(MsgHeader      header         MEMBER header)
    Q_PROPERTY(ClientStatus   client_status  MEMBER client_status)

    MsgHeader     header{enums::MsgType::StatusSetRequest};
    ClientStatus  client_status;
};

// Client -> Server
struct StatusSetResponse {
    Q_GADGET
public:
    static constexpr enums::MsgType TYPE_ID = enums::MsgType::StatusSetResponse;

    Q_PROPERTY(MsgHeader            header         MEMBER header)
    Q_PROPERTY(ClientStatus   client_status  MEMBER client_status)

    MsgHeader     header{enums::MsgType::StatusSetResponse};
    ClientStatus  client_status;
};

} // namespace gs::protocol
