#include "BatteryStrategy.h"

namespace pulse {

std::vector<Recommendation> BatteryStrategy::evaluate(
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& processes) const {

    std::vector<Recommendation> recs;

    // Suggest switching to Power Saver scheme
    Recommendation rPower;
    rPower.id = "battery_power_saver";
    rPower.title = "Switch to Windows Power Saver Scheme";
    rPower.observation = "Battery Saving profile is active.";
    rPower.diagnosis = "Default power plans allow aggressive CPU turbo boosting even during minor background events.";
    rPower.recommendedAction = "Activate the Power Saver power scheme to cap maximum CPU frequency and prioritize power states.";
    rPower.explanation = "Power consumption scales quadratically with voltage and frequency (P ~ C * V^2 * f). Limiting turbo peaks saves battery disproportionately.";
    rPower.benefits = "Extends laptop battery life by 20% - 40% depending on display and background load.";
    rPower.risksAndLimitations = "Intensive operations like video decompression or app launch will take slightly longer.";
    rPower.level = RecommendationLevel::Suggestion;
    rPower.isActionable = true;
    rPower.actionPayload = "set_power_saver_scheme";
    recs.push_back(rPower);

    // Identify runaway background processes draining battery
    for (const auto& proc : processes) {
        if (proc.cpuUsagePercent > 8.0 && proc.name != "System Idle Process") {
            Recommendation r;
            r.id = "battery_drain_" + std::to_string(proc.pid);
            r.title = "Background Power Drain: " + proc.name;
            r.observation = "Process '" + proc.name + "' is using " +
                            std::to_string(static_cast<int>(proc.cpuUsagePercent)) + "% CPU continuously.";
            r.diagnosis = "Background task prevents CPU package from settling into low-power C8/C10 sleep states.";
            r.recommendedAction = "Review or pause '" + proc.name + "' while on battery.";
            r.explanation = "Even an 8% background load keeps the memory controller and core voltages elevated, multiplying baseline package wattage.";
            r.benefits = "Prevents premature battery depletion.";
            r.risksAndLimitations = "Pausing background work defers sync until AC power is reconnected.";
            r.level = RecommendationLevel::Warning;
            r.isActionable = false;
            recs.push_back(r);
        }
    }

    return recs;
}

std::vector<OptimizationAction> BatteryStrategy::getAvailableActions() const {
    std::vector<OptimizationAction> actions;

    OptimizationAction a1;
    a1.id = "set_power_saver_scheme";
    a1.title = "Enable Power Saver Power Scheme";
    a1.description = "Configures Windows to favor energy efficiency, reduce display idle timer, and suppress aggressive processor boosting.";
    a1.expectedImpact = "Significantly reduced wattage draw and cooler battery temperature.";
    a1.rollbackDescription = "Restores previous power scheme.";
    a1.requiresElevation = false;
    a1.canRollback = true;
    actions.push_back(a1);

    return actions;
}

} // namespace pulse

