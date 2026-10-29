#include "SystemOptimizer.h"

#include <psapi.h>
#include <powrprof.h>
#include <tlhelp32.h>
#include <cguid.h>
#include <cstring>
#include <algorithm>

namespace pulse {

GUID SystemOptimizer::stringToGuid(const std::string& guidStr) {
    GUID guid{};
    std::memset(&guid, 0, sizeof(GUID));
    if (guidStr.empty()) return guid;

    unsigned long d1;
    unsigned int d2, d3;
    unsigned int d4[8];

    int count = sscanf(guidStr.c_str(),
                       "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                       &d1, &d2, &d3,
                       &d4[0], &d4[1], &d4[2], &d4[3],
                       &d4[4], &d4[5], &d4[6], &d4[7]);

    if (count == 11) {
        guid.Data1 = d1;
        guid.Data2 = static_cast<unsigned short>(d2);
        guid.Data3 = static_cast<unsigned short>(d3);
        for (int i = 0; i < 8; ++i) {
            guid.Data4[i] = static_cast<unsigned char>(d4[i]);
        }
    }
    return guid;
}

std::string SystemOptimizer::guidToString(const GUID& guid) {
    char guidStr[64] = {0};
    snprintf(guidStr, sizeof(guidStr),
             "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             guid.Data1, guid.Data2, guid.Data3,
             guid.Data4[0], guid.Data4[1], guid.Data4[2],
             guid.Data4[3], guid.Data4[4], guid.Data4[5],
             guid.Data4[6], guid.Data4[7]);
    return std::string(guidStr);
}

SystemOptimizer::SystemOptimizer() = default;

SystemOptimizer::ActionResult SystemOptimizer::setPowerScheme(const std::string& schemeGuid) {
    ActionResult res;
    res.actionId = "set_power_scheme";

    // 1. Query current scheme for rollback
    GUID* pCurrent = nullptr;
    if (PowerGetActiveScheme(nullptr, &pCurrent) == ERROR_SUCCESS && pCurrent) {
        res.previousValue = guidToString(*pCurrent);
        LocalFree(pCurrent);
    }

    // 2. Parse target GUID
    GUID target = stringToGuid(schemeGuid);
    GUID emptyGuid{};
    if (std::memcmp(&target, &emptyGuid, sizeof(GUID)) == 0) {
        res.success = false;
        res.message = "Invalid or unparseable power scheme GUID: " + schemeGuid;
        return res;
    }

    // 3. Apply via PowerSetActiveScheme
    DWORD err = PowerSetActiveScheme(nullptr, &target);
    if (err == ERROR_SUCCESS) {
        res.success = true;
        res.newValue = schemeGuid;
        res.message = "Successfully switched Windows power scheme.";
        res.canRollback = !res.previousValue.empty();
    } else {
        res.success = false;
        res.message = "Failed to set power scheme. Windows error code: " + std::to_string(err);
    }
    return res;
}

SystemOptimizer::ActionResult SystemOptimizer::trimProcessWorkingSets() {
    ActionResult res;
    res.actionId = "trim_working_sets";
    res.canRollback = false; // Working sets automatically refill on active demand

    uint32_t trimmedCount = 0;
    uint64_t totalReclaimedEst = 0;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);

        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (pe.th32ProcessID <= 4) continue; // Skip System / Idle

                HANDLE hProc = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProc) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                        size_t wsBefore = pmc.WorkingSetSize;
                        // Documented Win32 API to empty working set
                        if (EmptyWorkingSet(hProc)) {
                            trimmedCount++;
                            if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                                if (wsBefore > pmc.WorkingSetSize) {
                                    totalReclaimedEst += (wsBefore - pmc.WorkingSetSize);
                                }
                            }
                        }
                    }
                    CloseHandle(hProc);
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }

    res.success = (trimmedCount > 0);
    double reclaimedMb = static_cast<double>(totalReclaimedEst) / (1024.0 * 1024.0);
    res.message = "Trimmed working sets across " + std::to_string(trimmedCount) +
                  " accessible processes. Estimated ~" + std::to_string(static_cast<int>(reclaimedMb)) +
                  " MB returned to the standby list.";
    return res;
}

SystemOptimizer::ActionResult SystemOptimizer::setProcessPriority(uint32_t pid, DWORD priorityClass) {
    ActionResult res;
    res.actionId = "set_process_priority_" + std::to_string(pid);

    if (pid <= 4) {
        res.success = false;
        res.message = "Refusing to adjust priority of critical system process.";
        return res;
    }

    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProc) {
        res.success = false;
        res.message = "Cannot open process PID " + std::to_string(pid) + " (Access Denied or Process Exited).";
        return res;
    }

    DWORD prevPriority = GetPriorityClass(hProc);
    res.previousValue = std::to_string(prevPriority);

    if (SetPriorityClass(hProc, priorityClass)) {
        res.success = true;
        res.newValue = std::to_string(priorityClass);
        res.canRollback = true;
        res.message = "Successfully adjusted process priority class.";
    } else {
        res.success = false;
        res.message = "SetPriorityClass failed with error code: " + std::to_string(GetLastError());
    }

    CloseHandle(hProc);
    return res;
}

