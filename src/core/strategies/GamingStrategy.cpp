#include "GamingStrategy.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace pulse {

std::vector<Recommendation> GamingStrategy::evaluate(
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& processes) const {

    std::vector<Recommendation> recs;

    // Check for CPU-intensive background tasks that could induce frame drops
    for (const auto& proc : processes) {
        if (proc.cpuUsagePercent > 10.0 &&
            proc.name != "System Idle Process" &&
            proc.priorityClass >= NORMAL_PRIORITY_CLASS) {

            // If it's a known non-gaming heavy process
            if (proc.name == "SearchIndexer.exe" || proc.name == "TiWorker.exe" ||
                proc.name == "OneDrive.exe" || proc.name == "Dropbox.exe") {
                Recommendation r;
                r.id = "gaming_bg_interference_" + std::to_string(proc.pid);
                r.title = "Background Contention Detected: " + proc.name;
                r.observation = "Process '" + proc.name + "' is using " +
                                std::to_string(static_cast<int>(proc.cpuUsagePercent)) + "% CPU with Normal/High priority.";
                r.diagnosis = "Background indexing or synchronization tasks preempt game render threads, causing frametime spikes.";
                r.recommendedAction = "Lower priority of '" + proc.name + "' to Below Normal or pause synchronization during gaming.";
                r.explanation = "Windows scheduler assigns CPU quantum based on thread priority. Lowering non-essential tasks prevents them from interrupting the main game loop.";
                r.benefits = "Smoother 1% and 0.1% low framerates, reduced micro-stutter.";
                r.risksAndLimitations = "File synchronization or search indexing will take longer while lower priority is active.";
                r.level = RecommendationLevel::Warning;
                r.isActionable = true;
                r.actionPayload = "deprioritize_process_" + std::to_string(proc.pid);
                recs.push_back(r);
            }
        }
    }

    // High Performance Power Plan Recommendation
    Recommendation rPower;
    rPower.id = "gaming_power_plan";
    rPower.title = "Enable High Performance Power Plan";
    rPower.observation = "Current workload profile is Gaming & High Performance.";
    rPower.diagnosis = "Power saving frequency down-stepping causes micro-latency when transitioning from low to high GPU/CPU demand scenes.";
    rPower.recommendedAction = "Switch Windows power scheme to High Performance to keep core clocks pinned at maximum state.";
    rPower.explanation = "Prevents processor C-state and P-state sleep transitions that introduce 10-50ms latency spikes.";
    rPower.benefits = "Eliminates clock transition stutters in demanding game engines.";
    rPower.risksAndLimitations = "Increases idle power draw and fan speeds slightly.";
    rPower.level = RecommendationLevel::Suggestion;
    rPower.isActionable = true;
    rPower.actionPayload = "set_high_performance_power";
    recs.push_back(rPower);

    return recs;
}

std::vector<OptimizationAction> GamingStrategy::getAvailableActions() const {
    std::vector<OptimizationAction> actions;

    OptimizationAction a1;
    a1.id = "set_high_performance_power";
    a1.title = "Activate High Performance Power Scheme";
    a1.description = "Configures the Windows power management system to maintain full CPU core frequencies and prevent core parking.";
    a1.expectedImpact = "Maximum clock frequency residency, reduced frametime latency, faster scene loading.";
    a1.rollbackDescription = "Restores previous power scheme (e.g. Balanced).";
    a1.requiresElevation = false;
    a1.canRollback = true;
    actions.push_back(a1);

    return actions;
}

} // namespace pulse
