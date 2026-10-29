#include "AnalysisPage.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace pulse::ui {

AnalysisPage::AnalysisPage(std::shared_ptr<PerformanceAnalyzer> analyzer, QWidget* parent)
    : QWidget(parent), m_analyzer(std::move(analyzer)) {
    setupUi();
}

void AnalysisPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(14);

    // Header & Explanation
    auto* topLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel("Before & After Performance Analysis", this);
    titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    topLayout->addWidget(titleLabel);

    topLayout->addStretch();

    m_captureBaselineBtn = new QPushButton("Capture Baseline", this);
    m_captureBaselineBtn->setProperty("class", "actionBtn");
    m_captureBaselineBtn->setCursor(Qt::PointingHandCursor);
    connect(m_captureBaselineBtn, &QPushButton::clicked, this, &AnalysisPage::onCaptureBaselineClicked);
    topLayout->addWidget(m_captureBaselineBtn);

    m_capturePostBtn = new QPushButton("Capture Post-Optimization", this);
    m_capturePostBtn->setProperty("class", "actionBtn");
    m_capturePostBtn->setCursor(Qt::PointingHandCursor);
    connect(m_capturePostBtn, &QPushButton::clicked, this, &AnalysisPage::onCapturePostClicked);
    topLayout->addWidget(m_capturePostBtn);

    m_resetBtn = new QPushButton("Reset", this);
    m_resetBtn->setProperty("class", "actionBtn");
    m_resetBtn->setCursor(Qt::PointingHandCursor);
    connect(m_resetBtn, &QPushButton::clicked, this, &AnalysisPage::onResetClicked);
    topLayout->addWidget(m_resetBtn);

    mainLayout->addLayout(topLayout);

    // Methodology Explanatory Frame
    auto* methodFrame = new QFrame(this);
    methodFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; padding: 10px 14px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));
    auto* methodLayout = new QVBoxLayout(methodFrame);
    methodLayout->setContentsMargins(0, 0, 0, 0);
    methodLayout->setSpacing(4);

    auto* methodText = new QLabel(
        "<b>Rigorous Measurement Protocol:</b> Capture a baseline under representative workload, apply approved optimizations in the Safe Action Center, "
        "and capture a post-optimization snapshot under comparable conditions. PulseOS validates process consistency to avoid false claims of causation.",
        methodFrame);
    methodText->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    methodText->setWordWrap(true);
    methodLayout->addWidget(methodText);

    mainLayout->addWidget(methodFrame);

    // Side-by-side Snapshots Grid
    auto* snapGrid = new QGridLayout();
    snapGrid->setSpacing(12);

    auto createSnapBox = [](const QString& headerTitle, QLabel*& statusLbl, QLabel*& cpuLbl, QLabel*& ramLbl, QLabel*& procLbl, QWidget* parent) -> QFrame* {
        auto* frame = new QFrame(parent);
        frame->setStyleSheet(QString(
            "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px; }"
        ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));

        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);

        auto* title = new QLabel(headerTitle, frame);
        title->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentBlue));
        layout->addWidget(title);

        statusLbl = new QLabel("Not yet captured", frame);
        statusLbl->setStyleSheet(QString("font-size: 11px; color: %1;").arg(Theme::TextMuted));
        layout->addWidget(statusLbl);

        auto* innerGrid = new QGridLayout();
        innerGrid->setSpacing(6);

        innerGrid->addWidget(new QLabel("CPU Utilization:", frame), 0, 0);
        cpuLbl = new QLabel("--%", frame);
        cpuLbl->setStyleSheet(QString("font-weight: 600; color: %1;").arg(Theme::TextPrimary));
        innerGrid->addWidget(cpuLbl, 0, 1);

        innerGrid->addWidget(new QLabel("RAM Usage:", frame), 1, 0);
        ramLbl = new QLabel("--%", frame);
        ramLbl->setStyleSheet(QString("font-weight: 600; color: %1;").arg(Theme::TextPrimary));
        innerGrid->addWidget(ramLbl, 1, 1);

        innerGrid->addWidget(new QLabel("Process Count:", frame), 2, 0);
        procLbl = new QLabel("--", frame);
        procLbl->setStyleSheet(QString("font-weight: 600; color: %1;").arg(Theme::TextPrimary));
        innerGrid->addWidget(procLbl, 2, 1);

        layout->addLayout(innerGrid);
        return frame;
    };

    auto* baselineBox = createSnapBox("1. Baseline State", m_baselineStatusLabel, m_baselineCpuLabel, m_baselineRamLabel, m_baselineProcCountLabel, this);
    auto* postBox = createSnapBox("2. Post-Optimization State", m_postStatusLabel, m_postCpuLabel, m_postRamLabel, m_postProcCountLabel, this);

    snapGrid->addWidget(baselineBox, 0, 0);
    snapGrid->addWidget(postBox, 0, 1);

    mainLayout->addLayout(snapGrid);

    // Delta & Verification Card
    auto* deltaFrame = new QFrame(this);
    deltaFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 16px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));

    auto* deltaLayout = new QVBoxLayout(deltaFrame);
    deltaLayout->setContentsMargins(0, 0, 0, 0);
    deltaLayout->setSpacing(10);

    auto* deltaHeader = new QLabel("Observed Measured Deltas & Comparability", deltaFrame);
    deltaHeader->setStyleSheet(QString("font-size: 15px; font-weight: 700; color: %1;").arg(Theme::AccentPurple));
    deltaLayout->addWidget(deltaHeader);

    auto* deltasRow = new QHBoxLayout();
    deltasRow->setSpacing(24);

    m_cpuDeltaLabel = new QLabel("CPU Delta: --%", deltaFrame);
    m_cpuDeltaLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    deltasRow->addWidget(m_cpuDeltaLabel);

    m_ramDeltaLabel = new QLabel("RAM Delta: --%", deltaFrame);
    m_ramDeltaLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    deltasRow->addWidget(m_ramDeltaLabel);

    m_ramBytesDeltaLabel = new QLabel("RAM Net Shift: -- MB", deltaFrame);
    m_ramBytesDeltaLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    deltasRow->addWidget(m_ramBytesDeltaLabel);

    deltasRow->addStretch();
    deltaLayout->addLayout(deltasRow);

    // Comparability Warning
    m_comparabilityWarningLabel = new QLabel("", deltaFrame);
    m_comparabilityWarningLabel->setStyleSheet(QString("color: %1; font-weight: 500; font-size: 12px;").arg(Theme::AccentAmber));
    m_comparabilityWarningLabel->setWordWrap(true);
    deltaLayout->addWidget(m_comparabilityWarningLabel);

    // Summary Interpretation
    m_summaryMessageLabel = new QLabel("Capture both a baseline and a post-optimization snapshot to evaluate measurable impact.", deltaFrame);
    m_summaryMessageLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(Theme::TextSecondary));
    m_summaryMessageLabel->setWordWrap(true);
    deltaLayout->addWidget(m_summaryMessageLabel);

    mainLayout->addWidget(deltaFrame, 1);
}

