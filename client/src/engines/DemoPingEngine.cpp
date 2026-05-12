#include "client/engines/DemoPingEngine.h"

#include <QRandomGenerator>
#include <QDebug>

namespace gs::client::engines {

DemoPingEngine::DemoPingEngine(QObject* parent)
    : AbstractPingEngine(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(1000); // Ping 1 time per 1 sec

    connect(m_timer, &QTimer::timeout, this, &DemoPingEngine::doSimulatePing);
}

void DemoPingEngine::Start()
{
    if (!m_target.isEmpty()) {
        m_timer->start();
        qDebug() << "[DemoEngine] Started pinging:" << m_target;
    }
}

void DemoPingEngine::Stop()
{
    m_timer->stop();
    qDebug() << "[DemoEngine] Stopped.";
}

void DemoPingEngine::doSimulatePing()
{
    // Some packets lost with LOST_CHANCE
    bool success = QRandomGenerator::global()->bounded(100) > LOST_CHANCE;
    double roundtrip_ms = 0.0;

    if (success) {
        // Normal ping from NORM_MIN to NORM_MAX ms
        roundtrip_ms = QRandomGenerator::global()->bounded(NORM_MIN, NORM_MAX);

        // Some packets lag with LAG_CHANCE
        if (QRandomGenerator::global()->bounded(100) <= LAG_CHANCE) {
            roundtrip_ms += QRandomGenerator::global()->bounded(LAG_MIN, LAG_MAX);
        }

        // Ping is higher than timeout
        if (roundtrip_ms > m_timeout_ms) {
            success = false;
            roundtrip_ms = 0.0;
        }
    }

    emit pingResult(success, roundtrip_ms);
}

} // namespace gs::client::engines
