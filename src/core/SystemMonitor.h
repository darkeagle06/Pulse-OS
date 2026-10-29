#pragma once

#include "ISystemMonitor.h"
#include <deque>
#include <mutex>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace pulse {

class SystemMonitor : public ISystemMonitor {
public:
    explicit SystemMonitor(size_t historyCapacity = 60);
    ~SystemMonitor() override = default;

    SystemMetrics sample() override;
    [[nodiscard]] SystemMetrics getLastMetrics() const override;

    [[nodiscard]] std::string getCpuName() const override;
    [[nodiscard]] uint32_t getPhysicalCoreCount() const override;
    [[nodiscard]] uint32_t getLogicalProcessorCount() const override;
    [[nodiscard]] uint64_t getTotalPhysicalMemory() const override;

    [[nodiscard]] std::vector<double> getCpuHistory() const override;
    [[nodiscard]] std::vector<double> getRamHistory() const override;
    [[nodiscard]] size_t getMaxHistorySize() const override;

    void setHistoryCapacity(size_t capacity);

private:
    void initHardwareInfo();
    static uint64_t fileTimeToUint64(const FILETIME& ft);

    mutable std::mutex m_mutex;
    size_t m_historyCapacity{60};

    // Cached hardware details
    std::string m_cpuName{"Unknown Processor"};
    uint32_t m_physicalCores{0};
    uint32_t m_logicalProcessors{0};
    uint64_t m_totalPhysicalMemory{0};

    // State for CPU delta calculation
    bool m_hasPreviousCpuTime{false};
    uint64_t m_prevIdleTime{0};
    uint64_t m_prevKernelTime{0};
    uint64_t m_prevUserTime{0};

    // Latest sampled metrics
    SystemMetrics m_lastMetrics;

    // Historical buffers
    std::deque<double> m_cpuHistory;
    std::deque<double> m_ramHistory;
};

} // namespace pulse

