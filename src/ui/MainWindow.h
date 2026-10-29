#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include <QTimer>
#include <QThread>
#include <memory>
#include <map>

#include "../core/SettingsManager.h"
#include "../core/SystemMonitor.h"
#include "../core/ProcessMonitor.h"
#include "../core/TaskDetector.h"
#include "../core/RecommendationEngine.h"
#include "../core/SystemOptimizer.h"
#include "../core/PerformanceAnalyzer.h"
#include "../core/MonitoringWorker.h"
#include "../core/strategies/OptimizationStrategy.h"
#include "pages/DashboardPage.h"
#include "pages/ProcessesPage.h"
#include "pages/ProfilesPage.h"
#include "pages/RecommendationsPage.h"
#include "pages/AnalysisPage.h"
#include "pages/SettingsPage.h"

namespace pulse::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<SettingsManager> settings, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSidebarButtonClicked(int id);
    void onSamplingTimerFired();
    void onMonitoringDataReady(
        const pulse::SystemMetrics& sysMetrics,
        const std::vector<pulse::ProcessInfo>& processes,
        const pulse::TaskDetectionResult& detection,
        const std::vector<pulse::Recommendation>& recommendations);
    void onProfileChanged(WorkloadProfile profile);
    void onActionTriggered(const Recommendation& rec);
    void onMonitoringIntervalChanged(uint32_t intervalMs);
    void onMonitoringToggled(bool enabled);
    void onRollbackRequested();

private:
    void setupUi();
    QWidget* createSidebar();
    QWidget* createTopBar();
    void updateHeaderPills(const SystemMetrics& metrics);
    void initStrategies();

    std::shared_ptr<SettingsManager> m_settings;
    std::shared_ptr<SystemMonitor> m_systemMonitor;
    std::shared_ptr<ProcessMonitor> m_processMonitor;
    std::shared_ptr<SystemOptimizer> m_optimizer;
    std::shared_ptr<PerformanceAnalyzer> m_analyzer;
    std::shared_ptr<TaskDetector> m_taskDetector;
    std::shared_ptr<RecommendationEngine> m_recEngine;

    // Strategies
    std::map<WorkloadProfile, std::shared_ptr<OptimizationStrategy>> m_strategies;
    std::shared_ptr<OptimizationStrategy> m_activeStrategy;

    // Background worker thread for real-time Windows monitoring
    QThread* m_workerThread{nullptr};
    MonitoringWorker* m_worker{nullptr};

    QStackedWidget* m_stackedWidget{nullptr};
    QButtonGroup* m_sidebarGroup{nullptr};

    QLabel* m_statusPill{nullptr};
    QLabel* m_cpuQuickPill{nullptr};
    QLabel* m_ramQuickPill{nullptr};
    QLabel* m_activeProfilePill{nullptr};
    QPushButton* m_rollbackTopBtn{nullptr};

    // Pages
    DashboardPage* m_dashboardPage{nullptr};
    ProcessesPage* m_processesPage{nullptr};
    ProfilesPage* m_profilesPage{nullptr};
    RecommendationsPage* m_recommendationsPage{nullptr};
    AnalysisPage* m_analysisPage{nullptr};
    SettingsPage* m_settingsPage{nullptr};
};

} // namespace pulse::ui
