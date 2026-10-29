#include "SettingsPage.h"
#include "../Theme.h"
#include "../../core/ReportExporter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

namespace pulse::ui {

SettingsPage::SettingsPage(
    std::shared_ptr<SettingsManager> settings,
    std::shared_ptr<ISystemMonitor> sysMonitor,
    std::shared_ptr<IProcessMonitor> procMonitor,
    std::shared_ptr<SystemOptimizer> optimizer,
    QWidget* parent)
    : QWidget(parent),
      m_settings(std::move(settings)),
      m_sysMonitor(std::move(sysMonitor)),
      m_procMonitor(std::move(procMonitor)),
      m_optimizer(std::move(optimizer)) {
    setupUi();
}

void SettingsPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    // Title
    auto* titleLabel = new QLabel("Preferences & Performance Reporting", this);
    titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    mainLayout->addWidget(titleLabel);

    // 1. Monitoring Controls Frame
    auto* monFrame = new QFrame(this);
    monFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px 18px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));
    auto* monLayout = new QVBoxLayout(monFrame);
    monLayout->setContentsMargins(0, 0, 0, 0);
    monLayout->setSpacing(10);

    auto* monTitle = new QLabel("Telemetry & Sampling Rate", monFrame);
    monTitle->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentBlue));
    monLayout->addWidget(monTitle);

    auto* sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(new QLabel("Sampling Frequency:", monFrame));

    m_intervalSlider = new QSlider(Qt::Horizontal, monFrame);
    m_intervalSlider->setRange(500, 5000);
    m_intervalSlider->setSingleStep(250);
    m_intervalSlider->setValue(m_settings ? m_settings->getSamplingIntervalMs() : 1000);
    sliderLayout->addWidget(m_intervalSlider, 1);

    m_intervalLabel = new QLabel(QString("%1 ms").arg(m_intervalSlider->value()), monFrame);
    m_intervalLabel->setFixedWidth(65);
    m_intervalLabel->setStyleSheet(QString("font-weight: 600; color: %1;").arg(Theme::TextPrimary));
    sliderLayout->addWidget(m_intervalLabel);

    connect(m_intervalSlider, &QSlider::valueChanged, this, &SettingsPage::onSliderValueChanged);
    monLayout->addLayout(sliderLayout);

    // Live Monitor Toggle Button
    auto* toggleLayout = new QHBoxLayout();
    bool isMonEnabled = m_settings ? m_settings->isMonitoringEnabled() : true;
    m_monitorToggleBtn = new QPushButton(isMonEnabled ? "Pause Live Monitoring" : "Resume Live Monitoring", monFrame);
    m_monitorToggleBtn->setProperty("class", "actionBtn");
    m_monitorToggleBtn->setCursor(Qt::PointingHandCursor);
    connect(m_monitorToggleBtn, &QPushButton::clicked, this, &SettingsPage::onMonitoringToggleClicked);
    toggleLayout->addWidget(m_monitorToggleBtn);
    toggleLayout->addStretch();
    monLayout->addLayout(toggleLayout);

    mainLayout->addWidget(monFrame);

    // 2. Automation & Startup Frame
    auto* autoFrame = new QFrame(this);
    autoFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px 18px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));
    auto* autoLayout = new QVBoxLayout(autoFrame);
    autoLayout->setContentsMargins(0, 0, 0, 0);
    autoLayout->setSpacing(10);

    auto* autoTitle = new QLabel("Adaptive Automation & Startup", autoFrame);
    autoTitle->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentPurple));
    autoLayout->addWidget(autoTitle);

    m_autoDetectCheckbox = new QCheckBox("Enable Automatic Workload Detection", autoFrame);
    m_autoDetectCheckbox->setChecked(m_settings ? m_settings->isAutoProfileDetectionEnabled() : true);
    connect(m_autoDetectCheckbox, &QCheckBox::toggled, this, &SettingsPage::onAutoDetectToggled);
    autoLayout->addWidget(m_autoDetectCheckbox);

    m_startupCheckbox = new QCheckBox("Launch PulseOS on Windows Startup (Disabled by default)", autoFrame);
    m_startupCheckbox->setChecked(m_settings ? m_settings->isLaunchOnStartupEnabled() : false);
    connect(m_startupCheckbox, &QCheckBox::toggled, this, &SettingsPage::onStartupToggled);
    autoLayout->addWidget(m_startupCheckbox);

    mainLayout->addWidget(autoFrame);

    // 3. Privacy & Transparency Statement
    auto* privFrame = new QFrame(this);
    privFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px 18px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));
    auto* privLayout = new QVBoxLayout(privFrame);
    privLayout->setContentsMargins(0, 0, 0, 0);
    privLayout->setSpacing(8);

    auto* privTitle = new QLabel("Privacy & Data Transparency", privFrame);
    privTitle->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentGreen));
    privLayout->addWidget(privTitle);

    auto* privText = new QLabel(
        "🛡 <b>Local Execution Only:</b> PulseOS inspects system performance exclusively on your machine using documented "
        "Windows APIs (Toolhelp32, GetSystemTimes, GlobalMemoryStatusEx). No metrics, process names, or window titles are transmitted "
        "to external cloud servers.", privFrame);
    privText->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    privText->setWordWrap(true);
    privLayout->addWidget(privText);

    m_privacyCheckbox = new QCheckBox("Transparent Local Collection Mode Active", privFrame);
    m_privacyCheckbox->setChecked(m_settings ? m_settings->isDataCollectionTransparent() : true);
    connect(m_privacyCheckbox, &QCheckBox::toggled, this, &SettingsPage::onPrivacyToggled);
    privLayout->addWidget(m_privacyCheckbox);

    mainLayout->addWidget(privFrame);

    // 4. Audit Export Frame
    auto* exportFrame = new QFrame(this);
    exportFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px 18px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));
    auto* exportLayout = new QVBoxLayout(exportFrame);
    exportLayout->setContentsMargins(0, 0, 0, 0);
    exportLayout->setSpacing(10);

    auto* exportTitle = new QLabel("Performance Audit & Data Export", exportFrame);
    exportTitle->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    exportLayout->addWidget(exportTitle);

    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    auto* exportCsvBtn = new QPushButton("Export Metrics to CSV...", exportFrame);
    exportCsvBtn->setProperty("class", "primaryBtn");
    exportCsvBtn->setCursor(Qt::PointingHandCursor);
    connect(exportCsvBtn, &QPushButton::clicked, this, &SettingsPage::onExportCsvClicked);
    btnRow->addWidget(exportCsvBtn);

    auto* exportAuditBtn = new QPushButton("Export Markdown Audit Report...", exportFrame);
    exportAuditBtn->setProperty("class", "actionBtn");
    exportAuditBtn->setCursor(Qt::PointingHandCursor);
    connect(exportAuditBtn, &QPushButton::clicked, this, &SettingsPage::onExportAuditReportClicked);
    btnRow->addWidget(exportAuditBtn);

    btnRow->addStretch();

    auto* resetBtn = new QPushButton("Reset All to Defaults", exportFrame);
    resetBtn->setProperty("class", "dangerBtn");
    resetBtn->setCursor(Qt::PointingHandCursor);
    connect(resetBtn, &QPushButton::clicked, this, &SettingsPage::onResetDefaultsClicked);
    btnRow->addWidget(resetBtn);

    exportLayout->addLayout(btnRow);

    m_exportStatusLabel = new QLabel("", exportFrame);
    m_exportStatusLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(Theme::AccentGreen));
    exportLayout->addWidget(m_exportStatusLabel);

    mainLayout->addWidget(exportFrame);

    mainLayout->addStretch();
}