SystemOptimizer::ActionResult SystemOptimizer::preventSleep(bool enable) {
    ActionResult res;
    res.actionId = "prevent_sleep";
    res.previousValue = enable ? "normal" : "keep_awake";
    res.newValue = enable ? "keep_awake" : "normal";

    EXECUTION_STATE flags = ES_CONTINUOUS;
    if (enable) {
        flags |= ES_SYSTEM_REQUIRED;
    }

    EXECUTION_STATE prev = SetThreadExecutionState(flags);
    res.success = (prev != 0);
    res.canRollback = true;
    res.message = enable ? "System sleep prevention activated for active workload."
                         : "System sleep prevention released; normal power timeouts restored.";
    return res;
}

SystemOptimizer::ActionResult SystemOptimizer::applyAction(const OptimizationAction& action) {
    std::lock_guard<std::mutex> lock(m_mutex);

    ActionResult result;

    if (action.id == "set_high_performance_power") {
        result = setPowerScheme("8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
    } else if (action.id == "set_balanced_power" || action.id == "align_power_scheme") {
        result = setPowerScheme("381b4222-f694-41f0-9685-ff5bb260df2e");
    } else if (action.id == "set_power_saver_scheme") {
        result = setPowerScheme("a1841308-3541-4fab-bc81-f71556f20b4a");
    } else if (action.id == "trim_working_sets") {
        result = trimProcessWorkingSets();
    } else if (action.id == "prevent_sleep_during_render") {
        result = preventSleep(true);
    } else if (action.id.rfind("deprioritize_process_", 0) == 0) {
        std::string pidStr = action.id.substr(21);
        uint32_t pid = std::stoul(pidStr);
        result = setProcessPriority(pid, BELOW_NORMAL_PRIORITY_CLASS);
    } else {
        result.success = false;
        result.message = "Unrecognized optimization action payload: " + action.id;
        return result;
    }

    if (result.success && result.canRollback) {
        OptimizationAction recorded = action;
        recorded.isApplied = true;
        recorded.previousStateValue = result.previousValue;
        recorded.targetStateValue = result.newValue;
        recorded.appliedTimestamp = std::chrono::system_clock::now();
        m_appliedStack.push_back(recorded);
    }

    return result;
}

SystemOptimizer::ActionResult SystemOptimizer::rollbackAction(const std::string& actionId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = std::find_if(m_appliedStack.rbegin(), m_appliedStack.rend(),
                           [&actionId](const OptimizationAction& a) { return a.id == actionId; });

    if (it == m_appliedStack.rend()) {
        ActionResult res;
        res.success = false;
        res.message = "No rollback record found for action: " + actionId;
        return res;
    }

    OptimizationAction recorded = *it;
    ActionResult result;

    if (recorded.id.find("power") != std::string::npos && !recorded.previousStateValue.empty()) {
        result = setPowerScheme(recorded.previousStateValue);
    } else if (recorded.id == "prevent_sleep_during_render") {
        result = preventSleep(false);
    } else if (recorded.id.rfind("deprioritize_process_", 0) == 0) {
        std::string pidStr = recorded.id.substr(21);
        uint32_t pid = std::stoul(pidStr);
        DWORD prevPrio = std::stoul(recorded.previousStateValue);
        result = setProcessPriority(pid, prevPrio);
    } else {
        result.success = false;
        result.message = "Action cannot be rolled back: " + recorded.id;
        return result;
    }

    if (result.success) {
        auto baseIt = (it + 1).base();
        m_appliedStack.erase(baseIt);
        result.message = "Successfully rolled back: " + recorded.title;
    }

    return result;
}

SystemOptimizer::ActionResult SystemOptimizer::rollbackLastAction() {
    std::string lastId;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_appliedStack.empty()) {
            ActionResult res;
            res.success = false;
            res.message = "No actions available to rollback.";
            return res;
        }
        lastId = m_appliedStack.back().id;
    }
    return rollbackAction(lastId);
}

std::vector<OptimizationAction> SystemOptimizer::getAppliedActions() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return {m_appliedStack.begin(), m_appliedStack.end()};
}

bool SystemOptimizer::hasRollbackAvailable() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_appliedStack.empty();
}

} // namespace pulse
