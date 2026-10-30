#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QGridLayout>
#include <QFrame>
#include <QScrollArea>
#include <memory>

#include "../../core/Types.h"
#include "../../core/PerformanceAnalyzer.h"

namespace pulse::ui {

class AnalysisPage : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisPage(std::shared_ptr<PerformanceAnalyzer> analyzer, QWidget* parent = nullptr);
    ~AnalysisPage() override = default;

    void updateLiveMetrics(const SystemMetrics& metrics,
                           const std::vector<ProcessInfo>& processes,
                           const std::string& profileName);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onCaptureBaselineClicked();
    void onCapturePostClicked();
    void onResetClicked();

private:
    void setupUi();
    void updateComparisonView();
    void updateLayoutResponsiveness(int availableWidth);

    std::shared_ptr<PerformanceAnalyzer> m_analyzer;

    // Cached live telemetry
    SystemMetrics m_lastMetrics;
    std::vector<ProcessInfo> m_lastProcesses;
    std::string m_lastProfileName;

    // Header & Actions
    QGridLayout* m_headerLayout{nullptr};
    QLabel* m_titleLabel{nullptr};
    QWidget* m_btnContainer{nullptr};
    QPushButton* m_captureBaselineBtn{nullptr};
    QPushButton* m_capturePostBtn{nullptr};
    QPushButton* m_resetBtn{nullptr};

    // Methodology banner
    QFrame* m_methodFrame{nullptr};

    // Baseline & Post Snapshot Grid
    QGridLayout* m_snapGrid{nullptr};
    QFrame* m_baselineBox{nullptr};
    QFrame* m_postBox{nullptr};

    // Baseline details
    QLabel* m_baselineStatusLabel{nullptr};
    QLabel* m_baselineCpuLabel{nullptr};
    QProgressBar* m_baselineCpuBar{nullptr};
    QLabel* m_baselineRamLabel{nullptr};
    QProgressBar* m_baselineRamBar{nullptr};
    QLabel* m_baselineProcCountLabel{nullptr};

    // Post-optimization details
    QLabel* m_postStatusLabel{nullptr};
    QLabel* m_postCpuLabel{nullptr};
    QProgressBar* m_postCpuBar{nullptr};
    QLabel* m_postRamLabel{nullptr};
    QProgressBar* m_postRamBar{nullptr};
    QLabel* m_postProcCountLabel{nullptr};

    // Comparison & Delta Card
    QFrame* m_deltaFrame{nullptr};
    QGridLayout* m_deltaCardsGrid{nullptr};
    QFrame* m_cpuDeltaCard{nullptr};
    QFrame* m_ramDeltaCard{nullptr};
    QFrame* m_ramBytesCard{nullptr};

    QLabel* m_cpuDeltaLabel{nullptr};
    QLabel* m_ramDeltaLabel{nullptr};
    QLabel* m_ramBytesDeltaLabel{nullptr};
    QLabel* m_comparabilityWarningLabel{nullptr};
    QLabel* m_summaryMessageLabel{nullptr};

    // State tracking for responsive layout transitions
    bool m_isNarrowLayout{false};
};

} // namespace pulse::ui
