#pragma once

#include "IProcessMonitor.h"
#include <unordered_map>
#include <mutex>
#include <chrono>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace pulse {

class ProcessMonitor : public IProcessMonitor {
public:
    ProcessMonitor();
    ~ProcessMonitor() override = default;

    std::vector<ProcessInfo> sampleProcesses() override;
    [[nodiscard]] std::vector<ProcessInfo> getLastProcessList() const override;
    [[nodiscard]] std::optional<ProcessInfo> getProcessByPid(uint32_t pid) const override;
    [[nodiscard]] ForegroundInfo getForegroundProcessInfo() const override;

    // Filter and sort helpers
    [[nodiscard]] static std::vector<ProcessInfo> filterByName(
        const std::vector<ProcessInfo>& list, const std::string& query);

    enum class SortField { CpuDesc, MemoryDesc, PidAsc, NameAsc };
    static void sortProcesses(std::vector<ProcessInfo>& list, SortField field);

private:
    struct CpuSampleState {
        uint64_t kernelTime{0};
        uint64_t userTime{0};
        std::chrono::steady_clock::time_point timestamp;
    };

    static uint64_t fileTimeToUint64(const FILETIME& ft);
    static std::string priorityClassToString(DWORD priorityClass);

    mutable std::mutex m_mutex;
    std::unordered_map<uint32_t, CpuSampleState> m_previousSamples;
    std::vector<ProcessInfo> m_cachedProcesses;

    // System-wide CPU time for normalization
    bool m_hasPrevSysTimes{false};
    uint64_t m_prevSysKernel{0};
    uint64_t m_prevSysUser{0};
    uint32_t m_processorCount{1};
};

} // namespace pulse

