#include "ProcessMonitor.h"

#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>
#include <unordered_set>
#include <string>

namespace pulse {

uint64_t ProcessMonitor::fileTimeToUint64(const FILETIME& ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart;
}

std::string ProcessMonitor::priorityClassToString(DWORD priorityClass) {
    switch (priorityClass) {
        case IDLE_PRIORITY_CLASS: return "Idle";
        case BELOW_NORMAL_PRIORITY_CLASS: return "Below Normal";
        case NORMAL_PRIORITY_CLASS: return "Normal";
        case ABOVE_NORMAL_PRIORITY_CLASS: return "Above Normal";
        case HIGH_PRIORITY_CLASS: return "High";
        case REALTIME_PRIORITY_CLASS: return "Realtime";
        default: return "Normal";
    }
}

ProcessMonitor::ProcessMonitor() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_processorCount = (sysInfo.dwNumberOfProcessors > 0) ? sysInfo.dwNumberOfProcessors : 1;
    // Prime initial system times
    sampleProcesses();
}

std::vector<ProcessInfo> ProcessMonitor::sampleProcesses() {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 1. Get current system times for accurate relative CPU calculation
    FILETIME idleFt, sysKernelFt, sysUserFt;
    uint64_t sysDelta = 0;
    if (GetSystemTimes(&idleFt, &sysKernelFt, &sysUserFt)) {
        uint64_t sysKernel = fileTimeToUint64(sysKernelFt);
        uint64_t sysUser = fileTimeToUint64(sysUserFt);

        if (m_hasPrevSysTimes) {
            uint64_t kDelta = sysKernel - m_prevSysKernel;
            uint64_t uDelta = sysUser - m_prevSysUser;
            sysDelta = kDelta + uDelta;
        } else {
            m_hasPrevSysTimes = true;
        }
        m_prevSysKernel = sysKernel;
        m_prevSysUser = sysUser;
    }

    auto now = std::chrono::steady_clock::now();
    std::vector<ProcessInfo> result;
    std::unordered_set<uint32_t> currentPids;

    // 2. Snapshot active processes via Toolhelp32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return m_cachedProcesses;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            uint32_t pid = pe.th32ProcessID;
            currentPids.insert(pid);

            ProcessInfo info;
            info.pid = pid;
            info.threadCount = pe.cntThreads;

            // Convert wide string name to UTF-8
            int nameLen = WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, nullptr, 0, nullptr, nullptr);
            if (nameLen > 0) {
                std::string nameStr(nameLen - 1, '\0');
                WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, nameStr.data(), nameLen, nullptr, nullptr);
                info.name = nameStr;
            } else {
                info.name = "Unknown";
            }

            // PID 0 is System Idle Process
            if (pid == 0) {
                info.name = "System Idle Process";
                info.accessStatus = ProcessAccessStatus::SystemProtected;
                result.push_back(info);
                continue;
            }

            // Attempt to open process for metrics inspection
            // Try limited query first for best compatibility across privileges
            HANDLE hProcess = OpenProcess(
                PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                FALSE, pid);

            if (!hProcess) {
                // Try fallback with only query limited information
                hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
            }

            if (!hProcess) {
                DWORD err = GetLastError();
                if (err == ERROR_ACCESS_DENIED) {
                    info.accessStatus = ProcessAccessStatus::AccessDenied;
                } else {
                    info.accessStatus = ProcessAccessStatus::SystemProtected;
                }
                result.push_back(info);
                continue;
            }

            info.accessStatus = ProcessAccessStatus::Accessible;

            // Query priority class
            DWORD prio = GetPriorityClass(hProcess);
            info.priorityClass = prio;
            info.priorityClassString = priorityClassToString(prio);

            // Query executable full path
            WCHAR pathBuf[MAX_PATH];
            DWORD pathSize = MAX_PATH;
            if (QueryFullProcessImageNameW(hProcess, 0, pathBuf, &pathSize)) {
                int pathLen = WideCharToMultiByte(CP_UTF8, 0, pathBuf, -1, nullptr, 0, nullptr, nullptr);
                if (pathLen > 0) {
                    std::string pathStr(pathLen - 1, '\0');
                    WideCharToMultiByte(CP_UTF8, 0, pathBuf, -1, pathStr.data(), pathLen, nullptr, nullptr);
                    info.executablePath = pathStr;
                }
            }

            // Query memory statistics
            PROCESS_MEMORY_COUNTERS_EX pmc;
            if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
                info.workingSetBytes = pmc.WorkingSetSize;
                info.privateBytes = pmc.PrivateUsage;
            }

            // Query process CPU times
            FILETIME createTime, exitTime, kernelTimeFt, userTimeFt;
            if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTimeFt, &userTimeFt)) {
                uint64_t kTime = fileTimeToUint64(kernelTimeFt);
                uint64_t uTime = fileTimeToUint64(userTimeFt);

                auto prevIt = m_previousSamples.find(pid);
                if (prevIt != m_previousSamples.end() && sysDelta > 0) {
                    uint64_t pKDelta = (kTime >= prevIt->second.kernelTime) ? (kTime - prevIt->second.kernelTime) : 0;
                    uint64_t pUDelta = (uTime >= prevIt->second.userTime) ? (uTime - prevIt->second.userTime) : 0;
                    uint64_t procDelta = pKDelta + pUDelta;

                    // Per-process CPU utilization normalized by total system activity:
                    // (procDelta / sysDelta) * 100%
                    double cpuPct = (static_cast<double>(procDelta) / static_cast<double>(sysDelta)) * 100.0;
                    info.cpuUsagePercent = std::clamp(cpuPct, 0.0, 100.0);

                    prevIt->second.kernelTime = kTime;
                    prevIt->second.userTime = uTime;
                    prevIt->second.timestamp = now;
                } else {
                    info.cpuUsagePercent = 0.0;
                    m_previousSamples[pid] = {kTime, uTime, now};
                }
            }

            CloseHandle(hProcess);
            result.push_back(info);

        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);

    // Evict dead PIDs from history map to avoid unbounded growth
    for (auto it = m_previousSamples.begin(); it != m_previousSamples.end();) {
        if (currentPids.find(it->first) == currentPids.end()) {
            it = m_previousSamples.erase(it);
        } else {
            ++it;
        }
    }

    m_cachedProcesses = result;
    return result;
}

