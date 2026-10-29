#include "MainWindow.h"
#include "Theme.h"
#include "components/ActionConfirmDialog.h"
#include "../core/DiagnosticLog.h"

#include "../core/strategies/CodingStrategy.h"
#include "../core/strategies/GamingStrategy.h"
#include "../core/strategies/StudyStrategy.h"
#include "../core/strategies/RenderingStrategy.h"
#include "../core/strategies/BatteryStrategy.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>

namespace pulse::ui {

MainWindow::MainWindow(std::shared_ptr<SettingsManager> settings, QWidget *parent)
    : QMainWindow(parent),
      m_settings(std::move(settings)),
      m_systemMonitor(std::make_shared<SystemMonitor>(60)),
      m_processMonitor(std::make_shared<ProcessMonitor>()),
      m_optimizer(std::make_shared<SystemOptimizer>()),
      m_analyzer(std::make_shared<PerformanceAnalyzer>()) {

    logDiagnostic("[MainWindow] Starting MainWindow construction...");

    m_taskDetector = std::make_shared<TaskDetector>(m_processMonitor, m_systemMonitor);
    m_recEngine = std::make_shared<RecommendationEngine>();

    initStrategies();
    setupUi();

    // Start background monitoring worker thread
    m_workerThread = new QThread(this);
    m_worker = new MonitoringWorker(
        m_systemMonitor, m_processMonitor, m_taskDetector, m_recEngine, m_activeStrategy);
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, this, [this]() {
        uint32_t interval = m_settings ? m_settings->getSamplingIntervalMs() : 1000;
        m_worker->startMonitoring(interval);
    });
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &MonitoringWorker::dataSampled, this,
            &MainWindow::onMonitoringDataReady, Qt::QueuedConnection);

    m_workerThread->start();
    logDiagnostic("[MainWindow] MainWindow initialized and worker thread started.");
}

MainWindow::~MainWindow() {
    logDiagnostic("[MainWindow] Destroying MainWindow; stopping background worker thread...");
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait(3000);
    }
}

void MainWindow::initStrategies() {
    m_strategies[WorkloadProfile::Coding] = std::make_shared<CodingStrategy>();
    m_strategies[WorkloadProfile::Gaming] = std::make_shared<GamingStrategy>();
    m_strategies[WorkloadProfile::Study] = std::make_shared<StudyStrategy>();
    m_strategies[WorkloadProfile::Rendering] = std::make_shared<RenderingStrategy>();
    m_strategies[WorkloadProfile::BatterySaver] = std::make_shared<BatteryStrategy>();

    WorkloadProfile initProfile = m_settings ? m_settings->getSelectedProfile() : WorkloadProfile::Coding;
    auto it = m_strategies.find(initProfile);
    if (it != m_strategies.end()) {
        m_activeStrategy = it->second;
    } else {
        m_activeStrategy = m_strategies[WorkloadProfile::Coding];
    }
}

void MainWindow::setupUi() {
    setWindowTitle("PulseOS — Adaptive CPU Performance Assistant");
    resize(1280, 840);
    setMinimumSize(1020, 660);

    setStyleSheet(Theme::getApplicationStyleSheet());

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Left sidebar
    rootLayout->addWidget(createSidebar());

    // Right content area
    auto* rightContainer = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(20, 16, 20, 20);
    rightLayout->setSpacing(16);

    // Header bar
    rightLayout->addWidget(createTopBar());

    // Stacked widget for all 6 pages
    m_stackedWidget = new QStackedWidget(this);

    m_dashboardPage = new DashboardPage(this);
    m_processesPage = new ProcessesPage(this);
    m_profilesPage = new ProfilesPage(m_settings, this);
    m_recommendationsPage = new RecommendationsPage(this);
    m_analysisPage = new AnalysisPage(m_analyzer, this);
    m_settingsPage = new SettingsPage(m_settings, m_systemMonitor, m_processMonitor, m_optimizer, this);

    connect(m_profilesPage, &ProfilesPage::profileChanged, this, &MainWindow::onProfileChanged);
    connect(m_recommendationsPage, &RecommendationsPage::actionTriggered, this, &MainWindow::onActionTriggered);
    connect(m_settingsPage, &SettingsPage::monitoringIntervalChanged, this, &MainWindow::onMonitoringIntervalChanged);
    connect(m_settingsPage, &SettingsPage::monitoringToggled, this, &MainWindow::onMonitoringToggled);

    m_stackedWidget->addWidget(m_dashboardPage);
    m_stackedWidget->addWidget(m_processesPage);
    m_stackedWidget->addWidget(m_profilesPage);
    m_stackedWidget->addWidget(m_recommendationsPage);
    m_stackedWidget->addWidget(m_analysisPage);
    m_stackedWidget->addWidget(m_settingsPage);

    rightLayout->addWidget(m_stackedWidget, 1);
    rootLayout->addWidget(rightContainer, 1);
}

