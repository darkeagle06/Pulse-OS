#pragma once

#include <QWidget>
#include <QLabel>
#include <vector>

#include "../../core/Types.h"
#include "../../core/ISystemMonitor.h"
#include "../../core/IProcessMonitor.h"
#include "../components/MetricCard.h"
#include "../components/PerformanceChart.h"
#include "../components/ProcessTableView.h"

namespace pulse::ui {

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget* parent = nullptr);
    ~DashboardPage() override = default;

    void updateMetrics(const SystemMetrics& metrics,
                       const std::vector<double>& cpuHistory,
                       const std::vector<double>& ramHistory,
                       const std::vector<ProcessInfo>& topProcesses);
    void updateAlertStatus(const std::vector<ResourceAlert>& activeAlerts);
    void updateInstalledAppsCount(size_t totalApps, size_t unusedApps);

signals:
    void navigateToAlertsRequested();
    void navigateToAppsRequested();

private:
    void setupUi();
    static QString formatUptime(uint64_t seconds);

    // Alert Banner Strip
    QFrame* m_alertBannerFrame{nullptr};
    QLabel* m_alertBannerText{nullptr};
    QLabel* m_alertCountBadge{nullptr};

    // Cards
    MetricCard* m_cpuCard{nullptr};
    MetricCard* m_ramCard{nullptr};
    MetricCard* m_processesCard{nullptr};
    MetricCard* m_uptimeCard{nullptr};

    // Hardware banner labels
    QLabel* m_cpuModelLabel{nullptr};
    QLabel* m_coresLabel{nullptr};
    QLabel* m_totalRamLabel{nullptr};
    QLabel* m_installedAppsLabel{nullptr};

    // Charts
    PerformanceChart* m_cpuChart{nullptr};
    PerformanceChart* m_ramChart{nullptr};

    // Top processes table
    ProcessTableView* m_topProcessesTable{nullptr};
};

} // namespace pulse::ui
