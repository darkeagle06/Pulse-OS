#pragma once

#include "Types.h"
#include <optional>
#include <vector>
#include <string>

namespace pulse {

class PerformanceAnalyzer {
public:
    PerformanceAnalyzer() = default;
    ~PerformanceAnalyzer() = default;

    // Capture baseline metrics snapshot
    void captureBaseline(const std::string& label,
                         const SystemMetrics& metrics,
                         const std::vector<ProcessInfo>& processes,
                         const std::string& profileName);

    // Capture post-optimization metrics snapshot
    void capturePostOptimization(const std::string& label,
                                const SystemMetrics& metrics,
                                const std::vector<ProcessInfo>& processes,
                                const std::string& profileName);

    // Compute comparison between baseline and post-optimization
    [[nodiscard]] std::optional<ComparisonResult> compare() const;

    // Snapshot accessors
    [[nodiscard]] std::optional<PerformanceSnapshot> getBaseline() const { return m_baseline; }
    [[nodiscard]] std::optional<PerformanceSnapshot> getPostOptimization() const { return m_postOptimization; }

    void reset();

private:
    std::optional<PerformanceSnapshot> m_baseline;
    std::optional<PerformanceSnapshot> m_postOptimization;
};

} // namespace pulse

