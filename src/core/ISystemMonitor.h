#pragma once

#include "Types.h"
#include <memory>
#include <vector>

namespace pulse {

class ISystemMonitor {
public:
    virtual ~ISystemMonitor() = default;

    // Collect fresh system metrics from the operating system
    virtual SystemMetrics sample() = 0;

    // Retrieve the most recently cached metrics
    [[nodiscard]] virtual SystemMetrics getLastMetrics() const = 0;

    // Static processor and hardware information
    [[nodiscard]] virtual std::string getCpuName() const = 0;
    [[nodiscard]] virtual uint32_t getPhysicalCoreCount() const = 0;
    [[nodiscard]] virtual uint32_t getLogicalProcessorCount() const = 0;
    [[nodiscard]] virtual uint64_t getTotalPhysicalMemory() const = 0;

    // Query historical CPU and RAM utilization for charting
    [[nodiscard]] virtual std::vector<double> getCpuHistory() const = 0;
    [[nodiscard]] virtual std::vector<double> getRamHistory() const = 0;
    [[nodiscard]] virtual size_t getMaxHistorySize() const = 0;
};

} // namespace pulse

