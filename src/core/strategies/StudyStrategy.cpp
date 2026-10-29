#include "StudyStrategy.h"

namespace pulse {

std::vector<Recommendation> StudyStrategy::evaluate(
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& /*processes*/) const {

    std::vector<Recommendation> recs;

    if (sysMetrics.cpuUsagePercent > 40.0) {
        Recommendation r;
        r.id = "study_unusual_cpu";
        r.title = "Elevated CPU Load During Study/Reading Profile";
        r.observation = "Total CPU utilization is " +
                        std::to_string(static_cast<int>(sysMetrics.cpuUsagePercent)) +
                        "%, which is unusually high for text/study tasks.";
        r.diagnosis = "Background update tasks, heavy web scripts, or video decoders are consuming unnecessary processor power.";
        r.recommendedAction = "Inspect top processes in Process Explorer or enable Power Saver mode to limit fan noise.";
        r.explanation = "Study workflows normally average <10% CPU usage. Sustained load triggers thermal dissipation and audible fan noise.";
        r.benefits = "Quiet operation, longer battery life on laptops, cooler laptop chassis.";
        r.risksAndLimitations = "Capping CPU frequencies might make heavy media pages load slightly slower.";
        r.level = RecommendationLevel::Suggestion;
        r.isActionable = false;
        recs.push_back(r);
    }

    return recs;
}

std::vector<OptimizationAction> StudyStrategy::getAvailableActions() const {
    std::vector<OptimizationAction> actions;

    OptimizationAction a1;
    a1.id = "set_balanced_power";
    a1.title = "Ensure Balanced Power Profile";
    a1.description = "Maintains optimal fan acoustic curve and allows CPU to enter low-power sleep states when idling.";
    a1.expectedImpact = "Lower operating temperature, reduced fan noise, extended battery runtime.";
    a1.rollbackDescription = "Restores previous power scheme.";
    a1.requiresElevation = false;
    a1.canRollback = true;
    actions.push_back(a1);

    return actions;
}

} // namespace pulse