QWidget* MainWindow::createSidebar() {
    auto* sidebar = new QWidget(this);
    sidebar->setObjectName("sidebarWidget");
    sidebar->setFixedWidth(230);

    auto* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(14, 20, 14, 20);
    layout->setSpacing(6);

    // Brand Title
    auto* brandLabel = new QLabel("PULSE<b>OS</b>", sidebar);
    brandLabel->setStyleSheet(QString("font-size: 19px; font-weight: 800; color: %1; letter-spacing: 1.5px; padding-left: 6px;")
        .arg(Theme::AccentCyan));
    layout->addWidget(brandLabel);

    auto* subLabel = new QLabel("ADAPTIVE CPU ASSISTANT", sidebar);
    subLabel->setStyleSheet(QString("font-size: 10px; font-weight: 600; color: %1; letter-spacing: 0.8px; margin-bottom: 16px; padding-left: 6px;")
        .arg(Theme::TextMuted));
    layout->addWidget(subLabel);

    // Navigation buttons
    m_sidebarGroup = new QButtonGroup(this);
    m_sidebarGroup->setExclusive(true);

    const struct { int id; const char* label; } navItems[] = {
        {0, "  ▣   Dashboard"},
        {1, "  ☲   Process Explorer"},
        {2, "  ◈   Workload Profiles"},
        {3, "  ◇   Recommendations"},
        {4, "  ▲   Performance Analysis"},
        {5, "  ⚙   Settings & Privacy"}
    };

    for (const auto& item : navItems) {
        auto* btn = new QPushButton(item.label, sidebar);
        btn->setProperty("class", "sidebarBtn");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        if (item.id == 0) btn->setChecked(true);
        m_sidebarGroup->addButton(btn, item.id);
        layout->addWidget(btn);
    }

    connect(m_sidebarGroup, &QButtonGroup::idClicked, this, &MainWindow::onSidebarButtonClicked);

    layout->addStretch();

    // Footer info
    auto* footer = new QLabel("Windows 10/11 64-bit\nv1.0.0 Production", sidebar);
    footer->setStyleSheet(QString("font-size: 10px; color: %1; padding-left: 6px; line-height: 1.3;").arg(Theme::TextMuted));
    layout->addWidget(footer);

    return sidebar;
}

