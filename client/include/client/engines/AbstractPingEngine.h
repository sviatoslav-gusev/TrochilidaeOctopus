#pragma once

#include "shared/Enums.h"

#include <QObject>
#include <QString>

namespace gs::client::engines {

class AbstractPingEngine : public QObject {
    Q_OBJECT
public:
    explicit AbstractPingEngine(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~AbstractPingEngine() = default;

    virtual void SetTarget(const QString& target) = 0;
    virtual void SetTimeout(uint32_t timeout_ms) = 0;

    virtual void Start() = 0;
    virtual void Stop() = 0;

    virtual enums::ExecMode ExecMode() const = 0;

signals:
    // Calls after each ping attempt
    void pingResult(bool success, double rtt_ms);
};

} // namespace gs::client::engines
