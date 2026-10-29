#pragma once

#include "Types.h"
#include <string>
#include <vector>

namespace pulse {

class ReportExporter {
public:
    ReportExporter() = default;
    ~ReportExporter() = default;

    // Export collected metrics and active processes to standard CSV
    static bool exportToCsv(
        const std::string& filePath,
        const SystemMetrics& sysMetrics,
        const std::vector<ProcessInfo>& processes,
        const std::string& profileName);

    // Export comprehensive markdown audit report
    static bool exportAuditReport(
        const std::string& filePath,
        const SystemMetrics& sysMetrics,
        const std::vector<ProcessInfo>& processes,
        const std::vector<Recommendation>& recommendations,
        const std::vector<OptimizationAction>& appliedActions,
        const std::string& profileName);
};

} // namespace pulse

