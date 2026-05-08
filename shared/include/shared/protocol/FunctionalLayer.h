#pragma once

#include "shared/Enums.h"
#include "shared/protocol/MsgHeader.h"

#include <QJsonObject>
#include <QString>

#include <cstdint>

namespace gs::protocol {

// Client -> Server
struct Log {
    Q_GADGET

    Q_PROPERTY(MsgHeader header MEMBER header)
    Q_PROPERTY(QString   msg    MEMBER msg)

    MsgHeader header{enums::MsgType::Log};
    QString   msg;
};

// Client -> Server
struct NetworkMetrics {
    Q_GADGET

    Q_PROPERTY(MsgHeader header        MEMBER header)
    Q_PROPERTY(uint32_t  sent          MEMBER sent)
    Q_PROPERTY(uint32_t  received      MEMBER received)
    Q_PROPERTY(double    rcvd_total_ms MEMBER rcvd_total_ms)
    Q_PROPERTY(int64_t   outliers      MEMBER outliers)
    Q_PROPERTY(double    jitter        MEMBER jitter)

    MsgHeader header{enums::MsgType::NetworkMetrics};
    uint32_t sent{0};
    uint32_t received{0};
    double received_total_ms{0.0};
    uint32_t outliers{0};
    double jitter{0.0};
};

// Server -> Client
struct StatusGetRequest {
    Q_GADGET

    Q_PROPERTY(MsgHeader header MEMBER header)

    MsgHeader header{enums::MsgType::StatusGetRequest};
};

// Client -> Server
struct StatusGetResponse {
    Q_GADGET

    Q_PROPERTY(MsgHeader            header         MEMBER header)
    Q_PROPERTY(enums::RunningStatus running_status MEMBER running_status)
    Q_PROPERTY(enums::ExecStatus    exec_status    MEMBER exec_status)
    Q_PROPERTY(QString              ping_target    MEMBER ping_target)

    MsgHeader            header{enums::MsgType::StatusGetResponse};
    enums::RunningStatus running_status{enums::RunningStatus::Stopped};
    enums::ExecStatus    exec_status{enums::ExecStatus::Demo};
    QString              ping_target;
};

// Server -> Client
struct StatusSetRequest {
    Q_GADGET

    Q_PROPERTY(MsgHeader            header         MEMBER header)
    Q_PROPERTY(enums::RunningStatus running_status MEMBER running_status)
    Q_PROPERTY(enums::ExecStatus    exec_status    MEMBER exec_status)
    Q_PROPERTY(QString              ping_target    MEMBER ping_target)

    MsgHeader            header{enums::MsgType::StatusSetRequest};
    enums::RunningStatus running_status{enums::RunningStatus::Stopped};
    enums::ExecStatus    exec_status{enums::ExecStatus::Demo};
    QString              ping_target;
};

// Client -> Server
struct StatusSetResponse {
    Q_GADGET

    Q_PROPERTY(MsgHeader            header         MEMBER header)
    Q_PROPERTY(enums::RunningStatus running_status MEMBER running_status)
    Q_PROPERTY(enums::ExecStatus    exec_status    MEMBER exec_status)
    Q_PROPERTY(QString              ping_target    MEMBER ping_target)

    MsgHeader            header{enums::MsgType::StatusSetResponse};
    enums::RunningStatus running_status{enums::RunningStatus::Stopped};
    enums::ExecStatus    exec_status{enums::ExecStatus::Demo};
    QString              ping_target;
};

} // namespace gs::protocol
