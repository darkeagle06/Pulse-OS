#include "ProfilesPage.h"
#include "../Theme.h"
#include "../../core/DiagnosticLog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace pulse::ui {

ProfilesPage::ProfilesPage(std::shared_ptr<SettingsManager> settings, QWidget* parent)
    : QWidget(parent), m_settings(std::move(settings)) {
    logDiagnostic("[ProfilesPage] Initializing ProfilesPage...");
    if (m_settings) {
        m_currentProfile = m_settings->getSelectedProfile();
    }
    setupUi();
    updateProfileDetails(m_currentProfile);
    logDiagnostic("[ProfilesPage] ProfilesPage constructed successfully.");
}

void ProfilesPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(14);

    // 1. Task Detection Banner
    auto* bannerFrame = new QFrame(this);
    bannerFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px 18px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));

    auto* bannerLayout = new QVBoxLayout(bannerFrame);
    bannerLayout->setContentsMargins(0, 0, 0, 0);
    bannerLayout->setSpacing(8);

    auto* topRow = new QHBoxLayout();
    auto* bannerTitle = new QLabel("Adaptive Workload Detection", bannerFrame);
    bannerTitle->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentBlue));
    topRow->addWidget(bannerTitle);

    topRow->addStretch();

    m_autoDetectCheckbox = new QCheckBox("Auto-Apply Detected Profile", bannerFrame);
    m_autoDetectCheckbox->setChecked(m_settings ? m_settings->isAutoProfileDetectionEnabled() : true);
    m_autoDetectCheckbox->setStyleSheet(QString("color: %1; font-weight: 500;").arg(Theme::TextPrimary));
    connect(m_autoDetectCheckbox, &QCheckBox::toggled, this, &ProfilesPage::onAutoDetectToggled);
    topRow->addWidget(m_autoDetectCheckbox);

    bannerLayout->addLayout(topRow);

    // Detected Profile & Confidence
    auto* detectRow = new QHBoxLayout();
    detectRow->setSpacing(12);

    m_detectedProfileLabel = new QLabel("Analyzing current foreground application...", bannerFrame);
    m_detectedProfileLabel->setStyleSheet(QString("font-size: 13px; font-weight: 600; color: %1;").arg(Theme::TextPrimary));
    detectRow->addWidget(m_detectedProfileLabel);

    detectRow->addStretch();

    m_confidenceLabel = new QLabel("Confidence: --%", bannerFrame);
    m_confidenceLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(Theme::TextSecondary));
    detectRow->addWidget(m_confidenceLabel);

    m_confidenceBar = new QProgressBar(bannerFrame);
    m_confidenceBar->setRange(0, 100);
    m_confidenceBar->setValue(0);
    m_confidenceBar->setFixedWidth(120);
    m_confidenceBar->setFixedHeight(8);
    m_confidenceBar->setTextVisible(false);
    detectRow->addWidget(m_confidenceBar);

    bannerLayout->addLayout(detectRow);

    // Primary Evidence & Uncertainty Notes
    m_evidenceLabel = new QLabel("Primary Evidence: Querying active tasks...", bannerFrame);
    m_evidenceLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(Theme::TextSecondary));
    m_evidenceLabel->setWordWrap(true);
    bannerLayout->addWidget(m_evidenceLabel);

    m_factorsLabel = new QLabel("", bannerFrame);
    m_factorsLabel->setStyleSheet(QString("font-size: 11px; color: %1;").arg(Theme::TextMuted));
    m_factorsLabel->setWordWrap(true);
    bannerLayout->addWidget(m_factorsLabel);

    mainLayout->addWidget(bannerFrame);

    // 2. Selectable Profile Cards Grid
    auto* sectionLabel = new QLabel("Select System Workload Profile", this);
    sectionLabel->setStyleSheet(QString("font-size: 15px; font-weight: 600; color: %1; margin-top: 6px;")
        .arg(Theme::TextPrimary));
    mainLayout->addWidget(sectionLabel);

    auto* profileGrid = new QGridLayout();
    profileGrid->setSpacing(10);

    m_profileBtnGroup = new QButtonGroup(this);
    m_profileBtnGroup->setExclusive(true);

    const struct { WorkloadProfile profile; const char* title; const char* desc; } profiles[] = {
        {WorkloadProfile::Coding, "Coding & Development", "Compiler bursts, low scheduling latency, balanced power."},
        {WorkloadProfile::Gaming, "Gaming & High Performance", "Max core clock residency, reduced background preemption."},
        {WorkloadProfile::Study, "Browsing & Studying", "Quiet acoustics, low fan speed, efficient power usage."},
        {WorkloadProfile::Rendering, "Video Editing & Rendering", "Continuous all-core throughput, prevent sleep during jobs."},
        {WorkloadProfile::BatterySaver, "Battery Saving", "Cap turbo states, prioritize deep sleep states, max efficiency."}
    };

    int col = 0;
    int row = 0;
    for (const auto& item : profiles) {
        auto* btn = new QPushButton(this);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(80);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background-color: %1;"
            "  border: 1px solid %2;"
            "  border-radius: 8px;"
            "  padding: 10px 14px;"
            "  text-align: left;"
            "}"
            "QPushButton:hover {"
            "  background-color: %3;"
            "  border-color: %4;"
            "}"
            "QPushButton:checked {"
            "  background-color: %3;"
            "  border: 2px solid %4;"
            "}"
        ).arg(Theme::BgSurface).arg(Theme::BorderSubtle).arg(Theme::BgSurfaceAlt).arg(Theme::AccentBlue));

        auto* btnLayout = new QVBoxLayout(btn);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnLayout->setSpacing(4);

        auto* titleLbl = new QLabel(item.title, btn);
        titleLbl->setStyleSheet(QString("font-size: 13px; font-weight: 600; color: %1;").arg(Theme::TextPrimary));
        titleLbl->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        btnLayout->addWidget(titleLbl);

        auto* descLbl = new QLabel(item.desc, btn);
        descLbl->setStyleSheet(QString("font-size: 11px; color: %1;").arg(Theme::TextSecondary));
        descLbl->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        descLbl->setWordWrap(true);
        btnLayout->addWidget(descLbl);

        if (item.profile == m_currentProfile) {
            btn->setChecked(true);
        }

        m_profileBtnGroup->addButton(btn, static_cast<int>(item.profile));
        profileGrid->addWidget(btn, row, col);

        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }

    connect(m_profileBtnGroup, &QButtonGroup::idClicked, this, &ProfilesPage::onProfileButtonClicked);
    mainLayout->addLayout(profileGrid);

    // 3. Profile Strategy Inspector Frame
    auto* inspectorFrame = new QFrame(this);
    inspectorFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px 18px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));

    auto* inspLayout = new QVBoxLayout(inspectorFrame);
    inspLayout->setContentsMargins(0, 0, 0, 0);
    inspLayout->setSpacing(8);

    m_profileNameLabel = new QLabel(this);
    m_profileNameLabel->setStyleSheet(QString("font-size: 15px; font-weight: 700; color: %1;").arg(Theme::AccentPurple));
    inspLayout->addWidget(m_profileNameLabel);

    m_profileDescLabel = new QLabel(this);
    m_profileDescLabel->setStyleSheet(QString("color: %1;").arg(Theme::TextPrimary));
    m_profileDescLabel->setWordWrap(true);
    inspLayout->addWidget(m_profileDescLabel);

    m_profileRationaleLabel = new QLabel(this);
    m_profileRationaleLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(Theme::TextSecondary));
    m_profileRationaleLabel->setWordWrap(true);
    inspLayout->addWidget(m_profileRationaleLabel);

    m_powerSchemeLabel = new QLabel(this);
    m_powerSchemeLabel->setStyleSheet(QString("font-size: 12px; font-weight: 600; color: %1;").arg(Theme::AccentBlue));
    inspLayout->addWidget(m_powerSchemeLabel);

    mainLayout->addWidget(inspectorFrame, 1);
}

