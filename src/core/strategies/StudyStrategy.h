#pragma once

#include "OptimizationStrategy.h"

namespace pulse {

class StudyStrategy : public OptimizationStrategy {
public:
    StudyStrategy() = default;
    ~StudyStrategy() override = default;

    [[nodiscard]] WorkloadProfile getProfileType() const override {
        return WorkloadProfile::Study;
    }

    [[nodiscard]] std::string getProfileName() const override {
        return "Browsing & Studying";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "Optimized for quiet, cool, and distraction-free document reading, browsing, note taking, and research.";
    }

    [[nodiscard]] std::string getRationale() const override {
        return "Document viewing, research browsers, and PDF readers require minimal compute power. "
               "The strategy emphasizes cool thermal states, quiet acoustic profiles (low fan speeds), and power efficiency.";
    }

    [[nodiscard]] std::string getRecommendedPowerScheme() const override {
        return "381b4222-f694-41f0-9685-ff5bb260df2e"; // Balanced
    }

    [[nodiscard]] std::vector<Recommendation> evaluate(
        const SystemMetrics& sysMetrics,
        const std::vector<ProcessInfo>& processes) const override;

    [[nodiscard]] std::vector<OptimizationAction> getAvailableActions() const override;
};

} // namespace pulse

