#pragma once

#include "shared/Enums.h"

#include <QJsonObject>

namespace gs::protocol {

struct ClientSettings {
    Q_GADGET

public:
    Q_PROPERTY(enums::RunningState running_state   MEMBER running_state)
    Q_PROPERTY(enums::ExecMode     exec_mode       MEMBER exec_mode)
    Q_PROPERTY(QString             ping_target     MEMBER ping_target)
    Q_PROPERTY(uint32_t            ping_timeout_ms MEMBER ping_timeout_ms)

    enums::RunningState running_state{enums::RunningState::Stopped};
    enums::ExecMode     exec_mode{enums::ExecMode::Demo};
    QString             ping_target{"127.0.0.1"};
    uint32_t            ping_timeout_ms{500};

    bool operator==(const ClientSettings&) const = default;
};

} // namespace gs::protocol