QWidget* MainWindow::createTopBar() {
    auto* topBar = new QFrame(this);
    topBar->setStyleSheet(Theme::panelStyleSheet(8));

    auto* layout = new QHBoxLayout(topBar);
    layout->setContentsMargins(12, 5, 12, 5);
    layout->setSpacing(10);

    // Monitoring Status Indicator
    m_statusPill = new QLabel("● LIVE", topBar);
    m_statusPill->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::AccentGreen));
    layout->addWidget(m_statusPill);

    // Active Profile Pill
    std::string profName = m_activeStrategy ? m_activeStrategy->getProfileName() : "Coding & Development";
    m_activeProfilePill = new QLabel("Profile: " + QString::fromStdString(profName), topBar);
    m_activeProfilePill->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::AccentPurple));
    layout->addWidget(m_activeProfilePill);

    layout->addStretch();

    // Rollback Button
    m_rollbackTopBtn = new QPushButton("⟲ Rollback Action", topBar);
    m_rollbackTopBtn->setProperty("class", "actionBtn");
    m_rollbackTopBtn->setCursor(Qt::PointingHandCursor);
    m_rollbackTopBtn->setVisible(false);
    connect(m_rollbackTopBtn, &QPushButton::clicked, this, &MainWindow::onRollbackRequested);
    layout->addWidget(m_rollbackTopBtn);

    // Quick CPU Pill
    m_cpuQuickPill = new QLabel("CPU: --%", topBar);
    m_cpuQuickPill->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::AccentCyan));
    layout->addWidget(m_cpuQuickPill);

    // Quick RAM Pill
    m_ramQuickPill = new QLabel("RAM: --%", topBar);
    m_ramQuickPill->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::AccentGreen));
    layout->addWidget(m_ramQuickPill);

    return topBar;
}

void MainWindow::onSidebarButtonClicked(int id) {
    logDiagnostic("[MainWindow] User navigation requested: Switching to page index " + std::to_string(id));
    if (m_stackedWidget && id >= 0 && id < m_stackedWidget->count()) {
        m_stackedWidget->setCurrentIndex(id);
    }
    logDiagnostic("[MainWindow] Navigation completed for page index " + std::to_string(id));
}

void MainWindow::updateHeaderPills(const SystemMetrics& metrics) {
    if (m_cpuQuickPill) {
        m_cpuQuickPill->setText(QString("CPU: %1%").arg(QString::number(metrics.cpuUsagePercent, 'f', 1)));
        m_cpuQuickPill->setStyleSheet(QString(
            "background-color: %1; color: %2; border-radius: 12px; padding: 4px 10px; font-weight: 600; font-size: 11px;"
        ).arg(Theme::BgSurfaceAlt).arg(Theme::getUsageColorHex(metrics.cpuUsagePercent)));
    }

    if (m_ramQuickPill) {
        m_ramQuickPill->setText(QString("RAM: %1%").arg(QString::number(metrics.ramUsagePercent, 'f', 1)));
        m_ramQuickPill->setStyleSheet(QString(
            "background-color: %1; color: %2; border-radius: 12px; padding: 4px 10px; font-weight: 600; font-size: 11px;"
        ).arg(Theme::BgSurfaceAlt).arg(Theme::getUsageColorHex(metrics.ramUsagePercent)));
    }

    if (m_rollbackTopBtn && m_optimizer) {
        m_rollbackTopBtn->setVisible(m_optimizer->hasRollbackAvailable());
    }
}

void MainWindow::onProfileChanged(WorkloadProfile profile) {
    logDiagnostic("[MainWindow] onProfileChanged triggered with profile: " + profileToString(profile));
    auto it = m_strategies.find(profile);
    if (it != m_strategies.end()) {
        m_activeStrategy = it->second;
        if (m_worker) {
            m_worker->setActiveStrategy(m_activeStrategy);
        }
        if (m_activeProfilePill) {
            m_activeProfilePill->setText("Profile: " + QString::fromStdString(m_activeStrategy->getProfileName()));
        }
    }
}

void MainWindow::onActionTriggered(const Recommendation& rec) {
    if (!m_optimizer) return;

    OptimizationAction action;
    action.id = rec.actionPayload;
    action.title = rec.title;
    action.description = rec.recommendedAction;
    action.expectedImpact = rec.benefits;
    action.rollbackDescription = "Restores previous configuration state.";
    action.canRollback = (rec.actionPayload != "trim_working_sets");

    // Show Safe Action Center confirmation dialog
    ActionConfirmDialog dlg(action, this);
    if (dlg.exec() == QDialog::Accepted) {
        auto result = m_optimizer->applyAction(action);
        if (result.success) {
            QMessageBox::information(this, "Optimization Applied",
                QString::fromStdString(result.message));
        } else {
            QMessageBox::warning(this, "Optimization Failed",
                QString::fromStdString(result.message));
        }
        if (m_rollbackTopBtn) {
            m_rollbackTopBtn->setVisible(m_optimizer->hasRollbackAvailable());
        }
    }
}

