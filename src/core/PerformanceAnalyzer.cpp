#include "PerformanceAnalyzer.h"

#include <cmath>
#include <sstream>
#include <iomanip>

namespace pulse {

void PerformanceAnalyzer::captureBaseline(
    const std::string& label,
    const SystemMetrics& metrics,
    const std::vector<ProcessInfo>& processes,
    const std::string& profileName) {

    PerformanceSnapshot snap;
    snap.label = label.empty() ? "Baseline Snapshot" : label;
    snap.timestamp = std::chrono::system_clock::now();
    snap.averageCpuPercent = metrics.cpuUsagePercent;
    snap.peakCpuPercent = metrics.cpuUsagePercent;
    snap.averageRamPercent = metrics.ramUsagePercent;
    snap.averageUsedRamBytes = metrics.usedRamBytes;
    snap.sampledProcessCount = static_cast<uint32_t>(processes.size());
    snap.activeProfileName = profileName;

    m_baseline = snap;
}

void PerformanceAnalyzer::capturePostOptimization(
    const std::string& label,
    const SystemMetrics& metrics,
    const std::vector<ProcessInfo>& processes,
    const std::string& profileName) {

    PerformanceSnapshot snap;
    snap.label = label.empty() ? "Post-Optimization Snapshot" : label;
    snap.timestamp = std::chrono::system_clock::now();
    snap.averageCpuPercent = metrics.cpuUsagePercent;
    snap.peakCpuPercent = metrics.cpuUsagePercent;
    snap.averageRamPercent = metrics.ramUsagePercent;
    snap.averageUsedRamBytes = metrics.usedRamBytes;
    snap.sampledProcessCount = static_cast<uint32_t>(processes.size());
    snap.activeProfileName = profileName;

    m_postOptimization = snap;
}

std::optional<ComparisonResult> PerformanceAnalyzer::compare() const {
    if (!m_baseline.has_value() || !m_postOptimization.has_value()) {
        return std::nullopt;
    }

    ComparisonResult res;
    res.baseline = *m_baseline;
    res.postOptimization = *m_postOptimization;

    // Delta = Post - Baseline (negative means reduction, e.g. lower CPU or lower RAM)
    res.cpuDeltaPercent = res.postOptimization.averageCpuPercent - res.baseline.averageCpuPercent;
    res.ramDeltaPercent = res.postOptimization.averageRamPercent - res.baseline.averageRamPercent;
    res.ramDeltaBytes = static_cast<int64_t>(res.postOptimization.averageUsedRamBytes) -
                        static_cast<int64_t>(res.baseline.averageUsedRamBytes);

    // Comparability verification
    int procDelta = std::abs(static_cast<int>(res.postOptimization.sampledProcessCount) -
                             static_cast<int>(res.baseline.sampledProcessCount));

    if (procDelta > 15) {
        res.workloadAppearsComparable = false;
        res.comparabilityWarning = "Warning: Active process count shifted by " +
            std::to_string(procDelta) + " processes between snapshots. "
            "Background workload changes may account for measured variance rather than the optimization itself.";
    } else {
        res.workloadAppearsComparable = true;
    }

    // Summary interpretation
    std::ostringstream oss;
    if (std::abs(res.cpuDeltaPercent) < 1.0 && std::abs(res.ramDeltaPercent) < 1.0) {
        oss << "No statistically significant change in resource utilization detected (CPU delta: "
            << std::fixed << std::setprecision(1) << res.cpuDeltaPercent << "%, RAM delta: "
            << res.ramDeltaPercent << "%). Current workload was already operating efficiently.";
    } else {
        if (res.cpuDeltaPercent < -2.0) {
            oss << "Measured " << std::fixed << std::setprecision(1) << -res.cpuDeltaPercent
                << "% reduction in overall CPU utilization. ";
        } else if (res.cpuDeltaPercent > 2.0) {
            oss << "Measured " << std::fixed << std::setprecision(1) << res.cpuDeltaPercent
                << "% increase in CPU utilization. ";
        }

        if (res.ramDeltaBytes < 0) {
            double freedMb = static_cast<double>(-res.ramDeltaBytes) / (1024.0 * 1024.0);
            oss << "Reclaimed ~" << static_cast<int>(freedMb) << " MB of physical RAM. ";
        }
    }
    res.summaryMessage = oss.str();

    return res;
}

void PerformanceAnalyzer::reset() {
    m_baseline.reset();
    m_postOptimization.reset();
}

} // namespace pulse

