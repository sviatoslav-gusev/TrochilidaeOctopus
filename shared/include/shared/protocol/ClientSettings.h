#pragma once

#include "shared/Enums.h"

#include <QJsonObject>
#include <QString>

namespace gs::protocol {

struct ClientSettings {
    Q_GADGET

public:
    Q_PROPERTY(enums::RunningState running_state             MEMBER running_state)
    Q_PROPERTY(enums::ExecMode     exec_mode                 MEMBER exec_mode)
    Q_PROPERTY(QString             target                    MEMBER target)
    Q_PROPERTY(uint32_t            ping_timeout_ms           MEMBER ping_timeout_ms)
    Q_PROPERTY(uint32_t            metrics_report_interval_s MEMBER metrics_report_interval_s)

    enums::RunningState running_state{enums::RunningState::Stopped};
    enums::ExecMode     exec_mode{enums::ExecMode::Demo};
    QString             target{"127.0.0.1"};
    uint32_t            ping_timeout_ms{500};
    uint32_t            metrics_report_interval_s{10};

    bool operator==(const ClientSettings&) const = default;

    QString ToString() const {
        return QString("Mode: %1 | State: %2 | Ping timeout: %3 ms | Metrics interval: %4 s.")
                       .arg(static_cast<int>(exec_mode))
                       .arg(static_cast<int>(running_state))
                       .arg(ping_timeout_ms)
                       .arg(metrics_report_interval_s);
    }
};

} // namespace gs::protocol


