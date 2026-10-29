#include "ReportExporter.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace pulse {

bool ReportExporter::exportToCsv(
    const std::string& filePath,
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& processes,
    const std::string& profileName) {

    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    // Time string
    auto now = std::chrono::system_clock::to_time_t(sysMetrics.timestamp);
    std::tm tmNow{};
    localtime_s(&tmNow, &now);

    out << "# PulseOS System Performance Audit Log\n";
    out << "# Export Timestamp," << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S") << "\n";
    out << "# Active Workload Profile," << profileName << "\n";
    out << "# Processor," << sysMetrics.cpuName << "\n";
    out << "# Physical Cores," << sysMetrics.cpuPhysicalCores << "\n";
    out << "# Logical Processors," << sysMetrics.cpuLogicalProcessors << "\n";
    out << "# Total Physical RAM (MB)," << (sysMetrics.totalRamBytes / (1024 * 1024)) << "\n";
    out << "# Live CPU Usage (%)," << sysMetrics.cpuUsagePercent << "\n";
    out << "# Live RAM Usage (%)," << sysMetrics.ramUsagePercent << "\n";
    out << "# Active Processes Count," << sysMetrics.activeProcessCount << "\n";
    out << "# Active Threads Count," << sysMetrics.activeThreadCount << "\n";
    out << "# System Uptime (seconds)," << sysMetrics.uptimeSeconds << "\n\n";

    // Process Table Header
    out << "PID,ProcessName,CPU_Percent,WorkingSet_Bytes,WorkingSet_MB,PrivateCommit_MB,ThreadCount,PriorityClass,AccessStatus,ExecutablePath\n";

    for (const auto& p : processes) {
        out << p.pid << ","
            << "\"" << p.name << "\","
            << std::fixed << std::setprecision(1) << p.cpuUsagePercent << ","
            << p.workingSetBytes << ","
            << (p.workingSetBytes / (1024 * 1024)) << ","
            << (p.privateBytes / (1024 * 1024)) << ","
            << p.threadCount << ","
            << "\"" << p.priorityClassString << "\","
            << static_cast<int>(p.accessStatus) << ","
            << "\"" << p.executablePath << "\"\n";
    }

    out.close();
    return true;
}

bool ReportExporter::exportAuditReport(
    const std::string& filePath,
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& processes,
    const std::vector<Recommendation>& recommendations,
    const std::vector<OptimizationAction>& appliedActions,
    const std::string& profileName) {

    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    auto now = std::chrono::system_clock::to_time_t(sysMetrics.timestamp);
    std::tm tmNow{};
    localtime_s(&tmNow, &now);

    out << "# PulseOS System Performance Audit Report\n\n";
    out << "Generated on: **" << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S") << "**  \n";
    out << "Active Profile: **" << profileName << "**\n\n";

    out << "## 1. Hardware Architecture & System Environment\n\n";
    out << "- **Processor:** " << sysMetrics.cpuName << " ("
        << sysMetrics.cpuPhysicalCores << " Physical Cores, "
        << sysMetrics.cpuLogicalProcessors << " Logical Processors)\n";
    out << "- **Physical Memory:** " << (sysMetrics.totalRamBytes / (1024 * 1024 * 1024)) << " GB Total\n";
    out << "- **System Uptime:** " << (sysMetrics.uptimeSeconds / 3600) << " hours\n\n";

    out << "## 2. Live Performance Measurements\n\n";
    out << "- **Total CPU Load:** " << std::fixed << std::setprecision(1) << sysMetrics.cpuUsagePercent << "%\n";
    out << "- **RAM Utilization:** " << sysMetrics.ramUsagePercent << "% ("
        << (sysMetrics.usedRamBytes / (1024 * 1024)) << " MB used, "
        << (sysMetrics.availableRamBytes / (1024 * 1024)) << " MB free)\n";
    out << "- **Process / Thread Count:** " << sysMetrics.activeProcessCount << " processes, "
        << sysMetrics.activeThreadCount << " threads\n\n";

    out << "## 3. Active Recommendations & Diagnostics\n\n";
    if (recommendations.empty()) {
        out << "_No active performance warnings or diagnostics._\n\n";
    } else {
        for (const auto& r : recommendations) {
            out << "### " << r.title << "\n";
            out << "- **Observation:** " << r.observation << "\n";
            out << "- **Diagnosis:** " << r.diagnosis << "\n";
            out << "- **Recommended Action:** " << r.recommendedAction << "\n";
            out << "- **Benefits:** " << r.benefits << "\n";
            out << "- **Risks / Trade-offs:** " << r.risksAndLimitations << "\n\n";
        }
    }

    out << "## 4. Applied Optimization Audit Log\n\n";
    if (appliedActions.empty()) {
        out << "_No optimization actions have been applied in this session._\n\n";
    } else {
        for (const auto& a : appliedActions) {
            out << "- **" << a.title << "** (Applied)\n";
            out << "  - Expected Impact: " << a.expectedImpact << "\n";
            out << "  - Rollback capability: " << (a.canRollback ? "Supported" : "Dynamic Windows management") << "\n";
        }
        out << "\n";
    }

    out.close();
    return true;
}

} // namespace pulse