std::vector<ProcessInfo> ProcessMonitor::getLastProcessList() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cachedProcesses;
}

std::optional<ProcessInfo> ProcessMonitor::getProcessByPid(uint32_t pid) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& proc : m_cachedProcesses) {
        if (proc.pid == pid) return proc;
    }
    return std::nullopt;
}

IProcessMonitor::ForegroundInfo ProcessMonitor::getForegroundProcessInfo() const {
    ForegroundInfo info;
    HWND hWnd = GetForegroundWindow();
    if (!hWnd) return info;

    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    info.pid = pid;

    // Window Title with non-blocking timeout to avoid hangs on unresponsive windows
    WCHAR titleBuf[512] = {0};
    DWORD_PTR dwResult = 0;
    LRESULT lRes = SendMessageTimeoutW(
        hWnd,
        WM_GETTEXT,
        sizeof(titleBuf) / sizeof(WCHAR),
        reinterpret_cast<LPARAM>(titleBuf),
        SMTO_ABORTIFHUNG | SMTO_BLOCK,
        80, // 80 ms maximum wait
        &dwResult);

    if (lRes != 0 && titleBuf[0] != L'\0') {
        int u8Len = WideCharToMultiByte(CP_UTF8, 0, titleBuf, -1, nullptr, 0, nullptr, nullptr);
        if (u8Len > 0) {
            std::string titleStr(u8Len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, titleBuf, -1, titleStr.data(), u8Len, nullptr, nullptr);
            info.windowTitle = titleStr;
        }
    } else {
        // Fallback to fast internal title query without inter-thread message pump
        int intLen = InternalGetWindowText(hWnd, titleBuf, 512);
        if (intLen > 0) {
            int u8Len = WideCharToMultiByte(CP_UTF8, 0, titleBuf, -1, nullptr, 0, nullptr, nullptr);
            if (u8Len > 0) {
                std::string titleStr(u8Len - 1, '\0');
                WideCharToMultiByte(CP_UTF8, 0, titleBuf, -1, titleStr.data(), u8Len, nullptr, nullptr);
                info.windowTitle = titleStr;
            }
        }
    }

    // Process Name from PID
    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProc) {
        WCHAR nameBuf[MAX_PATH] = {0};
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProc, 0, nameBuf, &size)) {
            std::wstring fullPath(nameBuf);
            size_t slash = fullPath.find_last_of(L"\\/");
            std::wstring base = (slash != std::wstring::npos) ? fullPath.substr(slash + 1) : fullPath;

            int u8Len = WideCharToMultiByte(CP_UTF8, 0, base.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (u8Len > 0) {
                std::string procName(u8Len - 1, '\0');
                WideCharToMultiByte(CP_UTF8, 0, base.c_str(), -1, procName.data(), u8Len, nullptr, nullptr);
                info.processName = procName;
            }
        }
        CloseHandle(hProc);
    }

    return info;
}

std::vector<ProcessInfo> ProcessMonitor::filterByName(
    const std::vector<ProcessInfo>& list, const std::string& query) {
    if (query.empty()) return list;

    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    std::vector<ProcessInfo> filtered;
    for (const auto& proc : list) {
        std::string lowerName = proc.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if (lowerName.find(lowerQuery) != std::string::npos ||
            std::to_string(proc.pid).find(lowerQuery) != std::string::npos) {
            filtered.push_back(proc);
        }
    }
    return filtered;
}

void ProcessMonitor::sortProcesses(std::vector<ProcessInfo>& list, SortField field) {
    switch (field) {
        case SortField::CpuDesc:
            std::sort(list.begin(), list.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
                return a.cpuUsagePercent > b.cpuUsagePercent;
            });
            break;
        case SortField::MemoryDesc:
            std::sort(list.begin(), list.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
                return a.workingSetBytes > b.workingSetBytes;
            });
            break;
        case SortField::PidAsc:
            std::sort(list.begin(), list.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
                return a.pid < b.pid;
            });
            break;
        case SortField::NameAsc:
            std::sort(list.begin(), list.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
                return a.name < b.name;
            });
            break;
    }
}

} // namespace pulse

