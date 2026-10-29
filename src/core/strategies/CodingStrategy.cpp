#include "CodingStrategy.h"

namespace pulse {

std::vector<Recommendation> CodingStrategy::evaluate(
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& processes) const {

    std::vector<Recommendation> recs;

    // Check for build processes contending for CPU
    for (const auto& proc : processes) {
        if (proc.name == "cl.exe" || proc.name == "g++.exe" || proc.name == "clang++.exe" ||
            proc.name == "ninja.exe" || proc.name == "msbuild.exe") {
            if (proc.cpuUsagePercent > 50.0) {
                Recommendation r;
                r.id = "coding_active_build_" + std::to_string(proc.pid);
                r.title = "Active Compilation Job Detected: " + proc.name;
                r.observation = "Process '" + proc.name + "' (PID " + std::to_string(proc.pid) +
                                ") is utilizing " + std::to_string(static_cast<int>(proc.cpuUsagePercent)) + "% of total CPU capacity.";
                r.diagnosis = "A parallel compiler invocation is actively building source code. High CPU during builds is expected and desired.";
                r.recommendedAction = "Allow build to complete. If the system feels sluggish, consider limiting parallel build jobs (-j flag in Ninja/Make) or setting priority to Below Normal.";
                r.explanation = "Modern C++ compilers generate heavy AST and code-generation work across all CPU cores. This utilizes available thermal headroom by design.";
                r.benefits = "Maintains system responsiveness during long-running builds without prematurely terminating compilation.";
                r.risksAndLimitations = "Deprioritizing the compiler slightly increases total build duration.";
                r.level = RecommendationLevel::Info;
                r.isActionable = false;
                recs.push_back(r);
            }
        }

        // Heavy background processes during coding
        if ((proc.name == "chrome.exe" || proc.name == "msedge.exe") && proc.workingSetBytes > 2ULL * 1024 * 1024 * 1024) {
            Recommendation r;
            r.id = "coding_heavy_browser_" + std::to_string(proc.pid);
            r.title = "High Memory Browser Instance During Development";
            r.observation = "Browser process '" + proc.name + "' (PID " + std::to_string(proc.pid) +
                            ") is consuming " + std::to_string(proc.workingSetBytes / (1024 * 1024)) + " MB of RAM.";
            r.diagnosis = "Large number of open documentation or media tabs may restrict memory headroom for language servers and Docker.";
            r.recommendedAction = "Review unused browser tabs or enable browser memory-saver mode.";
            r.explanation = "Compilers and IDE indexers require fast heap allocation. Swapping or high RAM pressure degrades language server response times.";
            r.benefits = "Frees physical RAM for build toolchains and debugger heap.";
            r.risksAndLimitations = "Discarded tabs may need reloading when revisited.";
            r.level = RecommendationLevel::Suggestion;
            r.isActionable = false;
            recs.push_back(r);
        }
    }

    // High RAM pressure check
    if (sysMetrics.ramUsagePercent > 80.0) {
        Recommendation r;
        r.id = "coding_ram_pressure";
        r.title = "Memory Headroom Constrained for Build Tools";
        r.observation = "Total system memory usage is currently at " +
                        std::to_string(static_cast<int>(sysMetrics.ramUsagePercent)) + "%.";
        r.diagnosis = "High memory usage reduces file system cache buffers and may cause build tool page faults.";
        r.recommendedAction = "Close inactive background applications or trim memory working sets.";
        r.explanation = "The Windows standby list and page cache accelerate repeated header parsing during compilation.";
        r.benefits = "Faster header include processing in multi-file C++ compilations.";
        r.risksAndLimitations = "Trimming working sets causes a momentary brief read penalty when trimmed pages are recalled.";
        r.level = RecommendationLevel::Warning;
        r.isActionable = true;
        r.actionPayload = "trim_working_sets";
        recs.push_back(r);
    }

    return recs;
}

std::vector<OptimizationAction> CodingStrategy::getAvailableActions() const {
    std::vector<OptimizationAction> actions;

    OptimizationAction a1;
    a1.id = "set_balanced_power";
    a1.title = "Switch to Windows Balanced Power Scheme";
    a1.description = "Configures the Windows power subsystem to dynamic frequency scaling, allowing maximum clock boost during builds and low idle power while editing.";
    a1.expectedImpact = "Fast single-core editing responsiveness, full boost during compilation, reduced fan noise during idle typing.";
    a1.rollbackDescription = "Restores previous power scheme GUID.";
    a1.requiresElevation = false;
    a1.canRollback = true;
    actions.push_back(a1);

    OptimizationAction a2;
    a2.id = "trim_working_sets";
    a2.title = "Clean Unused Process Working Sets";
    a2.description = "Requests the Windows memory manager to release unreferenced private pages from idle background processes back to the standby list.";
    a2.expectedImpact = "Recovers 200MB - 1GB of physical RAM for immediate allocation by build tools.";
    a2.rollbackDescription = "No rollback required; pages naturally repopulate into RAM upon active reference.";
    a2.requiresElevation = false;
    a2.canRollback = false;
    actions.push_back(a2);

    return actions;
}

} // namespace pulse

