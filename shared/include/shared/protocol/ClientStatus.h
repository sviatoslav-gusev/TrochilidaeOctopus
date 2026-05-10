#pragma once

#include "shared/Enums.h"

#include <QJsonObject>

namespace gs::protocol {

struct ClientStatus {
    Q_GADGET

public:
    Q_PROPERTY(enums::RunningStatus running_status MEMBER running_status)
    Q_PROPERTY(enums::ExecStatus    exec_status    MEMBER exec_status)
    Q_PROPERTY(QString              ping_target    MEMBER ping_target)

    enums::RunningStatus running_status{enums::RunningStatus::Stopped};
    enums::ExecStatus    exec_status{enums::ExecStatus::Demo};
    QString              ping_target;

    bool operator==(const ClientStatus&) const = default;
};

} // namespace gs::protocol