void AnalysisPage::updateLiveMetrics(
    const SystemMetrics& metrics,
    const std::vector<ProcessInfo>& processes,
    const std::string& profileName) {

    m_lastMetrics = metrics;
    m_lastProcesses = processes;
    m_lastProfileName = profileName;
}

void AnalysisPage::onCaptureBaselineClicked() {
    if (!m_analyzer) return;
    m_analyzer->captureBaseline("Baseline Sample", m_lastMetrics, m_lastProcesses, m_lastProfileName);

    m_baselineStatusLabel->setText("Captured just now");
    m_baselineCpuLabel->setText(QString::number(m_lastMetrics.cpuUsagePercent, 'f', 1) + "%");
    m_baselineRamLabel->setText(QString::number(m_lastMetrics.ramUsagePercent, 'f', 1) + "%");
    m_baselineProcCountLabel->setText(QString::number(m_lastProcesses.size()) + " processes");

    updateComparisonView();
}

void AnalysisPage::onCapturePostClicked() {
    if (!m_analyzer) return;
    m_analyzer->capturePostOptimization("Post-Optimization Sample", m_lastMetrics, m_lastProcesses, m_lastProfileName);

    m_postStatusLabel->setText("Captured just now");
    m_postCpuLabel->setText(QString::number(m_lastMetrics.cpuUsagePercent, 'f', 1) + "%");
    m_postRamLabel->setText(QString::number(m_lastMetrics.ramUsagePercent, 'f', 1) + "%");
    m_postProcCountLabel->setText(QString::number(m_lastProcesses.size()) + " processes");

    updateComparisonView();
}

