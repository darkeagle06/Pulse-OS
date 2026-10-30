#include "MonitoringWorker.h"
#include "DiagnosticLog.h"
#include <chrono>

namespace pulse {

MonitoringWorker::MonitoringWorker(
    std::shared_ptr<SystemMonitor> sysMonitor,
    std::shared_ptr<ProcessMonitor> procMonitor,
    std::shared_ptr<TaskDetector> taskDetector,
    std::shared_ptr<RecommendationEngine> recEngine,
    std::shared_ptr<OptimizationStrategy> initialStrategy,
    std::shared_ptr<SmartAlertEngine> alertEngine,
    QObject* parent)
    : QObject(parent),
      m_sysMonitor(std::move(sysMonitor)),
      m_procMonitor(std::move(procMonitor)),
      m_taskDetector(std::move(taskDetector)),
      m_recEngine(std::move(recEngine)),
      m_alertEngine(std::move(alertEngine)),
      m_activeStrategy(std::move(initialStrategy)) {
    logDiagnostic("[MonitoringWorker] Initialized on worker thread.");
}

MonitoringWorker::~MonitoringWorker() {
    logDiagnostic("[MonitoringWorker] Destroying worker...");
    if (m_timer) {
        m_timer->stop();
    }
}

void MonitoringWorker::startMonitoring(uint32_t intervalMs) {
    m_intervalMs = intervalMs;
    logDiagnostic("[MonitoringWorker] Starting monitoring with interval " + std::to_string(intervalMs) + " ms.");

    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &MonitoringWorker::onTimerTimeout);
    }

    m_timer->start(static_cast<int>(m_intervalMs));
    // Trigger immediate first sample
    performSample();
}

void MonitoringWorker::stopMonitoring() {
    logDiagnostic("[MonitoringWorker] Stopping monitoring timer.");
    if (m_timer) {
        m_timer->stop();
    }
}

void MonitoringWorker::setInterval(uint32_t intervalMs) {
    m_intervalMs = intervalMs;
    logDiagnostic("[MonitoringWorker] Updating interval to " + std::to_string(intervalMs) + " ms.");
    if (m_timer && m_timer->isActive()) {
        m_timer->setInterval(static_cast<int>(m_intervalMs));
    }
}

void MonitoringWorker::setActiveStrategy(std::shared_ptr<pulse::OptimizationStrategy> strategy) {
    std::string stratName = strategy ? strategy->getProfileName() : "None";
    logDiagnostic("[MonitoringWorker] Setting active strategy to: " + stratName);
    std::lock_guard<std::mutex> lock(m_strategyMutex);
    m_activeStrategy = std::move(strategy);
}

void MonitoringWorker::setAlertEngine(std::shared_ptr<pulse::SmartAlertEngine> alertEngine) {
    m_alertEngine = std::move(alertEngine);
}

void MonitoringWorker::triggerImmediateSample() {
    performSample();
}

void MonitoringWorker::onTimerTimeout() {
    performSample();
}

void MonitoringWorker::performSample() {
    // Prevent overlapping samples if background operations take longer than interval
    bool expected = false;
    if (!m_isSampling.compare_exchange_strong(expected, true)) {
        logDiagnostic("[MonitoringWorker] Skipping tick: previous sampling cycle is still executing.");
        return;
    }

    auto tStart = std::chrono::steady_clock::now();

    try {
        SystemMetrics sysMetrics;
        if (m_sysMonitor) {
            sysMetrics = m_sysMonitor->sample();
        }

        std::vector<ProcessInfo> processes;
        if (m_procMonitor) {
            processes = m_procMonitor->sampleProcesses();
        }

        TaskDetectionResult detection;
        if (m_taskDetector) {
            detection = m_taskDetector->detect();
        }

        std::shared_ptr<OptimizationStrategy> strat;
        {
            std::lock_guard<std::mutex> lock(m_strategyMutex);
            strat = m_activeStrategy;
        }

        std::vector<Recommendation> recs;
        if (m_recEngine) {
            recs = m_recEngine->generateRecommendations(sysMetrics, processes, strat);
        }

        std::vector<ResourceAlert> activeAlerts;
        std::vector<ResourceAlert> newAlerts;
        if (m_alertEngine) {
            auto alertPair = m_alertEngine->evaluate(sysMetrics, processes, detection);
            activeAlerts = std::move(alertPair.first);
            newAlerts = std::move(alertPair.second);
        }

        auto tEnd = std::chrono::steady_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

        logDiagnostic("[MonitoringWorker] Sample completed in " + std::to_string(durationMs) +
                      " ms (" + std::to_string(processes.size()) + " processes, " +
                      std::to_string(recs.size()) + " recs, " +
                      std::to_string(activeAlerts.size()) + " active alerts). Emitting...");

        m_isSampling.store(false);
        emit dataSampled(sysMetrics, processes, detection, recs);
        if (m_alertEngine) {
            emit alertsEvaluated(activeAlerts, newAlerts);
        }

    } catch (const std::exception& ex) {
        m_isSampling.store(false);
        logDiagnostic(std::string("[MonitoringWorker] Exception during sampling: ") + ex.what());
        emit monitoringError(QString::fromUtf8(ex.what()));
    } catch (...) {
        m_isSampling.store(false);
        logDiagnostic("[MonitoringWorker] Unknown exception during sampling.");
        emit monitoringError("Unknown exception occurred during monitoring.");
    }
}

} // namespace pulse

