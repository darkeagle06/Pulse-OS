#pragma once

#include "Types.h"
#include "strategies/OptimizationStrategy.h"
#include <vector>
#include <memory>
#include <deque>

namespace pulse {

class RecommendationEngine {
public:
    RecommendationEngine();
    ~RecommendationEngine() = default;

    // Generate comprehensive recommendations based on system state and active profile
    [[nodiscard]] std::vector<Recommendation> generateRecommendations(
        const SystemMetrics& sysMetrics,
        const std::vector<ProcessInfo>& processes,
        const std::shared_ptr<OptimizationStrategy>& activeStrategy);

    // Enumerate startup applications from Windows Registry
    struct StartupItem {
        std::string name;
        std::string command;
        std::string source; // HKCU or HKLM
    };
    [[nodiscard]] static std::vector<StartupItem> queryStartupApplications();

    // Query active Windows power scheme GUID
    [[nodiscard]] static std::string getActivePowerSchemeGuid();

private:
    void updateCpuHistory(double currentCpu);

    std::deque<double> m_recentCpuSamples;
    static constexpr size_t SustainedWindowSize = 10; // 10 samples
};

} // namespace pulse

