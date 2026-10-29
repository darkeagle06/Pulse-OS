#include "ProcessesPage.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace pulse::ui {

ProcessesPage::ProcessesPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void ProcessesPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    // Top Header & Summary
    auto* headerLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel("Windows Process Explorer", this);
    titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    headerLayout->addWidget(titleLabel);

    m_summaryLabel = new QLabel("Scanning active processes...", this);
    m_summaryLabel->setStyleSheet(QString("color: %1;").arg(Theme::TextSecondary));
    headerLayout->addWidget(m_summaryLabel, 0, Qt::AlignRight);

    mainLayout->addLayout(headerLayout);

    // Main Process Table
    m_processTable = new ProcessTableView(this);
    mainLayout->addWidget(m_processTable, 1);

    // Process Details Inspector Panel (bottom)
    m_detailsPanel = new QFrame(this);
    m_detailsPanel->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 12px 16px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));

    auto* detailsLayout = new QVBoxLayout(m_detailsPanel);
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    detailsLayout->setSpacing(8);

    auto* panelTitle = new QLabel("Process Inspection & Diagnostics", m_detailsPanel);
    panelTitle->setStyleSheet(QString("font-size: 13px; font-weight: 600; color: %1;").arg(Theme::AccentBlue));
    detailsLayout->addWidget(panelTitle);

    auto* grid = new QGridLayout();
    grid->setSpacing(8);

    m_detailNameLabel = new QLabel("Select a process above to inspect details.", m_detailsPanel);
    m_detailNameLabel->setStyleSheet(QString("font-weight: 600; color: %1;").arg(Theme::TextPrimary));
    grid->addWidget(m_detailNameLabel, 0, 0, 1, 2);

    m_detailPidLabel = new QLabel("PID: --", m_detailsPanel);
    grid->addWidget(m_detailPidLabel, 0, 2);

    m_detailCpuLabel = new QLabel("CPU Load: --%", m_detailsPanel);
    grid->addWidget(m_detailCpuLabel, 0, 3);

    m_detailRamLabel = new QLabel("RAM Working Set: --", m_detailsPanel);
    grid->addWidget(m_detailRamLabel, 1, 0);

    m_detailThreadsLabel = new QLabel("Threads: --", m_detailsPanel);
    grid->addWidget(m_detailThreadsLabel, 1, 1);

    m_detailPrioLabel = new QLabel("Priority: --", m_detailsPanel);
    grid->addWidget(m_detailPrioLabel, 1, 2);

    m_detailAccessLabel = new QLabel("Status: --", m_detailsPanel);
    grid->addWidget(m_detailAccessLabel, 1, 3);

    m_detailPathLabel = new QLabel("Executable Path: --", m_detailsPanel);
    m_detailPathLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(Theme::TextMuted));
    m_detailPathLabel->setWordWrap(true);
    grid->addWidget(m_detailPathLabel, 2, 0, 1, 4);

    detailsLayout->addLayout(grid);

    // Non-destructive guarantee notice
    auto* notice = new QLabel("🛡 Safe Operation Guarantee: PulseOS never terminates processes automatically without explicit consent.", m_detailsPanel);
    notice->setStyleSheet(QString("color: %1; font-size: 11px; margin-top: 4px;").arg(Theme::AccentGreen));
    detailsLayout->addWidget(notice);

    mainLayout->addWidget(m_detailsPanel);

    connect(m_processTable, &ProcessTableView::processSelected, this, &ProcessesPage::onProcessSelected);
}

void ProcessesPage::updateProcesses(const std::vector<ProcessInfo>& processes) {
    m_processTable->updateProcesses(processes);

    uint64_t totalRam = 0;
    uint32_t accessibleCount = 0;
    for (const auto& p : processes) {
        totalRam += p.workingSetBytes;
        if (p.accessStatus == ProcessAccessStatus::Accessible) accessibleCount++;
    }

    double ramGb = static_cast<double>(totalRam) / (1024.0 * 1024.0 * 1024.0);
    m_summaryLabel->setText(QString("%1 Active Processes (%2 Accessible) | %3 GB Total Tracked")
        .arg(processes.size())
        .arg(accessibleCount)
        .arg(QString::number(ramGb, 'f', 1)));
}

void ProcessesPage::onProcessSelected(const ProcessInfo& process) {
    m_detailNameLabel->setText(QString::fromStdString(process.name));
    m_detailPidLabel->setText(QString("PID: %1").arg(process.pid));
    m_detailCpuLabel->setText(QString("CPU Load: %1%").arg(QString::number(process.cpuUsagePercent, 'f', 1)));

    double ramMb = static_cast<double>(process.workingSetBytes) / (1024.0 * 1024.0);
    double privMb = static_cast<double>(process.privateBytes) / (1024.0 * 1024.0);
    m_detailRamLabel->setText(QString("RAM: %1 MB (Commit: %2 MB)")
        .arg(QString::number(ramMb, 'f', 1))
        .arg(QString::number(privMb, 'f', 1)));

    m_detailThreadsLabel->setText(QString("Threads: %1").arg(process.threadCount));
    m_detailPrioLabel->setText(QString("Priority: %1").arg(QString::fromStdString(process.priorityClassString)));

    QString accessStr = (process.accessStatus == ProcessAccessStatus::Accessible) ? "Accessible" :
                        (process.accessStatus == ProcessAccessStatus::AccessDenied) ? "Restricted" : "System Protected";
    m_detailAccessLabel->setText(QString("Status: %1").arg(accessStr));

    m_detailPathLabel->setText(QString("Path: %1")
        .arg(process.executablePath.empty() ? "(Protected or system process)" : QString::fromStdString(process.executablePath)));
}

} // namespace pulse::ui

