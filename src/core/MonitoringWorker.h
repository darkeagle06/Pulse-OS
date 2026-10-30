#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include <atomic>
#include <mutex>
#include "Types.h"
#include "SystemMonitor.h"
#include "ProcessMonitor.h"
#include "TaskDetector.h"
#include "RecommendationEngine.h"
#include "SmartAlertEngine.h"
#include "strategies/OptimizationStrategy.h"

namespace pulse {

class MonitoringWorker : public QObject {
    Q_OBJECT

public:
    MonitoringWorker(std::shared_ptr<SystemMonitor> sysMonitor,
                     std::shared_ptr<ProcessMonitor> procMonitor,
                     std::shared_ptr<TaskDetector> taskDetector,
                     std::shared_ptr<RecommendationEngine> recEngine,
                     std::shared_ptr<OptimizationStrategy> initialStrategy,
                     std::shared_ptr<SmartAlertEngine> alertEngine = nullptr,
                     QObject* parent = nullptr);
    ~MonitoringWorker() override;

public slots:
    void startMonitoring(uint32_t intervalMs);
    void stopMonitoring();
    void setInterval(uint32_t intervalMs);
    void setActiveStrategy(std::shared_ptr<pulse::OptimizationStrategy> strategy);
    void setAlertEngine(std::shared_ptr<pulse::SmartAlertEngine> alertEngine);
    void triggerImmediateSample();

private slots:
    void onTimerTimeout();

signals:
    void dataSampled(const pulse::SystemMetrics& sysMetrics,
                     const std::vector<pulse::ProcessInfo>& processes,
                     const pulse::TaskDetectionResult& detection,
                     const std::vector<pulse::Recommendation>& recommendations);
    void alertsEvaluated(const std::vector<pulse::ResourceAlert>& activeAlerts,
                         const std::vector<pulse::ResourceAlert>& newAlerts);
    void monitoringError(const QString& errorMessage);

private:
    void performSample();

    std::shared_ptr<SystemMonitor> m_sysMonitor;
    std::shared_ptr<ProcessMonitor> m_procMonitor;
    std::shared_ptr<TaskDetector> m_taskDetector;
    std::shared_ptr<RecommendationEngine> m_recEngine;
    std::shared_ptr<SmartAlertEngine> m_alertEngine;

    std::mutex m_strategyMutex;
    std::shared_ptr<OptimizationStrategy> m_activeStrategy;

    QTimer* m_timer{nullptr};
    uint32_t m_intervalMs{1000};
    std::atomic<bool> m_isSampling{false};
};

} // namespace pulse

