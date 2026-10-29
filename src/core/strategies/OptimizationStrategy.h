#pragma once

#include "../Types.h"
#include <vector>
#include <string>
#include <memory>

namespace pulse {

class OptimizationStrategy {
public:
    virtual ~OptimizationStrategy() = default;

    [[nodiscard]] virtual WorkloadProfile getProfileType() const = 0;
    [[nodiscard]] virtual std::string getProfileName() const = 0;
    [[nodiscard]] virtual std::string getDescription() const = 0;
    [[nodiscard]] virtual std::string getRationale() const = 0;

    // Supported Windows power mode recommendation
    [[nodiscard]] virtual std::string getRecommendedPowerScheme() const = 0;

    // Evaluate live metrics against this strategy's criteria
    [[nodiscard]] virtual std::vector<Recommendation> evaluate(
        const SystemMetrics& sysMetrics,
        const std::vector<ProcessInfo>& processes) const = 0;

    // Supported safe actions specific to this strategy
    [[nodiscard]] virtual std::vector<OptimizationAction> getAvailableActions() const = 0;
};

} // namespace pulse

