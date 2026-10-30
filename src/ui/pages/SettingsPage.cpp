#include "SettingsPage.h"
#include "../Theme.h"
#include "../../core/ReportExporter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

namespace pulse::ui {

SettingsPage::SettingsPage(
    std::shared_ptr<SettingsManager> settings,
    std::shared_ptr<ISystemMonitor> sysMonitor,
    std::shared_ptr<IProcessMonitor> procMonitor,
    std::shared_ptr<SystemOptimizer> optimizer,
    std::shared_ptr<SmartAlertEngine> alertEngine,
    QWidget* parent)
    : QWidget(parent),
      m_settings(std::move(settings)),
      m_sysMonitor(std::move(sysMonitor)),
      m_procMonitor(std::move(procMonitor)),
      m_optimizer(std::move(optimizer)),
      m_alertEngine(std::move(alertEngine)) {
    setupUi();
    populateAlertSettings();
}

void SettingsPage::setupUi() {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto* contentWidget = new QWidget(scrollArea);
    auto* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(0, 0, 12, 16);
    mainLayout->setSpacing(14);

    // Title
    auto* titleLabel = new QLabel("Preferences & Performance Reporting", contentWidget);
    titleLabel->setStyleSheet(Theme::pageTitleStyleSheet());
    mainLayout->addWidget(titleLabel);

    // 1. Monitoring Controls Frame
    auto* monFrame = new QFrame(contentWidget);
    monFrame->setStyleSheet(Theme::panelStyleSheet(8));
    auto* monLayout = new QVBoxLayout(monFrame);
    monLayout->setContentsMargins(16, 14, 16, 14);
    monLayout->setSpacing(10);

    auto* monTitle = new QLabel("Telemetry & Sampling Rate", monFrame);
    monTitle->setStyleSheet(Theme::sectionTitleStyleSheet());
    monLayout->addWidget(monTitle);

    auto* sliderLayout = new QHBoxLayout();
    auto* freqLabel = new QLabel("Sampling Frequency:", monFrame);
    freqLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    sliderLayout->addWidget(freqLabel);

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

    // 2. Smart Resource Alerts & Notifications Frame
    auto* alertFrame = new QFrame(contentWidget);
    alertFrame->setStyleSheet(Theme::panelStyleSheet(8));
    auto* alertLayout = new QVBoxLayout(alertFrame);
    alertLayout->setContentsMargins(16, 14, 16, 14);
    alertLayout->setSpacing(10);

    auto* alertTitle = new QLabel("Smart Resource Alerts & System Notifications", alertFrame);
    alertTitle->setStyleSheet(Theme::sectionTitleStyleSheet());
    alertLayout->addWidget(alertTitle);

    auto* alertDesc = new QLabel(
        "PulseOS uses sustained multi-second measurements to detect genuine resource starvation while preventing transient spike false-alarms.",
        alertFrame);
    alertDesc->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    alertDesc->setWordWrap(true);
    alertLayout->addWidget(alertDesc);

    // Alert categories checkboxes
    auto* checksGrid = new QGridLayout();
    checksGrid->setSpacing(8);

    m_chkAlertHighMem = new QCheckBox("Sustained High System RAM Usage", alertFrame);
    m_chkAlertHighCpu = new QCheckBox("Sustained High CPU Load per Process", alertFrame);
    m_chkAlertHeavyBg = new QCheckBox("Resource-Heavy Background Applications", alertFrame);
    m_chkAlertIdleBg  = new QCheckBox("Potentially Idle Background Applications", alertFrame);
    m_chkAlertPressure = new QCheckBox("Critical System Memory Pressure Warnings", alertFrame);

    QString chkStyle = QString("color: %1; font-size: 12px;").arg(Theme::TextPrimary);
    m_chkAlertHighMem->setStyleSheet(chkStyle);
    m_chkAlertHighCpu->setStyleSheet(chkStyle);
    m_chkAlertHeavyBg->setStyleSheet(chkStyle);
    m_chkAlertIdleBg->setStyleSheet(chkStyle);
    m_chkAlertPressure->setStyleSheet(chkStyle);

    checksGrid->addWidget(m_chkAlertHighMem, 0, 0);
    checksGrid->addWidget(m_chkAlertHighCpu, 0, 1);
    checksGrid->addWidget(m_chkAlertHeavyBg, 1, 0);
    checksGrid->addWidget(m_chkAlertIdleBg, 1, 1);
    checksGrid->addWidget(m_chkAlertPressure, 2, 0, 1, 2);

    connect(m_chkAlertHighMem, &QCheckBox::toggled, this, &SettingsPage::onAlertToggled);
    connect(m_chkAlertHighCpu, &QCheckBox::toggled, this, &SettingsPage::onAlertToggled);
    connect(m_chkAlertHeavyBg, &QCheckBox::toggled, this, &SettingsPage::onAlertToggled);
    connect(m_chkAlertIdleBg, &QCheckBox::toggled, this, &SettingsPage::onAlertToggled);
    connect(m_chkAlertPressure, &QCheckBox::toggled, this, &SettingsPage::onAlertToggled);

    alertLayout->addLayout(checksGrid);

    // Threshold sliders
    auto* threshGrid = new QGridLayout();
    threshGrid->setSpacing(10);

    // System RAM threshold slider (60% to 95%)
    auto* lblRam = new QLabel("System RAM Alert Threshold:", alertFrame);
    lblRam->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    threshGrid->addWidget(lblRam, 0, 0);

    m_sliderRamThreshold = new QSlider(Qt::Horizontal, alertFrame);
    m_sliderRamThreshold->setRange(60, 95);
    m_sliderRamThreshold->setValue(85);
    threshGrid->addWidget(m_sliderRamThreshold, 0, 1);

    m_lblRamThreshold = new QLabel("85%", alertFrame);
    m_lblRamThreshold->setFixedWidth(50);
    m_lblRamThreshold->setStyleSheet(QString("font-weight: 600; color: %1;").arg(Theme::TextPrimary));
    threshGrid->addWidget(m_lblRamThreshold, 0, 2);

    // Process CPU threshold slider (15% to 80%)
    auto* lblCpu = new QLabel("Process CPU Alert Threshold:", alertFrame);
    lblCpu->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    threshGrid->addWidget(lblCpu, 1, 0);

    m_sliderCpuThreshold = new QSlider(Qt::Horizontal, alertFrame);
    m_sliderCpuThreshold->setRange(15, 80);
    m_sliderCpuThreshold->setValue(30);
    threshGrid->addWidget(m_sliderCpuThreshold, 1, 1);

    m_lblCpuThreshold = new QLabel("30%", alertFrame);
    m_lblCpuThreshold->setFixedWidth(50);
    m_lblCpuThreshold->setStyleSheet(QString("font-weight: 600; color: %1;").arg(Theme::TextPrimary));
    threshGrid->addWidget(m_lblCpuThreshold, 1, 2);

    // Cooldown duration slider (60s to 900s)
    auto* lblCd = new QLabel("Alert Repeat Cooldown:", alertFrame);
    lblCd->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    threshGrid->addWidget(lblCd, 2, 0);

    m_sliderCooldown = new QSlider(Qt::Horizontal, alertFrame);
    m_sliderCooldown->setRange(60, 900);
    m_sliderCooldown->setSingleStep(30);
    m_sliderCooldown->setValue(300);
    threshGrid->addWidget(m_sliderCooldown, 2, 1);

    m_lblCooldown = new QLabel("300 s", alertFrame);
    m_lblCooldown->setFixedWidth(50);
    m_lblCooldown->setStyleSheet(QString("font-weight: 600; color: %1;").arg(Theme::TextPrimary));
    threshGrid->addWidget(m_lblCooldown, 2, 2);

    connect(m_sliderRamThreshold, &QSlider::valueChanged, this, &SettingsPage::onAlertSliderChanged);
    connect(m_sliderCpuThreshold, &QSlider::valueChanged, this, &SettingsPage::onAlertSliderChanged);
    connect(m_sliderCooldown, &QSlider::valueChanged, this, &SettingsPage::onAlertSliderChanged);

    alertLayout->addLayout(threshGrid);

    // Ignore list container
    auto* ignoreTitle = new QLabel("Alert Ignore List (Process names excluded from alerts):", alertFrame);
    ignoreTitle->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600;").arg(Theme::TextPrimary));
    alertLayout->addWidget(ignoreTitle);

    auto* ignoreRow = new QHBoxLayout();
    ignoreRow->setSpacing(8);

    m_ignoreInput = new QLineEdit(alertFrame);
    m_ignoreInput->setPlaceholderText("e.g. devenv.exe or obs64.exe");
    m_ignoreInput->setClearButtonEnabled(true);
    ignoreRow->addWidget(m_ignoreInput, 1);

    auto* btnAddIgnore = new QPushButton("Add to Ignore List", alertFrame);
    btnAddIgnore->setProperty("class", "actionBtn");
    btnAddIgnore->setCursor(Qt::PointingHandCursor);
    connect(btnAddIgnore, &QPushButton::clicked, this, &SettingsPage::onAddIgnoreClicked);
    ignoreRow->addWidget(btnAddIgnore);

    m_btnRemoveIgnore = new QPushButton("Remove Selected", alertFrame);
    m_btnRemoveIgnore->setProperty("class", "actionBtn");
    m_btnRemoveIgnore->setCursor(Qt::PointingHandCursor);
    connect(m_btnRemoveIgnore, &QPushButton::clicked, this, &SettingsPage::onRemoveIgnoreClicked);
    ignoreRow->addWidget(m_btnRemoveIgnore);

    alertLayout->addLayout(ignoreRow);

    m_ignoreListWidget = new QListWidget(alertFrame);
    m_ignoreListWidget->setFixedHeight(90);
    m_ignoreListWidget->setStyleSheet(QString("QListWidget { background: %1; border: 1px solid %2; border-radius: 6px; color: %3; font-size: 12px; padding: 4px; }")
                                          .arg(Theme::BgSurfaceAlt)
                                          .arg(Theme::BorderSubtle)
                                          .arg(Theme::TextPrimary));
    alertLayout->addWidget(m_ignoreListWidget);

    // Alert history / log row
    auto* histRow = new QHBoxLayout();
    m_alertHistoryStatus = new QLabel("Alert Engine Active.", alertFrame);
    m_alertHistoryStatus->setStyleSheet(QString("color: %1; font-size: 11px;").arg(Theme::AccentGreen));
    histRow->addWidget(m_alertHistoryStatus, 1);

    auto* btnClearHist = new QPushButton("Clear Alert History & Cooldowns", alertFrame);
    btnClearHist->setProperty("class", "actionBtn");
    btnClearHist->setCursor(Qt::PointingHandCursor);
    connect(btnClearHist, &QPushButton::clicked, this, &SettingsPage::onClearAlertHistoryClicked);
    histRow->addWidget(btnClearHist);

    alertLayout->addLayout(histRow);

    mainLayout->addWidget(alertFrame);

    // 3. Automation & Startup Frame
    auto* autoFrame = new QFrame(contentWidget);
    autoFrame->setStyleSheet(Theme::panelStyleSheet(8));
    auto* autoLayout = new QVBoxLayout(autoFrame);
    autoLayout->setContentsMargins(16, 14, 16, 14);
    autoLayout->setSpacing(10);

    auto* autoTitle = new QLabel("Adaptive Automation & Startup", autoFrame);
    autoTitle->setStyleSheet(Theme::sectionTitleStyleSheet());
    autoLayout->addWidget(autoTitle);

    m_autoDetectCheckbox = new QCheckBox("Enable Automatic Workload Detection", autoFrame);
    m_autoDetectCheckbox->setChecked(m_settings ? m_settings->isAutoProfileDetectionEnabled() : true);
    m_autoDetectCheckbox->setStyleSheet(chkStyle);
    connect(m_autoDetectCheckbox, &QCheckBox::toggled, this, &SettingsPage::onAutoDetectToggled);
    autoLayout->addWidget(m_autoDetectCheckbox);

    m_startupCheckbox = new QCheckBox("Launch PulseOS on Windows Startup (Disabled by default)", autoFrame);
    m_startupCheckbox->setChecked(m_settings ? m_settings->isLaunchOnStartupEnabled() : false);
    m_startupCheckbox->setStyleSheet(chkStyle);
    connect(m_startupCheckbox, &QCheckBox::toggled, this, &SettingsPage::onStartupToggled);
    autoLayout->addWidget(m_startupCheckbox);

    mainLayout->addWidget(autoFrame);

    // 4. Privacy & Transparency Statement
    auto* privFrame = new QFrame(contentWidget);
    privFrame->setStyleSheet(Theme::panelStyleSheet(8));
    auto* privLayout = new QVBoxLayout(privFrame);
    privLayout->setContentsMargins(16, 14, 16, 14);
    privLayout->setSpacing(8);

    auto* privTitle = new QLabel("Privacy & Data Transparency", privFrame);
    privTitle->setStyleSheet(Theme::sectionTitleStyleSheet());
    privLayout->addWidget(privTitle);

    auto* privText = new QLabel(
        "🛡 <b>Local Execution Only:</b> PulseOS inspects system performance exclusively on your machine using documented "
        "Windows APIs (Toolhelp32, GetSystemTimes, GlobalMemoryStatusEx). No metrics, process names, or window titles are transmitted "
        "to external cloud servers.", privFrame);
    privText->setStyleSheet(QString("color: %1; font-size: 12px; line-height: 1.4;").arg(Theme::TextSecondary));
    privText->setWordWrap(true);
    privLayout->addWidget(privText);

    m_privacyCheckbox = new QCheckBox("Transparent Local Collection Mode Active", privFrame);
    m_privacyCheckbox->setChecked(m_settings ? m_settings->isDataCollectionTransparent() : true);
    m_privacyCheckbox->setStyleSheet(chkStyle);
    connect(m_privacyCheckbox, &QCheckBox::toggled, this, &SettingsPage::onPrivacyToggled);
    privLayout->addWidget(m_privacyCheckbox);

    mainLayout->addWidget(privFrame);

    // 5. Audit Export Frame
    auto* exportFrame = new QFrame(contentWidget);
    exportFrame->setStyleSheet(Theme::panelStyleSheet(8));
    auto* exportLayout = new QVBoxLayout(exportFrame);
    exportLayout->setContentsMargins(16, 14, 16, 14);
    exportLayout->setSpacing(10);

    auto* exportTitle = new QLabel("Performance Audit & Data Export", exportFrame);
    exportTitle->setStyleSheet(Theme::sectionTitleStyleSheet());
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

    scrollArea->setWidget(contentWidget);
    outerLayout->addWidget(scrollArea);
}

void SettingsPage::populateAlertSettings() {
    AlertSettings s;
    if (m_settings) {
        s = m_settings->getAlertSettings();
    }

    m_chkAlertHighMem->setChecked(s.enableHighMemory);
    m_chkAlertHighCpu->setChecked(s.enableHighCpuProcess);
    m_chkAlertHeavyBg->setChecked(s.enableHeavyBackground);
    m_chkAlertIdleBg->setChecked(s.enableIdleBackground);
    m_chkAlertPressure->setChecked(s.enableMemoryPressure);

    m_sliderRamThreshold->setValue(static_cast<int>(s.systemRamThresholdPercent));
    m_lblRamThreshold->setText(QString("%1%").arg(static_cast<int>(s.systemRamThresholdPercent)));

    m_sliderCpuThreshold->setValue(static_cast<int>(s.processCpuThresholdPercent));
    m_lblCpuThreshold->setText(QString("%1%").arg(static_cast<int>(s.processCpuThresholdPercent)));

    m_sliderCooldown->setValue(static_cast<int>(s.cooldownDurationSeconds));
    m_lblCooldown->setText(QString("%1 s").arg(s.cooldownDurationSeconds));

    refreshIgnoreList();
}

void SettingsPage::refreshIgnoreList() {
    m_ignoreListWidget->clear();
    AlertSettings s = m_settings ? m_settings->getAlertSettings() : AlertSettings{};
    for (const auto& item : s.ignoreList) {
        m_ignoreListWidget->addItem(QString::fromStdString(item));
    }
}

void SettingsPage::onAlertToggled() {
    if (!m_settings) return;

    auto s = m_settings->getAlertSettings();
    s.enableHighMemory = m_chkAlertHighMem->isChecked();
    s.enableHighCpuProcess = m_chkAlertHighCpu->isChecked();
    s.enableHeavyBackground = m_chkAlertHeavyBg->isChecked();
    s.enableIdleBackground = m_chkAlertIdleBg->isChecked();
    s.enableMemoryPressure = m_chkAlertPressure->isChecked();

    m_settings->setAlertSettings(s);
    if (m_alertEngine) {
        m_alertEngine->setSettings(s);
    }
    emit alertSettingsChanged(s);
}

void SettingsPage::onAlertSliderChanged() {
    if (!m_settings) return;

    int ramVal = m_sliderRamThreshold->value();
    int cpuVal = m_sliderCpuThreshold->value();
    int cdVal = m_sliderCooldown->value();

    m_lblRamThreshold->setText(QString("%1%").arg(ramVal));
    m_lblCpuThreshold->setText(QString("%1%").arg(cpuVal));
    m_lblCooldown->setText(QString("%1 s").arg(cdVal));

    auto s = m_settings->getAlertSettings();
    s.systemRamThresholdPercent = static_cast<double>(ramVal);
    s.processCpuThresholdPercent = static_cast<double>(cpuVal);
    s.cooldownDurationSeconds = static_cast<uint32_t>(cdVal);

    m_settings->setAlertSettings(s);
    if (m_alertEngine) {
        m_alertEngine->setSettings(s);
    }
    emit alertSettingsChanged(s);
}

void SettingsPage::onAddIgnoreClicked() {
    QString text = m_ignoreInput->text().trimmed();
    if (text.isEmpty() || !m_settings) return;

    std::string procName = text.toStdString();
    auto s = m_settings->getAlertSettings();
    // Prevent duplicate
    for (const auto& existing : s.ignoreList) {
        if (existing == procName) return;
    }
    s.ignoreList.push_back(procName);
    m_settings->setAlertSettings(s);

    if (m_alertEngine) {
        m_alertEngine->addToIgnoreList(procName);
    }

    m_ignoreInput->clear();
    refreshIgnoreList();
    emit alertSettingsChanged(s);
}

void SettingsPage::onRemoveIgnoreClicked() {
    auto items = m_ignoreListWidget->selectedItems();
    if (items.empty() || !m_settings) return;

    QString target = items.first()->text();
    auto s = m_settings->getAlertSettings();
    auto it = std::remove(s.ignoreList.begin(), s.ignoreList.end(), target.toStdString());
    s.ignoreList.erase(it, s.ignoreList.end());

    m_settings->setAlertSettings(s);
    if (m_alertEngine) {
        m_alertEngine->removeFromIgnoreList(target.toStdString());
    }

    refreshIgnoreList();
    emit alertSettingsChanged(s);
}

void SettingsPage::onClearAlertHistoryClicked() {
    if (m_alertEngine) {
        m_alertEngine->clearHistory();
        m_alertEngine->clearCooldowns();
    }
    m_alertHistoryStatus->setText("Alert history and repeat cooldowns cleared successfully.");
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
    if (filePath.isEmpty() || !m_sysMonitor || !m_procMonitor) return;

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
    if (filePath.isEmpty() || !m_sysMonitor || !m_procMonitor) return;

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
    auto res = QMessageBox::question(
        this, "Reset Configuration",
        "Are you sure you want to restore all settings and alert preferences to their initial defaults?",
        QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::Yes && m_settings) {
        m_settings->resetToDefaults();
        m_intervalSlider->setValue(m_settings->getSamplingIntervalMs());
        m_autoDetectCheckbox->setChecked(m_settings->isAutoProfileDetectionEnabled());
        m_privacyCheckbox->setChecked(m_settings->isDataCollectionTransparent());
        m_startupCheckbox->setChecked(m_settings->isLaunchOnStartupEnabled());
        populateAlertSettings();
        m_exportStatusLabel->setText("All preferences reset to factory defaults.");
    }
}

} // namespace pulse::ui