void ProfilesPage::updateProfileDetails(WorkloadProfile profile) {
    m_currentProfile = profile;

    switch (profile) {
        case WorkloadProfile::Coding:
            m_profileNameLabel->setText("Strategy Details: Coding & Development");
            m_profileDescLabel->setText("Optimized for rapid iterative development, IDE editing, and multithreaded compiler bursts.");
            m_profileRationaleLabel->setText("Rationale: Keeps foreground editing responsive while allowing full CPU clock frequencies during compilation. Balances cooling and power.");
            m_powerSchemeLabel->setText("Recommended Windows Power Plan: Balanced (GUID: 381b4222-f694-41f0-9685-ff5bb260df2e)");
            break;
        case WorkloadProfile::Gaming:
            m_profileNameLabel->setText("Strategy Details: Gaming & High Performance");
            m_profileDescLabel->setText("Tuned for low frametime variance, minimized input latency, and zero background thread interference.");
            m_profileRationaleLabel->setText("Rationale: Real-time game loops stutter if interrupted by background workers. High Performance keeps core clocks unthrottled.");
            m_powerSchemeLabel->setText("Recommended Windows Power Plan: High Performance (GUID: 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c)");
            break;
        case WorkloadProfile::Study:
            m_profileNameLabel->setText("Strategy Details: Browsing & Studying");
            m_profileDescLabel->setText("Designed for cool lap temperatures, low acoustic fan noise, and light research tasks.");
            m_profileRationaleLabel->setText("Rationale: Text reading and web browsing require minimal computation. Suppresses unnecessary background spikes to maintain silent fans.");
            m_powerSchemeLabel->setText("Recommended Windows Power Plan: Balanced (GUID: 381b4222-f694-41f0-9685-ff5bb260df2e)");
            break;
        case WorkloadProfile::Rendering:
            m_profileNameLabel->setText("Strategy Details: Video Editing & Rendering");
            m_profileDescLabel->setText("Maximizes sustained all-core rendering throughput and prevents sleep timeouts during long export jobs.");
            m_profileRationaleLabel->setText("Rationale: Export pipelines saturate CPU/GPU continuously. Ensures the operating system does not enter sleep states mid-render.");
            m_powerSchemeLabel->setText("Recommended Windows Power Plan: High Performance (GUID: 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c)");
            break;
        case WorkloadProfile::BatterySaver:
            m_profileNameLabel->setText("Strategy Details: Battery Saving");
            m_profileDescLabel->setText("Maximizes laptop battery runtime by curbing turbo clocks and favoring processor sleep states.");
            m_profileRationaleLabel->setText("Rationale: Power draw escalates quadratically with clock speed. Capping frequencies yields significant battery life extensions.");
            m_powerSchemeLabel->setText("Recommended Windows Power Plan: Power Saver (GUID: a1841308-3541-4fab-bc81-f71556f20b4a)");
            break;
        default:
            break;
    }
}

