#include "DashboardPage.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace pulse::ui {

DashboardPage::DashboardPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void DashboardPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(14);

    // 1. Hardware Info Banner
    auto* hwFrame = new QFrame(this);
    hwFrame->setStyleSheet(Theme::bannerStyleSheet(Theme::AccentCyan));
    auto* hwLayout = new QHBoxLayout(hwFrame);
    hwLayout->setContentsMargins(14, 10, 14, 10);
    hwLayout->setSpacing(24);

    m_cpuModelLabel = new QLabel("Processor: Querying...", hwFrame);
    m_cpuModelLabel->setStyleSheet(QString("font-weight: 600; font-size: 12px; color: %1;").arg(Theme::TextPrimary));
    hwLayout->addWidget(m_cpuModelLabel);

    m_coresLabel = new QLabel("Cores: --", hwFrame);
    m_coresLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(Theme::TextSecondary));
    hwLayout->addWidget(m_coresLabel);

    m_totalRamLabel = new QLabel("Physical RAM: -- GB", hwFrame);
    m_totalRamLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(Theme::TextSecondary));
    hwLayout->addWidget(m_totalRamLabel);

    hwLayout->addStretch();
    mainLayout->addWidget(hwFrame);

    // 2. Metrics Cards Grid (4 columns)
    auto* cardsGrid = new QGridLayout();
    cardsGrid->setSpacing(12);

    m_cpuCard = new MetricCard("Total CPU Load", "%", this);
    m_cpuCard->setShowProgressBar(true);
    cardsGrid->addWidget(m_cpuCard, 0, 0);

    m_ramCard = new MetricCard("Memory Usage", "%", this);
    m_ramCard->setShowProgressBar(true);
    cardsGrid->addWidget(m_ramCard, 0, 1);

    m_processesCard = new MetricCard("Active Processes", "", this);
    cardsGrid->addWidget(m_processesCard, 0, 2);

    m_uptimeCard = new MetricCard("System Uptime", "", this);
    cardsGrid->addWidget(m_uptimeCard, 0, 3);

    mainLayout->addLayout(cardsGrid);

    // 3. Performance Charts (2 columns: CPU on left, RAM on right)
    auto* chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(12);

    m_cpuChart = new PerformanceChart("CPU Utilization History (Last 60s)", Theme::colorAccentCyan(), this);
    chartsLayout->addWidget(m_cpuChart, 1);

    m_ramChart = new PerformanceChart("RAM Utilization History (Last 60s)", Theme::colorAccentGreen(), this);
    chartsLayout->addWidget(m_ramChart, 1);

    mainLayout->addLayout(chartsLayout);

    // 4. Top Resource Consuming Processes Section
    auto* topHeader = new QLabel("Top Resource Consuming Processes", this);
    topHeader->setStyleSheet(Theme::sectionTitleStyleSheet());
    mainLayout->addWidget(topHeader);

    m_topProcessesTable = new ProcessTableView(this);
    mainLayout->addWidget(m_topProcessesTable, 1);
}

QString DashboardPage::formatUptime(uint64_t seconds) {
    uint64_t days = seconds / 86400;
    uint64_t hours = (seconds % 86400) / 3600;
    uint64_t minutes = (seconds % 3600) / 60;
    uint64_t secs = seconds % 60;

    if (days > 0) {
        return QString("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
    }
    return QString("%1h %2m %3s").arg(hours).arg(minutes).arg(secs);
}

void DashboardPage::updateMetrics(
    const SystemMetrics& metrics,
    const std::vector<double>& cpuHistory,
    const std::vector<double>& ramHistory,
    const std::vector<ProcessInfo>& topProcesses) {

    // Update hardware info
    if (!metrics.cpuName.empty()) {
        m_cpuModelLabel->setText("Processor: " + QString::fromStdString(metrics.cpuName));
    }
    m_coresLabel->setText(QString("Cores: %1 Physical / %2 Logical")
        .arg(metrics.cpuPhysicalCores)
        .arg(metrics.cpuLogicalProcessors));

    double totalRamGb = static_cast<double>(metrics.totalRamBytes) / (1024.0 * 1024.0 * 1024.0);
    m_totalRamLabel->setText(QString("Physical RAM: %1 GB").arg(QString::number(totalRamGb, 'f', 1)));

    // Update Cards
    m_cpuCard->setValue(metrics.cpuUsagePercent);
    m_cpuCard->setProgress(static_cast<int>(metrics.cpuUsagePercent));
    m_cpuCard->setAccentColor(Theme::getUsageColor(metrics.cpuUsagePercent));
    m_cpuCard->setSubtext(QString("%1% unreserved capacity")
        .arg(QString::number(100.0 - metrics.cpuUsagePercent, 'f', 1)));

    m_ramCard->setValue(metrics.ramUsagePercent);
    m_ramCard->setProgress(static_cast<int>(metrics.ramUsagePercent));
    m_ramCard->setAccentColor(Theme::getUsageColor(metrics.ramUsagePercent));
    double usedGb = static_cast<double>(metrics.usedRamBytes) / (1024.0 * 1024.0 * 1024.0);
    double availGb = static_cast<double>(metrics.availableRamBytes) / (1024.0 * 1024.0 * 1024.0);
    m_ramCard->setSubtext(QString("%1 GB Used / %2 GB Available")
        .arg(QString::number(usedGb, 'f', 1))
        .arg(QString::number(availGb, 'f', 1)));

    m_processesCard->setValueString(QString::number(metrics.activeProcessCount));
    m_processesCard->setSubtext(QString("%1 Active Threads").arg(metrics.activeThreadCount));

    m_uptimeCard->setValueString(formatUptime(metrics.uptimeSeconds));
    m_uptimeCard->setSubtext("Continuous system operation");

    // Update Charts
    m_cpuChart->updateData(cpuHistory, metrics.cpuUsagePercent);
    m_ramChart->updateData(ramHistory, metrics.ramUsagePercent);

    // Update Top Processes
    m_topProcessesTable->updateProcesses(topProcesses);
}

} // namespace pulse::ui