void MainWindow::onRollbackRequested() {
    if (!m_optimizer) return;
    auto res = m_optimizer->rollbackLastAction();
    if (res.success) {
        QMessageBox::information(this, "Rollback Successful", QString::fromStdString(res.message));
    } else {
        QMessageBox::warning(this, "Rollback Failed", QString::fromStdString(res.message));
    }
    if (m_rollbackTopBtn) {
        m_rollbackTopBtn->setVisible(m_optimizer->hasRollbackAvailable());
    }
}

void MainWindow::onMonitoringIntervalChanged(uint32_t intervalMs) {
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "setInterval", Qt::QueuedConnection, Q_ARG(uint32_t, intervalMs));
    }
}

void MainWindow::onMonitoringToggled(bool enabled) {
    if (!m_worker) return;
    if (enabled) {
        uint32_t interval = m_settings ? m_settings->getSamplingIntervalMs() : 1000;
        QMetaObject::invokeMethod(m_worker, "startMonitoring", Qt::QueuedConnection, Q_ARG(uint32_t, interval));
        if (m_statusPill) {
            m_statusPill->setText("● Live");
            m_statusPill->setStyleSheet(QString("color: %1; font-weight: 600; font-size: 12px; padding-left: 6px;")
                .arg(Theme::AccentGreen));
        }
    } else {
        QMetaObject::invokeMethod(m_worker, "stopMonitoring", Qt::QueuedConnection);
        if (m_statusPill) {
            m_statusPill->setText("○ Paused");
            m_statusPill->setStyleSheet(QString("color: %1; font-weight: 600; font-size: 12px; padding-left: 6px;")
                .arg(Theme::AccentAmber));
        }
    }
}

void MainWindow::onSamplingTimerFired() {
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "triggerImmediateSample", Qt::QueuedConnection);
    }
}

void MainWindow::onMonitoringDataReady(
    const pulse::SystemMetrics& sysMetrics,
    const std::vector<pulse::ProcessInfo>& processes,
    const pulse::TaskDetectionResult& detection,
    const std::vector<pulse::Recommendation>& recommendations) {

    auto tStart = std::chrono::steady_clock::now();

    // 1. Update Header pills
    updateHeaderPills(sysMetrics);

    // 2. Update Dashboard page
    if (m_dashboardPage) {
        auto cpuHistory = m_systemMonitor->getCpuHistory();
        auto ramHistory = m_systemMonitor->getRamHistory();

        auto topProcesses = processes;
        ProcessMonitor::sortProcesses(topProcesses, ProcessMonitor::SortField::CpuDesc);
        if (topProcesses.size() > 10) topProcesses.resize(10);

        m_dashboardPage->updateMetrics(sysMetrics, cpuHistory, ramHistory, topProcesses);
    }

    // 3. Update Process Explorer page
    if (m_processesPage) {
        m_processesPage->updateProcesses(processes);
    }

    // 4. Update Profiles page
    if (m_profilesPage) {
        m_profilesPage->updateDetection(detection);
    }

    // 5. Update Recommendations page
    if (m_recommendationsPage) {
        m_recommendationsPage->updateRecommendations(recommendations);
    }

    // 6. Update Analysis page live data
    if (m_analysisPage) {
        std::string prof = m_activeStrategy ? m_activeStrategy->getProfileName() : "Standard";
        m_analysisPage->updateLiveMetrics(sysMetrics, processes, prof);
    }

    auto tEnd = std::chrono::steady_clock::now();
    auto durMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();
    if (durMs > 25) {
        logDiagnostic("[MainWindow] UI update on GUI thread took " + std::to_string(durMs) + " ms");
    }
}

} // namespace pulse::ui