void SettingsPage::onSliderValueChanged(int val) {
    m_intervalLabel->setText(QString("%1 ms").arg(val));
    if (m_settings) {
        m_settings->setSamplingIntervalMs(val);
    }
    emit monitoringIntervalChanged(static_cast<uint32_t>(val));
}

void SettingsPage::onMonitoringToggleClicked() {
    bool current = m_settings ? m_settings->isMonitoringEnabled() : true;
    bool updated = !current;
    if (m_settings) m_settings->setMonitoringEnabled(updated);

    m_monitorToggleBtn->setText(updated ? "Pause Live Monitoring" : "Resume Live Monitoring");
    emit monitoringToggled(updated);
}

void SettingsPage::onAutoDetectToggled(bool checked) {
    if (m_settings) m_settings->setAutoProfileDetectionEnabled(checked);
}

void SettingsPage::onPrivacyToggled(bool checked) {
    if (m_settings) m_settings->setDataCollectionTransparent(checked);
}

void SettingsPage::onStartupToggled(bool checked) {
    if (m_settings) m_settings->setLaunchOnStartupEnabled(checked);
}

void SettingsPage::onExportCsvClicked() {
    QString defPath = QString::fromStdString(m_settings ? m_settings->getExportDirectory() : "") +
                      "/PulseOS_Performance_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".csv";

    QString filePath = QFileDialog::getSaveFileName(this, "Export System Performance CSV", defPath, "CSV Files (*.csv)");
    if (filePath.isEmpty()) return;

    if (!m_sysMonitor || !m_procMonitor) return;

    SystemMetrics sys = m_sysMonitor->getLastMetrics();
    auto procs = m_procMonitor->getLastProcessList();
    std::string profile = m_settings ? profileToString(m_settings->getSelectedProfile()) : "Standard";

    if (ReportExporter::exportToCsv(filePath.toStdString(), sys, procs, profile)) {
        m_exportStatusLabel->setText("Successfully exported CSV to: " + filePath);
    } else {
        QMessageBox::warning(this, "Export Failed", "Failed to write CSV file to destination path.");
    }
}

void SettingsPage::onExportAuditReportClicked() {
    QString defPath = QString::fromStdString(m_settings ? m_settings->getExportDirectory() : "") +
                      "/PulseOS_AuditReport_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".md";

    QString filePath = QFileDialog::getSaveFileName(this, "Export Audit Markdown Report", defPath, "Markdown Files (*.md)");
    if (filePath.isEmpty()) return;

    if (!m_sysMonitor || !m_procMonitor) return;

    SystemMetrics sys = m_sysMonitor->getLastMetrics();
    auto procs = m_procMonitor->getLastProcessList();
    std::string profile = m_settings ? profileToString(m_settings->getSelectedProfile()) : "Standard";
    auto appliedActions = m_optimizer ? m_optimizer->getAppliedActions() : std::vector<OptimizationAction>{};

    if (ReportExporter::exportAuditReport(filePath.toStdString(), sys, procs, {}, appliedActions, profile)) {
        m_exportStatusLabel->setText("Successfully exported Audit Report to: " + filePath);
    } else {
        QMessageBox::warning(this, "Export Failed", "Failed to write Markdown audit report.");
    }
}

void SettingsPage::onResetDefaultsClicked() {
    if (QMessageBox::question(this, "Reset Defaults", "Reset all monitoring intervals, profiles, and preferences to default?") == QMessageBox::Yes) {
        if (m_settings) m_settings->resetToDefaults();
        m_intervalSlider->setValue(1000);
        m_autoDetectCheckbox->setChecked(true);
        m_privacyCheckbox->setChecked(true);
        m_startupCheckbox->setChecked(false);
        m_exportStatusLabel->setText("Preferences reset to factory defaults.");
    }
}

} // namespace pulse::ui

