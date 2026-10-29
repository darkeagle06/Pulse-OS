#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <optional>

namespace pulse {

// Workload profile identifiers
enum class WorkloadProfile {
    Coding,
    Gaming,
    Study,
    Rendering,
    BatterySaver,
    AutoDetected
};

inline std::string profileToString(WorkloadProfile profile) {
    switch (profile) {
        case WorkloadProfile::Coding: return "Coding & Development";
        case WorkloadProfile::Gaming: return "Gaming & High Performance";
        case WorkloadProfile::Study: return "Browsing & Studying";
        case WorkloadProfile::Rendering: return "Video Editing & Rendering";
        case WorkloadProfile::BatterySaver: return "Battery Saving";
        case WorkloadProfile::AutoDetected: return "Auto-Detected";
        default: return "Unknown";
    }
}

// System-wide live metrics
struct SystemMetrics {
    double cpuUsagePercent{0.0};       // 0.0 - 100.0%
    uint64_t totalRamBytes{0};
    uint64_t usedRamBytes{0};
    uint64_t availableRamBytes{0};
    double ramUsagePercent{0.0};       // 0.0 - 100.0%
    uint64_t uptimeSeconds{0};
    uint32_t activeProcessCount{0};
    uint32_t activeThreadCount{0};
    std::string cpuName;
    uint32_t cpuPhysicalCores{0};
    uint32_t cpuLogicalProcessors{0};
    std::chrono::system_clock::time_point timestamp;
    bool isValid{false};
    std::string errorMessage;
};

// Process access status
enum class ProcessAccessStatus {
    Accessible,
    AccessDenied,
    Terminated,
    SystemProtected
};

// Information and metrics for an individual process
struct ProcessInfo {
    uint32_t pid{0};
    std::string name;
    std::string executablePath;
    double cpuUsagePercent{0.0};       // 0.0 - 100.0% (normalized to total CPU capacity)
    uint64_t workingSetBytes{0};       // Physical RAM used
    uint64_t privateBytes{0};          // Private commit
    uint32_t threadCount{0};
    ProcessAccessStatus accessStatus{ProcessAccessStatus::Accessible};
    std::string priorityClassString{"Normal"};
    uint32_t priorityClass{0};
    bool isElevated{false};
};

// Detection result from TaskDetector
struct TaskDetectionResult {
    WorkloadProfile detectedProfile{WorkloadProfile::Study};
    double confidence{0.0};            // 0.0 to 1.0 (0% - 100%)
    std::string primaryEvidence;
    std::vector<std::string> contributingFactors;
    std::string foregroundProcessName;
    std::string foregroundWindowTitle;
    std::chrono::system_clock::time_point timestamp;
};

// Recommendation severity/urgency
enum class RecommendationLevel {
    Info,
    Suggestion,
    Warning,
    Critical
};

// Single recommendation item
struct Recommendation {
    std::string id;
    std::string title;
    std::string observation;           // Objective measured fact
    std::string diagnosis;             // Interpretation (clearly distinct from observation)
    std::string recommendedAction;     // What the user can do
    std::string explanation;           // Underlying mechanism
    std::string benefits;              // Potential advantages
    std::string risksAndLimitations;   // Possible downsides, trade-offs
    RecommendationLevel level{RecommendationLevel::Suggestion};
    bool isActionable{false};          // Can be applied via Safe Action Center
    std::string actionPayload;         // Internal action identifier if actionable
};

// Safe optimization action details
struct OptimizationAction {
    std::string id;
    std::string title;
    std::string description;
    std::string expectedImpact;
    std::string rollbackDescription;
    bool requiresElevation{false};
    bool isApplied{false};
    bool canRollback{true};
    std::string previousStateValue;
    std::string targetStateValue;
    std::chrono::system_clock::time_point appliedTimestamp;
};

// Performance comparison snapshot
struct PerformanceSnapshot {
    std::string label;
    std::chrono::system_clock::time_point timestamp;
    double averageCpuPercent{0.0};
    double peakCpuPercent{0.0};
    double averageRamPercent{0.0};
    uint64_t averageUsedRamBytes{0};
    uint32_t sampledProcessCount{0};
    size_t sampleCount{0};
    std::string activeProfileName;
};

// Comparison result between two snapshots
struct ComparisonResult {
    PerformanceSnapshot baseline;
    PerformanceSnapshot postOptimization;
    double cpuDeltaPercent{0.0};       // negative = reduction in CPU (improvement)
    double ramDeltaPercent{0.0};       // negative = reduction in RAM (improvement)
    int64_t ramDeltaBytes{0};
    bool workloadAppearsComparable{true};
    std::string comparabilityWarning;
    std::string summaryMessage;
};

} // namespace pulse

#ifdef QT_CORE_LIB
#include <QMetaType>
Q_DECLARE_METATYPE(pulse::WorkloadProfile)
Q_DECLARE_METATYPE(pulse::SystemMetrics)
Q_DECLARE_METATYPE(pulse::ProcessInfo)
Q_DECLARE_METATYPE(std::vector<pulse::ProcessInfo>)
Q_DECLARE_METATYPE(pulse::TaskDetectionResult)
Q_DECLARE_METATYPE(pulse::Recommendation)
Q_DECLARE_METATYPE(std::vector<pulse::Recommendation>)
#endif


