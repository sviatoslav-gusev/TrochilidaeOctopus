#pragma once

#include <QJsonObject>
#include <QString>

#include <cstdint>

namespace gs::protocol {

struct NetworkMetrics {
    Q_GADGET

public:
    Q_PROPERTY(uint32_t  sent            MEMBER sent)
    Q_PROPERTY(uint32_t  received        MEMBER received)
    Q_PROPERTY(int64_t   outliers        MEMBER outliers)
    Q_PROPERTY(double    rcvd_total_ms   MEMBER rcvd_total_ms)
    Q_PROPERTY(double    total_jitter_ms MEMBER total_jitter_ms)
    Q_PROPERTY(uint32_t  n_jitters       MEMBER n_jitters)


    uint32_t sent{0};
    uint32_t received{0};
    uint32_t outliers{0};
    double   rcvd_total_ms{0.0};
    double   total_jitter_ms{0.0};
    uint32_t n_jitters{0};

    bool operator==(const NetworkMetrics&) const = default;

    QString ToString() const {
        return QString("Pings: %1/%2 | RTT: %3 ms | Jitter: %4 ms")
                       .arg(received)
                       .arg(sent)
                       .arg(received
                            ? rcvd_total_ms / received
                            : 0.0, 0, 'f', 1)
                       .arg(n_jitters
                            ? total_jitter_ms / n_jitters
                            : 0.0, 0, 'f', 1);
    }
};

} // namespace gs::protocol


