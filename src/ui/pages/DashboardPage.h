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

private:
    void setupUi();
    static QString formatUptime(uint64_t seconds);

    // Cards
    MetricCard* m_cpuCard{nullptr};
    MetricCard* m_ramCard{nullptr};
    MetricCard* m_processesCard{nullptr};
    MetricCard* m_uptimeCard{nullptr};

    // Hardware banner labels
    QLabel* m_cpuModelLabel{nullptr};
    QLabel* m_coresLabel{nullptr};
    QLabel* m_totalRamLabel{nullptr};

    // Charts
    PerformanceChart* m_cpuChart{nullptr};
    PerformanceChart* m_ramChart{nullptr};

    // Top processes table
    ProcessTableView* m_topProcessesTable{nullptr};
};

} // namespace pulse::ui
