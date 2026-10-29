#pragma once

#include "OptimizationStrategy.h"

namespace pulse {

class CodingStrategy : public OptimizationStrategy {
public:
    CodingStrategy() = default;
    ~CodingStrategy() override = default;

    [[nodiscard]] WorkloadProfile getProfileType() const override {
        return WorkloadProfile::Coding;
    }

    [[nodiscard]] std::string getProfileName() const override {
        return "Coding & Development";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "Optimized for software development: quick compiler burst responsiveness, multi-threaded builds, and language server performance.";
    }

    [[nodiscard]] std::string getRationale() const override {
        return "Compilers, language servers, and dev containers exhibit bursty, highly parallel CPU utilization interspersed with idle editing periods. "
               "The strategy prioritizes low scheduling latency for active compilation while keeping background resource consumption moderate.";
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

