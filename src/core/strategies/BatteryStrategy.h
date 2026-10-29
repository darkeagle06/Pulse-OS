#pragma once

#include "OptimizationStrategy.h"

namespace pulse {

class BatteryStrategy : public OptimizationStrategy {
public:
    BatteryStrategy() = default;
    ~BatteryStrategy() override = default;

    [[nodiscard]] WorkloadProfile getProfileType() const override {
        return WorkloadProfile::BatterySaver;
    }

    [[nodiscard]] std::string getProfileName() const override {
        return "Battery Saving";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "Conserves watt-hours, curbs thermal emission, and prioritizes power efficiency on portable laptops.";
    }

    [[nodiscard]] std::string getRationale() const override {
        return "Modern high-performance processors can consume upwards of 45-80W under turbo boost. "
               "The strategy constrains background burst states, facilitates deep core sleep states (C-states), and trims unnecessary background resource draw.";
    }

    [[nodiscard]] std::string getRecommendedPowerScheme() const override {
        return "a1841308-3541-4fab-bc81-f71556f20b4a"; // Power Saver
    }

    [[nodiscard]] std::vector<Recommendation> evaluate(
        const SystemMetrics& sysMetrics,
        const std::vector<ProcessInfo>& processes) const override;

    [[nodiscard]] std::vector<OptimizationAction> getAvailableActions() const override;
};

} // namespace pulse

