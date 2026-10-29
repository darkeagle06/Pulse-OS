#include "SystemMonitor.h"

#include <psapi.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace pulse {

uint64_t SystemMonitor::fileTimeToUint64(const FILETIME& ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart;
}

SystemMonitor::SystemMonitor(size_t historyCapacity)
    : m_historyCapacity(historyCapacity) {
    initHardwareInfo();
    // Prime the initial CPU baseline sample
    sample();
}

void SystemMonitor::initHardwareInfo() {
    // 1. Processor logical cores
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_logicalProcessors = sysInfo.dwNumberOfProcessors;

    // 2. Physical cores count via GetLogicalProcessorInformationEx
    DWORD len = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &len);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && len > 0) {
        std::vector<uint8_t> buffer(len);
        auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());
        if (GetLogicalProcessorInformationEx(RelationProcessorCore, info, &len)) {
            uint32_t coreCount = 0;
            DWORD offset = 0;
            while (offset < len) {
                auto* current = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data() + offset);
                if (current->Relationship == RelationProcessorCore) {
                    coreCount++;
                }
                offset += current->Size;
            }
            m_physicalCores = (coreCount > 0) ? coreCount : m_logicalProcessors;
        }
    }
    if (m_physicalCores == 0) {
        m_physicalCores = m_logicalProcessors;
    }

    // 3. Processor Brand String from Registry
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char brand[256] = {0};
        DWORD bufSize = sizeof(brand);
        if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr,
                             reinterpret_cast<LPBYTE>(brand), &bufSize) == ERROR_SUCCESS) {
            m_cpuName = brand;
            // Trim leading/trailing whitespace
            while (!m_cpuName.empty() && m_cpuName.front() == ' ') m_cpuName.erase(m_cpuName.begin());
            while (!m_cpuName.empty() && m_cpuName.back() == ' ') m_cpuName.pop_back();
        }
        RegCloseKey(hKey);
    }

    // 4. Total physical RAM
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        m_totalPhysicalMemory = memStatus.ullTotalPhys;
    }
}

SystemMetrics SystemMonitor::sample() {
    std::lock_guard<std::mutex> lock(m_mutex);

    SystemMetrics metrics;
    metrics.timestamp = std::chrono::system_clock::now();
    metrics.cpuName = m_cpuName;
    metrics.cpuPhysicalCores = m_physicalCores;
    metrics.cpuLogicalProcessors = m_logicalProcessors;
    metrics.totalRamBytes = m_totalPhysicalMemory;
    metrics.uptimeSeconds = GetTickCount64() / 1000;

    // 1. Memory measurements via GlobalMemoryStatusEx
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        metrics.totalRamBytes = memStatus.ullTotalPhys;
        metrics.availableRamBytes = memStatus.ullAvailPhys;
        metrics.usedRamBytes = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
        metrics.ramUsagePercent = static_cast<double>(memStatus.dwMemoryLoad);
        metrics.isValid = true;
    } else {
        metrics.isValid = false;
        metrics.errorMessage = "Failed to query system memory status.";
    }

    // 2. Performance info (active processes and threads)
    PERFORMANCE_INFORMATION perfInfo;
    perfInfo.cb = sizeof(perfInfo);
    if (GetPerformanceInfo(&perfInfo, sizeof(perfInfo))) {
        metrics.activeProcessCount = static_cast<uint32_t>(perfInfo.ProcessCount);
        metrics.activeThreadCount = static_cast<uint32_t>(perfInfo.ThreadCount);
    }

    // 3. Genuine CPU utilization via GetSystemTimes
    FILETIME idleTimeFt, kernelTimeFt, userTimeFt;
    if (GetSystemTimes(&idleTimeFt, &kernelTimeFt, &userTimeFt)) {
        uint64_t idleTime = fileTimeToUint64(idleTimeFt);
        uint64_t kernelTime = fileTimeToUint64(kernelTimeFt);
        uint64_t userTime = fileTimeToUint64(userTimeFt);

        if (m_hasPreviousCpuTime) {
            uint64_t idleDelta = idleTime - m_prevIdleTime;
            uint64_t kernelDelta = kernelTime - m_prevKernelTime;
            uint64_t userDelta = userTime - m_prevUserTime;

            // In Windows, kernelTime includes idleTime
            uint64_t totalSys = kernelDelta + userDelta;

            if (totalSys > 0) {
                // If totalSys >= idleDelta, compute utilization
                if (totalSys >= idleDelta) {
                    double busy = static_cast<double>(totalSys - idleDelta);
                    metrics.cpuUsagePercent = (busy / static_cast<double>(totalSys)) * 100.0;
                } else {
                    metrics.cpuUsagePercent = 0.0;
                }
            } else {
                metrics.cpuUsagePercent = 0.0;
            }

            metrics.cpuUsagePercent = std::clamp(metrics.cpuUsagePercent, 0.0, 100.0);
        } else {
            // First sample baseline established
            metrics.cpuUsagePercent = 0.0;
            m_hasPreviousCpuTime = true;
        }

        m_prevIdleTime = idleTime;
        m_prevKernelTime = kernelTime;
        m_prevUserTime = userTime;
    } else {
        metrics.isValid = false;
        metrics.errorMessage += " Failed to query GetSystemTimes.";
    }

    // Record into historical buffers
    m_cpuHistory.push_back(metrics.cpuUsagePercent);
    if (m_cpuHistory.size() > m_historyCapacity) {
        m_cpuHistory.pop_front();
    }

    m_ramHistory.push_back(metrics.ramUsagePercent);
    if (m_ramHistory.size() > m_historyCapacity) {
        m_ramHistory.pop_front();
    }

    m_lastMetrics = metrics;
    return metrics;
}

SystemMetrics SystemMonitor::getLastMetrics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lastMetrics;
}

std::string SystemMonitor::getCpuName() const {
    return m_cpuName;
}

uint32_t SystemMonitor::getPhysicalCoreCount() const {
    return m_physicalCores;
}

uint32_t SystemMonitor::getLogicalProcessorCount() const {
    return m_logicalProcessors;
}

uint64_t SystemMonitor::getTotalPhysicalMemory() const {
    return m_totalPhysicalMemory;
}

std::vector<double> SystemMonitor::getCpuHistory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return {m_cpuHistory.begin(), m_cpuHistory.end()};
}

std::vector<double> SystemMonitor::getRamHistory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return {m_ramHistory.begin(), m_ramHistory.end()};
}

size_t SystemMonitor::getMaxHistorySize() const {
    return m_historyCapacity;
}

void SystemMonitor::setHistoryCapacity(size_t capacity) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_historyCapacity = capacity;
    while (m_cpuHistory.size() > m_historyCapacity) m_cpuHistory.pop_front();
    while (m_ramHistory.size() > m_historyCapacity) m_ramHistory.pop_front();
}

} // namespace pulse