void AnalysisPage::onResetClicked() {
    if (!m_analyzer) return;
    m_analyzer->reset();

    m_baselineStatusLabel->setText("Not yet captured");
    m_baselineCpuLabel->setText("--%");
    m_baselineRamLabel->setText("--%");
    m_baselineProcCountLabel->setText("--");

    m_postStatusLabel->setText("Not yet captured");
    m_postCpuLabel->setText("--%");
    m_postRamLabel->setText("--%");
    m_postProcCountLabel->setText("--");

    m_cpuDeltaLabel->setText("CPU Delta: --%");
    m_ramDeltaLabel->setText("RAM Delta: --%");
    m_ramBytesDeltaLabel->setText("RAM Net Shift: -- MB");
    m_comparabilityWarningLabel->setText("");
    m_summaryMessageLabel->setText("Capture both a baseline and a post-optimization snapshot to evaluate measurable impact.");
}

void AnalysisPage::updateComparisonView() {
    if (!m_analyzer) return;
    auto comp = m_analyzer->compare();
    if (!comp.has_value()) return;

    const auto& c = *comp;

    // CPU Delta
    QString cpuSign = (c.cpuDeltaPercent > 0) ? "+" : "";
    m_cpuDeltaLabel->setText(QString("CPU Delta: %1%2%").arg(cpuSign).arg(QString::number(c.cpuDeltaPercent, 'f', 1)));
    if (c.cpuDeltaPercent < -1.0) {
        m_cpuDeltaLabel->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentGreen));
    } else if (c.cpuDeltaPercent > 1.0) {
        m_cpuDeltaLabel->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentRed));
    } else {
        m_cpuDeltaLabel->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::TextSecondary));
    }

    // RAM Delta
    QString ramSign = (c.ramDeltaPercent > 0) ? "+" : "";
    m_ramDeltaLabel->setText(QString("RAM Delta: %1%2%").arg(ramSign).arg(QString::number(c.ramDeltaPercent, 'f', 1)));
    if (c.ramDeltaPercent < -1.0) {
        m_ramDeltaLabel->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentGreen));
    } else if (c.ramDeltaPercent > 1.0) {
        m_ramDeltaLabel->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::AccentAmber));
    } else {
        m_ramDeltaLabel->setStyleSheet(QString("font-size: 14px; font-weight: 700; color: %1;").arg(Theme::TextSecondary));
    }

    // RAM Bytes freed/gained
    double mbShift = static_cast<double>(c.ramDeltaBytes) / (1024.0 * 1024.0);
    QString mbSign = (mbShift > 0) ? "+" : "";
    m_ramBytesDeltaLabel->setText(QString("RAM Net Shift: %1%2 MB").arg(mbSign).arg(QString::number(mbShift, 'f', 0)));

    // Warning and Summary
    m_comparabilityWarningLabel->setText(QString::fromStdString(c.comparabilityWarning));
    m_summaryMessageLabel->setText(QString::fromStdString(c.summaryMessage));
}

} // namespace pulse::ui

