#include "RenderingStrategy.h"

namespace pulse {

std::vector<Recommendation> RenderingStrategy::evaluate(
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& processes) const {

    std::vector<Recommendation> recs;

    // Check for high CPU rendering process
    bool hasActiveRenderer = false;
    for (const auto& proc : processes) {
        if (proc.name == "blender.exe" || proc.name == "Adobe Premiere Pro.exe" ||
            proc.name == "AfterFX.exe" || proc.name == "HandBrake.exe" ||
            proc.name == "Resolve.exe" || proc.name == "ffmpeg.exe") {
            hasActiveRenderer = true;
            if (proc.cpuUsagePercent > 40.0) {
                Recommendation r;
                r.id = "rendering_job_active_" + std::to_string(proc.pid);
                r.title = "High-Throughput Render Pipeline Active: " + proc.name;
                r.observation = "Process '" + proc.name + "' is utilizing " +
                                std::to_string(static_cast<int>(proc.cpuUsagePercent)) + "% of CPU threads.";
                r.diagnosis = "Full export/encode in progress. High CPU and fan activity is expected.";
                r.recommendedAction = "Prevent system sleep or power state changes until the render completes.";
                r.explanation = "Video encoders saturate both integer and vector/AVX execution units. Windows sleep settings may suspend overnight renders if idle timeouts are short.";
                r.benefits = "Prevents corrupted render exports and unexpected job cancellation.";
                r.risksAndLimitations = "Keeps the machine awake and drawing power until manually resumed.";
                r.level = RecommendationLevel::Info;
                r.isActionable = true;
                r.actionPayload = "prevent_sleep_during_render";
                recs.push_back(r);
            }
        }
    }

    if (sysMetrics.cpuUsagePercent > 90.0) {
        Recommendation r;
        r.id = "rendering_sustained_max_load";
        r.title = "Sustained Peak CPU Load (>90%)";
        r.observation = "Total processor load is currently at " +
                        std::to_string(static_cast<int>(sysMetrics.cpuUsagePercent)) + "%.";
        r.diagnosis = "CPU is operating at maximum thermal envelope. Normal behavior for rendering pipelines.";
        r.recommendedAction = "Ensure vents are clear and ambient ventilation is sufficient to avoid thermal throttling.";
        r.explanation = "When junction temperatures reach TjMax (~100°C), modern Intel/AMD processors downclock clock multipliers to protect silicon.";
        r.benefits = "Maintains maximum export framerates and prevents thermal downclocking.";
        r.risksAndLimitations = "Requires physical airflow maintenance.";
        r.level = RecommendationLevel::Suggestion;
        r.isActionable = false;
        recs.push_back(r);
    }

    return recs;
}

std::vector<OptimizationAction> RenderingStrategy::getAvailableActions() const {
    std::vector<OptimizationAction> actions;

    OptimizationAction a1;
    a1.id = "set_high_performance_power";
    a1.title = "Enable High Performance Power Profile";
    a1.description = "Maintains uninterrupted peak core frequencies for continuous render throughput.";
    a1.expectedImpact = "Maximum render speed, faster frame export times.";
    a1.rollbackDescription = "Restores previous power scheme.";
    a1.requiresElevation = false;
    a1.canRollback = true;
    actions.push_back(a1);

    OptimizationAction a2;
    a2.id = "prevent_sleep_during_render";
    a2.title = "Prevent Sleep While Rendering (SetThreadExecutionState)";
    a2.description = "Informs Windows power manager that continuous background processing is active, preventing sleep or screen-lock timeouts during exports.";
    a2.expectedImpact = "Long-running renders will finish without interruption.";
    a2.rollbackDescription = "Releases execution state flag.";
    a2.requiresElevation = false;
    a2.canRollback = true;
    actions.push_back(a2);

    return actions;
}

} // namespace pulse

