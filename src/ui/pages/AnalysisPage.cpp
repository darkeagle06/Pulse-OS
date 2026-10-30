#include "AnalysisPage.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>

namespace pulse::ui {

AnalysisPage::AnalysisPage(std::shared_ptr<PerformanceAnalyzer> analyzer, QWidget* parent)
    : QWidget(parent), m_analyzer(std::move(analyzer)) {
    setupUi();
}

void AnalysisPage::setupUi() {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Wrap page in a smooth vertical QScrollArea with zero horizontal scroll
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto* contentWidget = new QWidget(scrollArea);
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(14);

    // 1. Header Bar with Title & Action Controls (Responsive Layout)
    m_headerLayout = new QGridLayout();
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(10);

    m_titleLabel = new QLabel("Before & After Performance Analysis", contentWidget);
    m_titleLabel->setStyleSheet(Theme::pageTitleStyleSheet());
    m_titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_btnContainer = new QWidget(contentWidget);
    m_btnContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    auto* btnLayout = new QHBoxLayout(m_btnContainer);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(8);

    m_captureBaselineBtn = new QPushButton("Capture Baseline", m_btnContainer);
    m_captureBaselineBtn->setProperty("class", "actionBtn");
    m_captureBaselineBtn->setCursor(Qt::PointingHandCursor);
    connect(m_captureBaselineBtn, &QPushButton::clicked, this, &AnalysisPage::onCaptureBaselineClicked);
    btnLayout->addWidget(m_captureBaselineBtn);

    m_capturePostBtn = new QPushButton("Capture Post-Optimization", m_btnContainer);
    m_capturePostBtn->setProperty("class", "actionBtn");
    m_capturePostBtn->setCursor(Qt::PointingHandCursor);
    connect(m_capturePostBtn, &QPushButton::clicked, this, &AnalysisPage::onCapturePostClicked);
    btnLayout->addWidget(m_capturePostBtn);

    m_resetBtn = new QPushButton("Reset", m_btnContainer);
    m_resetBtn->setProperty("class", "actionBtn");
    m_resetBtn->setCursor(Qt::PointingHandCursor);
    connect(m_resetBtn, &QPushButton::clicked, this, &AnalysisPage::onResetClicked);
    btnLayout->addWidget(m_resetBtn);

    mainLayout->addLayout(m_headerLayout);

    // 2. Methodology Explanatory Banner
    m_methodFrame = new QFrame(contentWidget);
    m_methodFrame->setStyleSheet(Theme::bannerStyleSheet(Theme::AccentCyan));
    m_methodFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* methodLayout = new QVBoxLayout(m_methodFrame);
    methodLayout->setContentsMargins(14, 10, 14, 10);
    methodLayout->setSpacing(4);

    auto* methodText = new QLabel(
        "<b>Rigorous Measurement Protocol:</b> Capture a baseline under representative workload, apply approved optimizations in the Safe Action Center, "
        "and capture a post-optimization snapshot under comparable conditions. PulseOS validates process consistency to avoid false claims of causation.",
        m_methodFrame);
    methodText->setStyleSheet(QString("color: %1; font-size: 12px; line-height: 1.4;").arg(Theme::TextSecondary));
    methodText->setWordWrap(true);
    methodLayout->addWidget(methodText);

    mainLayout->addWidget(m_methodFrame);

    // 3. Side-by-side / Stacked Snapshots Grid
    m_snapGrid = new QGridLayout();
    m_snapGrid->setContentsMargins(0, 0, 0, 0);
    m_snapGrid->setSpacing(12);

    auto createSnapBox = [](const QString& headerTitle,
                            QLabel*& statusLbl,
                            QLabel*& cpuLbl,
                            QProgressBar*& cpuBar,
                            QLabel*& ramLbl,
                            QProgressBar*& ramBar,
                            QLabel*& procLbl,
                            QWidget* parent) -> QFrame* {
        auto* frame = new QFrame(parent);
        frame->setStyleSheet(Theme::panelStyleSheet(8));
        frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(8);

        auto* title = new QLabel(headerTitle, frame);
        title->setStyleSheet(QString("font-size: 13px; font-weight: 700; color: %1;").arg(Theme::AccentCyan));
        layout->addWidget(title);

        statusLbl = new QLabel("Not yet captured", frame);
        statusLbl->setStyleSheet(QString("font-size: 11px; color: %1;").arg(Theme::TextMuted));
        layout->addWidget(statusLbl);

        auto* innerGrid = new QGridLayout();
        innerGrid->setSpacing(6);
        innerGrid->setContentsMargins(0, 4, 0, 0);

        // CPU row
        auto* cpuTitle = new QLabel("CPU Utilization:", frame);
        cpuTitle->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
        innerGrid->addWidget(cpuTitle, 0, 0);

        cpuLbl = new QLabel("--%", frame);
        cpuLbl->setStyleSheet(QString("font-weight: 600; font-size: 12px; color: %1;").arg(Theme::TextPrimary));
        cpuLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        innerGrid->addWidget(cpuLbl, 0, 1);

        cpuBar = new QProgressBar(frame);
        cpuBar->setRange(0, 100);
        cpuBar->setValue(0);
        cpuBar->setFixedHeight(4);
        cpuBar->setTextVisible(false);
        cpuBar->setStyleSheet(QString(
            "QProgressBar { background-color: %1; border: none; border-radius: 2px; }"
            "QProgressBar::chunk { background-color: %2; border-radius: 2px; }"
        ).arg(Theme::BgSurfaceAlt).arg(Theme::AccentCyan));
        innerGrid->addWidget(cpuBar, 1, 0, 1, 2);

        // RAM row
        auto* ramTitle = new QLabel("RAM Usage:", frame);
        ramTitle->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
        innerGrid->addWidget(ramTitle, 2, 0);

        ramLbl = new QLabel("--%", frame);
        ramLbl->setStyleSheet(QString("font-weight: 600; font-size: 12px; color: %1;").arg(Theme::TextPrimary));
        ramLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        innerGrid->addWidget(ramLbl, 2, 1);

        ramBar = new QProgressBar(frame);
        ramBar->setRange(0, 100);
        ramBar->setValue(0);
        ramBar->setFixedHeight(4);
        ramBar->setTextVisible(false);
        ramBar->setStyleSheet(QString(
            "QProgressBar { background-color: %1; border: none; border-radius: 2px; }"
            "QProgressBar::chunk { background-color: %2; border-radius: 2px; }"
        ).arg(Theme::BgSurfaceAlt).arg(Theme::AccentGreen));
        innerGrid->addWidget(ramBar, 3, 0, 1, 2);

        // Process count row
        auto* procTitle = new QLabel("Process Count:", frame);
        procTitle->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
        innerGrid->addWidget(procTitle, 4, 0);

        procLbl = new QLabel("--", frame);
        procLbl->setStyleSheet(QString("font-weight: 600; font-size: 12px; color: %1;").arg(Theme::TextPrimary));
        procLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        innerGrid->addWidget(procLbl, 4, 1);

        innerGrid->setColumnStretch(0, 1);
        innerGrid->setColumnStretch(1, 1);

        layout->addLayout(innerGrid);
        return frame;
    };

    m_baselineBox = createSnapBox("1. Baseline State",
                                  m_baselineStatusLabel,
                                  m_baselineCpuLabel,
                                  m_baselineCpuBar,
                                  m_baselineRamLabel,
                                  m_baselineRamBar,
                                  m_baselineProcCountLabel,
                                  contentWidget);

    m_postBox = createSnapBox("2. Post-Optimization State",
                              m_postStatusLabel,
                              m_postCpuLabel,
                              m_postCpuBar,
                              m_postRamLabel,
                              m_postRamBar,
                              m_postProcCountLabel,
                              contentWidget);

    mainLayout->addLayout(m_snapGrid);

    // 4. Comparison Section (Full width panel with 3 responsive metric cards)
    m_deltaFrame = new QFrame(contentWidget);
    m_deltaFrame->setStyleSheet(Theme::panelStyleSheet(8));
    m_deltaFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* deltaLayout = new QVBoxLayout(m_deltaFrame);
    deltaLayout->setContentsMargins(16, 14, 16, 14);
    deltaLayout->setSpacing(12);

    auto* deltaHeader = new QLabel("Observed Measured Deltas & Comparability", m_deltaFrame);
    deltaHeader->setStyleSheet(Theme::sectionTitleStyleSheet());
    deltaLayout->addWidget(deltaHeader);

    // Three Aligned Metric Cards Grid
    m_deltaCardsGrid = new QGridLayout();
    m_deltaCardsGrid->setContentsMargins(0, 0, 0, 0);
    m_deltaCardsGrid->setSpacing(10);

    auto createDeltaCard = [](const QString& titleText,
                              QLabel*& valLabel,
                              const QString& subtext,
                              QWidget* parent) -> QFrame* {
        auto* card = new QFrame(parent);
        card->setStyleSheet(QString(
            "QFrame {"
            "  background-color: %1;"
            "  border: 1px solid %2;"
            "  border-radius: 6px;"
            "}"
        ).arg(Theme::BgSurfaceAlt).arg(Theme::BorderSubtle));
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        auto* layout = new QVBoxLayout(card);
        layout->setContentsMargins(14, 12, 14, 12);
        layout->setSpacing(4);

        auto* titleLbl = new QLabel(titleText, card);
        titleLbl->setStyleSheet(QString(
            "font-size: 11px; font-weight: 600; color: %1; text-transform: uppercase; letter-spacing: 0.5px;"
        ).arg(Theme::TextSecondary));
        layout->addWidget(titleLbl);

        valLabel = new QLabel("--%", card);
        valLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
        layout->addWidget(valLabel);

        auto* subLbl = new QLabel(subtext, card);
        subLbl->setStyleSheet(QString("font-size: 11px; color: %1;").arg(Theme::TextMuted));
        subLbl->setWordWrap(true);
        layout->addWidget(subLbl);

        return card;
    };

    m_cpuDeltaCard = createDeltaCard("CPU Delta", m_cpuDeltaLabel, "Baseline vs Post shift", m_deltaFrame);
    m_ramDeltaCard = createDeltaCard("RAM Delta", m_ramDeltaLabel, "Percentage points difference", m_deltaFrame);
    m_ramBytesCard = createDeltaCard("RAM Net Shift", m_ramBytesDeltaLabel, "Working set volume change", m_deltaFrame);
    m_ramBytesDeltaLabel->setText("-- MB");

    deltaLayout->addLayout(m_deltaCardsGrid);

    // Comparability Warning
    m_comparabilityWarningLabel = new QLabel("", m_deltaFrame);
    m_comparabilityWarningLabel->setStyleSheet(QString("color: %1; font-weight: 500; font-size: 12px;").arg(Theme::AccentAmber));
    m_comparabilityWarningLabel->setWordWrap(true);
    m_comparabilityWarningLabel->hide();
    deltaLayout->addWidget(m_comparabilityWarningLabel);

    // Summary Interpretation
    m_summaryMessageLabel = new QLabel("Capture both a baseline and a post-optimization snapshot to evaluate measurable impact.", m_deltaFrame);
    m_summaryMessageLabel->setStyleSheet(QString("color: %1; font-size: 12px; line-height: 1.4;").arg(Theme::TextSecondary));
    m_summaryMessageLabel->setWordWrap(true);
    deltaLayout->addWidget(m_summaryMessageLabel);

    mainLayout->addWidget(m_deltaFrame);

    scrollArea->setWidget(contentWidget);
    outerLayout->addWidget(scrollArea);

    // Initial responsiveness update
    updateLayoutResponsiveness(width() > 0 ? width() : 800);
}

void AnalysisPage::updateLayoutResponsiveness(int availableWidth) {
    bool narrow = (availableWidth < 700);

    // 1. Header Layout Reflow
    if (m_headerLayout && m_titleLabel && m_btnContainer) {
        m_headerLayout->removeWidget(m_titleLabel);
        m_headerLayout->removeWidget(m_btnContainer);
        if (narrow) {
            m_headerLayout->addWidget(m_titleLabel, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
            m_headerLayout->addWidget(m_btnContainer, 1, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
            m_headerLayout->setColumnStretch(0, 1);
            m_headerLayout->setColumnStretch(1, 0);
        } else {
            m_headerLayout->addWidget(m_titleLabel, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
            m_headerLayout->addWidget(m_btnContainer, 0, 1, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
            m_headerLayout->setColumnStretch(0, 1);
            m_headerLayout->setColumnStretch(1, 0);
        }
    }

    // 2. Snapshots Grid Reflow
    if (m_snapGrid && m_baselineBox && m_postBox) {
        m_snapGrid->removeWidget(m_baselineBox);
        m_snapGrid->removeWidget(m_postBox);
        if (narrow) {
            m_snapGrid->addWidget(m_baselineBox, 0, 0);
            m_snapGrid->addWidget(m_postBox, 1, 0);
            m_snapGrid->setColumnStretch(0, 1);
            m_snapGrid->setColumnStretch(1, 0);
        } else {
            m_snapGrid->addWidget(m_baselineBox, 0, 0);
            m_snapGrid->addWidget(m_postBox, 0, 1);
            m_snapGrid->setColumnStretch(0, 1);
            m_snapGrid->setColumnStretch(1, 1);
        }
    }

    // 3. Comparison Metric Cards Reflow
    if (m_deltaCardsGrid && m_cpuDeltaCard && m_ramDeltaCard && m_ramBytesCard) {
        m_deltaCardsGrid->removeWidget(m_cpuDeltaCard);
        m_deltaCardsGrid->removeWidget(m_ramDeltaCard);
        m_deltaCardsGrid->removeWidget(m_ramBytesCard);
        if (narrow) {
            m_deltaCardsGrid->addWidget(m_cpuDeltaCard, 0, 0);
            m_deltaCardsGrid->addWidget(m_ramDeltaCard, 1, 0);
            m_deltaCardsGrid->addWidget(m_ramBytesCard, 2, 0);
            m_deltaCardsGrid->setColumnStretch(0, 1);
            m_deltaCardsGrid->setColumnStretch(1, 0);
            m_deltaCardsGrid->setColumnStretch(2, 0);
        } else {
            m_deltaCardsGrid->addWidget(m_cpuDeltaCard, 0, 0);
            m_deltaCardsGrid->addWidget(m_ramDeltaCard, 0, 1);
            m_deltaCardsGrid->addWidget(m_ramBytesCard, 0, 2);
            m_deltaCardsGrid->setColumnStretch(0, 1);
            m_deltaCardsGrid->setColumnStretch(1, 1);
            m_deltaCardsGrid->setColumnStretch(2, 1);
        }
    }

    m_isNarrowLayout = narrow;
}

void AnalysisPage::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateLayoutResponsiveness(event->size().width());
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
    m_baselineCpuBar->setValue(static_cast<int>(m_lastMetrics.cpuUsagePercent));

    m_baselineRamLabel->setText(QString::number(m_lastMetrics.ramUsagePercent, 'f', 1) + "%");
    m_baselineRamBar->setValue(static_cast<int>(m_lastMetrics.ramUsagePercent));

    m_baselineProcCountLabel->setText(QString::number(m_lastProcesses.size()) + " processes");

    updateComparisonView();
}

void AnalysisPage::onCapturePostClicked() {
    if (!m_analyzer) return;
    m_analyzer->capturePostOptimization("Post-Optimization Sample", m_lastMetrics, m_lastProcesses, m_lastProfileName);

    m_postStatusLabel->setText("Captured just now");
    m_postCpuLabel->setText(QString::number(m_lastMetrics.cpuUsagePercent, 'f', 1) + "%");
    m_postCpuBar->setValue(static_cast<int>(m_lastMetrics.cpuUsagePercent));

    m_postRamLabel->setText(QString::number(m_lastMetrics.ramUsagePercent, 'f', 1) + "%");
    m_postRamBar->setValue(static_cast<int>(m_lastMetrics.ramUsagePercent));

    m_postProcCountLabel->setText(QString::number(m_lastProcesses.size()) + " processes");

    updateComparisonView();
}

void AnalysisPage::onResetClicked() {
    if (!m_analyzer) return;
    m_analyzer->reset();

    m_baselineStatusLabel->setText("Not yet captured");
    m_baselineCpuLabel->setText("--%");
    m_baselineCpuBar->setValue(0);
    m_baselineRamLabel->setText("--%");
    m_baselineRamBar->setValue(0);
    m_baselineProcCountLabel->setText("--");

    m_postStatusLabel->setText("Not yet captured");
    m_postCpuLabel->setText("--%");
    m_postCpuBar->setValue(0);
    m_postRamLabel->setText("--%");
    m_postRamBar->setValue(0);
    m_postProcCountLabel->setText("--");

    m_cpuDeltaLabel->setText("--%");
    m_cpuDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));

    m_ramDeltaLabel->setText("--%");
    m_ramDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));

