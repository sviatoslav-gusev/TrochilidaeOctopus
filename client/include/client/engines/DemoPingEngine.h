#pragma once
#include "./AbstractPingEngine.h"
#include <QTimer>

namespace gs::client::engines {

class DemoPingEngine final : public AbstractPingEngine {
    Q_OBJECT
public:
    explicit DemoPingEngine(QObject* parent = nullptr);

    void SetTarget(const QString& target) override { m_target = target; };
    void SetTimeout(uint32_t timeout_ms) override { m_timeout_ms = timeout_ms; };

    void Start() override;
    void Stop() override;

    enums::ExecMode ExecMode() const override { return enums::ExecMode::Demo; }

private slots:
    void doSimulatePing();

private:
    static constexpr int LOST_CHANCE = 5;

    static constexpr int LAG_CHANCE = 10;
    static constexpr int LAG_MIN = 100;
    static constexpr int LAG_MAX = 300;

    static constexpr int NORM_MIN = 20;
    static constexpr int NORM_MAX = 80;

    QTimer* m_timer;

    QString m_target;
    uint32_t m_timeout_ms{500};
};

} // namespace gs::client::engines
