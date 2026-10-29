#pragma once

#include "OptimizationStrategy.h"

namespace pulse {

class GamingStrategy : public OptimizationStrategy {
public:
    GamingStrategy() = default;
    ~GamingStrategy() override = default;

    [[nodiscard]] WorkloadProfile getProfileType() const override {
        return WorkloadProfile::Gaming;
    }

    [[nodiscard]] std::string getProfileName() const override {
        return "Gaming & High Performance";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "Tuned for low frametime variance, maximum CPU core clock residency, and minimal background scheduling contention.";
    }

    [[nodiscard]] std::string getRationale() const override {
        return "Real-time games are highly sensitive to scheduling latency and micro-stutter caused by background thread preemption. "
               "The strategy emphasizes uninterrupted core residency for the primary game thread and sustained CPU frequency.";
    }

    [[nodiscard]] std::string getRecommendedPowerScheme() const override {
        return "8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c"; // High Performance
    }

    [[nodiscard]] std::vector<Recommendation> evaluate(
        const SystemMetrics& sysMetrics,
        const std::vector<ProcessInfo>& processes) const override;

    [[nodiscard]] std::vector<OptimizationAction> getAvailableActions() const override;
};

} // namespace pulse