    m_ramBytesDeltaLabel->setText("-- MB");
    m_ramBytesDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));

    m_comparabilityWarningLabel->setText("");
    m_comparabilityWarningLabel->hide();

    m_summaryMessageLabel->setText("Capture both a baseline and a post-optimization snapshot to evaluate measurable impact.");
}

void AnalysisPage::updateComparisonView() {
    if (!m_analyzer) return;
    auto comp = m_analyzer->compare();
    if (!comp.has_value()) return;

    const auto& c = *comp;

    // CPU Delta
    QString cpuSign = (c.cpuDeltaPercent > 0) ? "+" : "";
    m_cpuDeltaLabel->setText(QString("%1%2%").arg(cpuSign).arg(QString::number(c.cpuDeltaPercent, 'f', 1)));
    if (c.cpuDeltaPercent < -1.0) {
        m_cpuDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::AccentGreen));
    } else if (c.cpuDeltaPercent > 1.0) {
        m_cpuDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::AccentRose));
    } else {
        m_cpuDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::TextSecondary));
    }

    // RAM Delta
    QString ramSign = (c.ramDeltaPercent > 0) ? "+" : "";
    m_ramDeltaLabel->setText(QString("%1%2%").arg(ramSign).arg(QString::number(c.ramDeltaPercent, 'f', 1)));
    if (c.ramDeltaPercent < -1.0) {
        m_ramDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::AccentGreen));
    } else if (c.ramDeltaPercent > 1.0) {
        m_ramDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::AccentAmber));
    } else {
        m_ramDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::TextSecondary));
    }

    // RAM Bytes freed/gained
    double mbShift = static_cast<double>(c.ramDeltaBytes) / (1024.0 * 1024.0);
    QString mbSign = (mbShift > 0) ? "+" : "";
    m_ramBytesDeltaLabel->setText(QString("%1%2 MB").arg(mbSign).arg(QString::number(mbShift, 'f', 0)));
    if (mbShift < -50.0) {
        m_ramBytesDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::AccentGreen));
    } else if (mbShift > 50.0) {
        m_ramBytesDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::AccentAmber));
    } else {
        m_ramBytesDeltaLabel->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(Theme::TextSecondary));
    }

    // Warning and Summary
    if (!c.comparabilityWarning.empty()) {
        m_comparabilityWarningLabel->setText(QString("⚠ Warning: ") + QString::fromStdString(c.comparabilityWarning));
        m_comparabilityWarningLabel->show();
    } else {
        m_comparabilityWarningLabel->setText("");
        m_comparabilityWarningLabel->hide();
    }

    m_summaryMessageLabel->setText(QString::fromStdString(c.summaryMessage));
}

} // namespace pulse::ui
