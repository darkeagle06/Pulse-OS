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

// --- MODULE 1: Installed Application Manager Types ---

enum class AppCategory {
    All,
    PotentiallyUnused,
    Large,
    RecentlyInstalled,
    UnknownPublisher
};

inline std::string appCategoryToString(AppCategory category) {
    switch (category) {
        case AppCategory::All: return "All Applications";
        case AppCategory::PotentiallyUnused: return "Potentially Unused";
        case AppCategory::Large: return "Large Applications";
        case AppCategory::RecentlyInstalled: return "Recently Installed";
        case AppCategory::UnknownPublisher: return "Unknown / Unverified";
        default: return "Unknown";
    }
}

struct InstalledAppInfo {
    std::string id;                    // Registry subkey name
    std::string displayName;           // DisplayName
    std::string name;                  // Name (alias for displayName)
    std::string publisher;             // Publisher
    std::string displayVersion;        // DisplayVersion
    std::string version;               // Version (alias for displayVersion)
    std::string installDate;           // Formatted date (e.g. "2026-05-18")
    std::string installDateRaw;        // Raw registry string (e.g. "20260518")
    uint64_t estimatedSize{0};         // Bytes (derived from EstimatedSize in KB)
    uint64_t estimatedSizeBytes{0};    // Bytes (alias for estimatedSize)
    std::string installLocation;       // InstallLocation
    std::string uninstallString;       // UninstallString
    std::string quietUninstallString;  // QuietUninstallString
    std::string registryRoot;          // "HKLM", "HKLM_32", or "HKCU"
    bool isSystemComponent{false};     // SystemComponent == 1
    bool isWindowsUpdate{false};       // ParentKeyName present or KB-prefix
    bool canUninstall{false};          // Valid uninstall string & not protected
    AppCategory category{AppCategory::All};
    std::string usageEvidence;         // Measured factual evidence (e.g., "Last modified 120 days ago; zero running processes")
    std::chrono::system_clock::time_point lastWriteTime;
    bool isCurrentlyRunning{false};    // Has an active process in the current process snapshot

    std::string formattedSize() const {
        uint64_t sz = estimatedSize ? estimatedSize : estimatedSizeBytes;
        if (sz == 0) return "--";
        double mb = static_cast<double>(sz) / (1024.0 * 1024.0);
        if (mb >= 1024.0) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f GB", mb / 1024.0);
            return std::string(buf);
        }
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f MB", mb);
        return std::string(buf);
    }
};

struct UninstallResult {
    bool success{false};
    uint32_t exitCode{0};
    std::string message;
};

// --- MODULE 2: Smart Resource Alerts & Notifications Types ---

enum class AlertType {
    HighMemoryUsage,
    HighCpuProcess,
    HeavyBackground,
    IdleBackground,
    MemoryPressure
};

inline std::string alertTypeToString(AlertType type) {
    switch (type) {
        case AlertType::HighMemoryUsage: return "High Memory Usage";
        case AlertType::HighCpuProcess: return "High CPU Consumption";
        case AlertType::HeavyBackground: return "Resource-Heavy Background App";
        case AlertType::IdleBackground: return "Potentially Idle Background App";
        case AlertType::MemoryPressure: return "Critical System Memory Pressure";
        default: return "System Alert";
    }
}

enum class AlertSeverity {
    Info,
    Warning,
    Critical
};

struct ResourceAlert {
    std::string id;
    AlertType type{AlertType::HighCpuProcess};
    AlertSeverity severity{AlertSeverity::Warning};
    std::string title;
    std::string observation;           // Objective measured fact
    std::string diagnosis;             // Technical diagnosis
    std::string suggestedAction;       // Recommended user action
    std::string targetProcessName;     // Executable name (if process-specific)
    uint32_t targetPid{0};
    double triggerValue{0.0};          // Triggering metric value (CPU% or RAM%)
    std::chrono::system_clock::time_point timestamp;
    uint32_t sustainedSeconds{0};      // How long the condition has persisted
    bool isSnoozed{false};
    bool isDismissed{false};
};

struct AlertSettings {
    bool enableHighMemory{true};
    bool enableHighCpuProcess{true};
    bool enableHeavyBackground{true};
    bool enableIdleBackground{true};
    bool enableMemoryPressure{true};

    double systemRamThresholdPercent{85.0};
    double processCpuThresholdPercent{30.0};
    double heavyBackgroundCpuPercent{15.0};
    uint64_t heavyBackgroundRamBytes{1024ULL * 1024 * 1024}; // 1 GB
    uint32_t sustainedDurationSeconds{15};                   // 15 seconds sustained
    uint32_t cooldownDurationSeconds{300};                   // 5 minutes cooldown between repeat alerts

    std::vector<std::string> ignoreList;                    // Process names never to alert on
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
Q_DECLARE_METATYPE(pulse::InstalledAppInfo)
Q_DECLARE_METATYPE(std::vector<pulse::InstalledAppInfo>)
Q_DECLARE_METATYPE(pulse::ResourceAlert)
Q_DECLARE_METATYPE(std::vector<pulse::ResourceAlert>)
#endif