void ProfilesPage::updateDetection(const TaskDetectionResult& detection) {
    int confPct = static_cast<int>(detection.confidence * 100.0);

    m_detectedProfileLabel->setText(QString("Detected: %1 (Foreground: %2)")
        .arg(QString::fromStdString(profileToString(detection.detectedProfile)))
        .arg(QString::fromStdString(detection.foregroundProcessName.empty() ? "System" : detection.foregroundProcessName)));

    m_confidenceLabel->setText(QString("Confidence: %1%").arg(confPct));
    m_confidenceBar->setValue(confPct);

    if (confPct >= 80) {
        m_confidenceBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; border-radius: 4px; }")
            .arg(Theme::AccentGreen));
    } else if (confPct >= 60) {
        m_confidenceBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; border-radius: 4px; }")
            .arg(Theme::AccentAmber));
    } else {
        m_confidenceBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; border-radius: 4px; }")
            .arg(Theme::AccentBlue));
    }

    m_evidenceLabel->setText("Primary Evidence: " + QString::fromStdString(detection.primaryEvidence));

    QString factorsText;
    for (const auto& f : detection.contributingFactors) {
        if (!factorsText.isEmpty()) factorsText += " • ";
        factorsText += QString::fromStdString(f);
    }
    m_factorsLabel->setText(factorsText);

    // Auto-apply ONLY IF:
    // 1. Auto-detect checkbox is enabled
    // 2. Confidence is high (>= 70%)
    // 3. Detected profile is a concrete profile (not AutoDetected / PulseOS self)
    // 4. Different from currently active profile
    if (m_autoDetectCheckbox->isChecked() &&
        detection.confidence >= 0.70 &&
        detection.detectedProfile != WorkloadProfile::AutoDetected &&
        detection.detectedProfile != m_currentProfile) {
        logDiagnostic("[ProfilesPage] Auto-applying high-confidence profile: " +
                      profileToString(detection.detectedProfile));
        setActiveProfile(detection.detectedProfile);
    }
}

void ProfilesPage::setActiveProfile(WorkloadProfile profile) {
    if (m_currentProfile == profile) {
        auto* btn = m_profileBtnGroup->button(static_cast<int>(profile));
        if (btn && !btn->isChecked()) btn->setChecked(true);
        return;
    }
    logDiagnostic("[ProfilesPage] Setting active profile to: " + profileToString(profile));
    m_currentProfile = profile;
    auto* btn = m_profileBtnGroup->button(static_cast<int>(profile));
    if (btn) btn->setChecked(true);
    updateProfileDetails(profile);
    emit profileChanged(profile);
}

void ProfilesPage::onProfileButtonClicked(int id) {
    auto profile = static_cast<WorkloadProfile>(id);
    logDiagnostic("[ProfilesPage] User clicked profile button: " + profileToString(profile));
    setActiveProfile(profile);
    if (m_settings) {
        m_settings->setSelectedProfile(profile);
    }
}

void ProfilesPage::onAutoDetectToggled(bool checked) {
    logDiagnostic(std::string("[ProfilesPage] Auto-detect toggled: ") + (checked ? "ON" : "OFF"));
    if (m_settings) {
        m_settings->setAutoProfileDetectionEnabled(checked);
    }
}

} // namespace pulse::ui

