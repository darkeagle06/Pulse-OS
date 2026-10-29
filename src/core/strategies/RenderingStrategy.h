#pragma once

#include "OptimizationStrategy.h"

namespace pulse {

class RenderingStrategy : public OptimizationStrategy {
public:
    RenderingStrategy() = default;
    ~RenderingStrategy() override = default;

    [[nodiscard]] WorkloadProfile getProfileType() const override {
        return WorkloadProfile::Rendering;
    }

    [[nodiscard]] std::string getProfileName() const override {
        return "Video Editing & Rendering";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "Tailored for heavy export, encoding, 3D raytracing, and video rendering workloads (Premiere, Blender, Handbrake, DaVinci).";
    }

    [[nodiscard]] std::string getRationale() const override {
        return "Render tasks sustain 100% all-core load across all execution units for hours. "
               "The strategy prioritizes continuous system wakefulness, maximum thread throughput, and thermal stability.";
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

