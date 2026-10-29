#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
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

private slots:
    void onCaptureBaselineClicked();
    void onCapturePostClicked();
    void onResetClicked();

private:
    void setupUi();
    void updateComparisonView();

    std::shared_ptr<PerformanceAnalyzer> m_analyzer;

    // Cached live data
    SystemMetrics m_lastMetrics;
    std::vector<ProcessInfo> m_lastProcesses;
    std::string m_lastProfileName;

    // Buttons
    QPushButton* m_captureBaselineBtn{nullptr};
    QPushButton* m_capturePostBtn{nullptr};
    QPushButton* m_resetBtn{nullptr};

    // Baseline Card
    QLabel* m_baselineStatusLabel{nullptr};
    QLabel* m_baselineCpuLabel{nullptr};
    QLabel* m_baselineRamLabel{nullptr};
    QLabel* m_baselineProcCountLabel{nullptr};

    // Post Card
    QLabel* m_postStatusLabel{nullptr};
    QLabel* m_postCpuLabel{nullptr};
    QLabel* m_postRamLabel{nullptr};
    QLabel* m_postProcCountLabel{nullptr};

    // Delta Card
    QLabel* m_cpuDeltaLabel{nullptr};
    QLabel* m_ramDeltaLabel{nullptr};
    QLabel* m_ramBytesDeltaLabel{nullptr};
    QLabel* m_comparabilityWarningLabel{nullptr};
    QLabel* m_summaryMessageLabel{nullptr};
};

} // namespace pulse::ui

