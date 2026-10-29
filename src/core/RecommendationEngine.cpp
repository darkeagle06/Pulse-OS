#include "RecommendationEngine.h"
#include "DiagnosticLog.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <powrprof.h>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace pulse {

RecommendationEngine::RecommendationEngine() = default;

void RecommendationEngine::updateCpuHistory(double currentCpu) {
    m_recentCpuSamples.push_back(currentCpu);
    if (m_recentCpuSamples.size() > SustainedWindowSize) {
        m_recentCpuSamples.pop_front();
    }
}

std::string RecommendationEngine::getActivePowerSchemeGuid() {
    GUID* pActiveGuid = nullptr;
    if (PowerGetActiveScheme(nullptr, &pActiveGuid) == ERROR_SUCCESS && pActiveGuid) {
        char guidStr[64] = {0};
        snprintf(guidStr, sizeof(guidStr),
                 "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                 pActiveGuid->Data1, pActiveGuid->Data2, pActiveGuid->Data3,
                 pActiveGuid->Data4[0], pActiveGuid->Data4[1], pActiveGuid->Data4[2],
                 pActiveGuid->Data4[3], pActiveGuid->Data4[4], pActiveGuid->Data4[5],
                 pActiveGuid->Data4[6], pActiveGuid->Data4[7]);
        LocalFree(pActiveGuid);
        return std::string(guidStr);
    }
    return "";
}

std::vector<RecommendationEngine::StartupItem> RecommendationEngine::queryStartupApplications() {
    std::vector<StartupItem> items;

    auto readKey = [&items](HKEY root, const char* subKey, const std::string& sourceName) {
        HKEY hKey;
        if (RegOpenKeyExA(root, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD index = 0;
            char valName[256];
            BYTE valData[1024];
            DWORD valNameLen = sizeof(valName);
            DWORD valDataLen = sizeof(valData);
            DWORD type = 0;

            while (RegEnumValueA(hKey, index, valName, &valNameLen, nullptr, &type, valData, &valDataLen) == ERROR_SUCCESS) {
                if (type == REG_SZ || type == REG_EXPAND_SZ) {
                    StartupItem item;
                    item.name = valName;
                    item.command = reinterpret_cast<char*>(valData);
                    item.source = sourceName;
                    items.push_back(item);
                }
                index++;
                valNameLen = sizeof(valName);
                valDataLen = sizeof(valData);
            }
            RegCloseKey(hKey);
        }
    };

    readKey(HKEY_CURRENT_USER, R"(Software\Microsoft\Windows\CurrentVersion\Run)", "Current User");
    readKey(HKEY_LOCAL_MACHINE, R"(Software\Microsoft\Windows\CurrentVersion\Run)", "All Users");

    return items;
}

std::vector<Recommendation> RecommendationEngine::generateRecommendations(
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& processes,
    const std::shared_ptr<OptimizationStrategy>& activeStrategy) {

    logDiagnostic("[RecommendationEngine] Evaluating rules for strategy: " +
                  (activeStrategy ? activeStrategy->getProfileName() : "None"));
    std::vector<Recommendation> recs;
    updateCpuHistory(sysMetrics.cpuUsagePercent);

    // 1. Strategy-specific recommendations
    if (activeStrategy) {
        auto stratRecs = activeStrategy->evaluate(sysMetrics, processes);
        recs.insert(recs.end(), stratRecs.begin(), stratRecs.end());
    }

    // 2. Rule: Sustained High CPU (>75% average over the sample window)
    if (m_recentCpuSamples.size() >= SustainedWindowSize) {
        double sum = std::accumulate(m_recentCpuSamples.begin(), m_recentCpuSamples.end(), 0.0);
        double avg = sum / static_cast<double>(m_recentCpuSamples.size());

        if (avg >= 75.0) {
            Recommendation r;
            r.id = "rule_sustained_high_cpu";
            r.title = "Sustained High CPU Utilization Detected";
            r.observation = "Average CPU utilization over the last " +
                            std::to_string(m_recentCpuSamples.size()) + " seconds is " +
                            std::to_string(static_cast<int>(avg)) + "%.";
            r.diagnosis = "Prolonged high CPU load. This may be expected during heavy compute (compilation, rendering, gaming) or indicative of runaway background processes.";
            r.recommendedAction = "Review active processes sorted by CPU in Process Explorer to identify the primary workload driver.";
            r.explanation = "Continuous high CPU keeps core temperatures elevated, eventually triggering thermal throttling where clocks drop to prevent overheating.";
            r.benefits = "Identifying whether load is intentional prevents thermal throttling and unnecessary fan noise.";
            r.risksAndLimitations = "High CPU alone is an observation, not a defect. Do not terminate productive tasks.";
            r.level = (avg >= 90.0) ? RecommendationLevel::Critical : RecommendationLevel::Warning;
            r.isActionable = false;
            recs.push_back(r);
        }
    }

    // 3. Rule: Memory Pressure (>85% RAM load)
    if (sysMetrics.ramUsagePercent >= 85.0) {
        Recommendation r;
        r.id = "rule_high_ram_pressure";
        r.title = "High Physical Memory Pressure";
        r.observation = "Total system RAM usage is " +
                        std::to_string(static_cast<int>(sysMetrics.ramUsagePercent)) +
                        "% (" + std::to_string(sysMetrics.availableRamBytes / (1024 * 1024)) + " MB available).";
        r.diagnosis = "Operating system is low on physical memory headroom and may begin paging non-active working sets to the Windows pagefile (disk thrashing).";
        r.recommendedAction = "Apply working set memory cleanup or close memory-intensive background apps.";
        r.explanation = "Pagefile thrashing drastically reduces responsiveness as SSD/HDD paging latencies are orders of magnitude slower than DRAM access.";
        r.benefits = "Restores physical memory headroom, prevents UI micro-freezes.";
        r.risksAndLimitations = "Trimmed processes will page back in if accessed immediately.";
        r.level = RecommendationLevel::Warning;
        r.isActionable = true;
        r.actionPayload = "trim_working_sets";
        recs.push_back(r);
    }

    // 4. Rule: Individual Runaway / Heavy Process
    for (const auto& proc : processes) {
        if (proc.cpuUsagePercent >= 35.0 &&
            proc.name != "System Idle Process" &&
            proc.name != "PulseOS.exe") {

            Recommendation r;
            r.id = "rule_heavy_proc_" + std::to_string(proc.pid);
            r.title = "Substantial CPU Consumption by " + proc.name;
            r.observation = "Process '" + proc.name + "' (PID " + std::to_string(proc.pid) +
                            ") is consuming " + std::to_string(static_cast<int>(proc.cpuUsagePercent)) +
                            "% of total system CPU capacity.";
            r.diagnosis = "Single application is driving significant processor load. Verify if this matches current user activity.";
            r.recommendedAction = "If unexpected, check if the application is frozen, indexing, or in an infinite loop.";
            r.explanation = "A single process consuming multiple cores creates thermal and power draw even when the machine is otherwise idle.";
            r.benefits = "Allows user to intervene before battery or thermal limits are reached.";
            r.risksAndLimitations = "Legitimate applications (e.g. video exporters, games, compilers) operate at high CPU by design.";
            r.level = RecommendationLevel::Suggestion;
            r.isActionable = false;
            recs.push_back(r);
            break; // Report the single most prominent runaway process to prevent card spam
        }
    }

    // 5. Rule: Startup Applications Review
    auto startupItems = queryStartupApplications();
    if (startupItems.size() >= 5) {
        Recommendation r;
        r.id = "rule_startup_review";
        r.title = "Review Startup Applications (" + std::to_string(startupItems.size()) + " Detected)";
        r.observation = std::to_string(startupItems.size()) + " autostart entries are registered in the Windows Run registry keys.";
        r.diagnosis = "Numerous startup applications consume background RAM and CPU cycles from the moment Windows boots.";
        r.recommendedAction = "Review startup items in Windows Task Manager > Startup or Task Settings.";
        r.explanation = "Each background tray utility or update agent adds 30MB-150MB of resident memory and occasional background thread wakeups.";
        r.benefits = "Faster Windows boot times, reduced background thread contention.";
        r.risksAndLimitations = "Disabling utilities like cloud sync or audio control panels may require manual launch when needed.";
        r.level = RecommendationLevel::Info;
        r.isActionable = false;
        recs.push_back(r);
    }

    // 6. Rule: Power Scheme Alignment
    if (activeStrategy) {
        std::string currentGuid = getActivePowerSchemeGuid();
        std::string recommendedGuid = activeStrategy->getRecommendedPowerScheme();

        if (!currentGuid.empty() && !recommendedGuid.empty() &&
            currentGuid != recommendedGuid) {

            std::string recName = (recommendedGuid == "8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c") ? "High Performance" :
                                  (recommendedGuid == "a1841308-3541-4fab-bc81-f71556f20b4a") ? "Power Saver" : "Balanced";

            Recommendation r;
            r.id = "rule_power_alignment";
            r.title = "Power Scheme Misalignment for " + activeStrategy->getProfileName();
            r.observation = "Windows active power scheme (" + currentGuid.substr(0, 8) + "...) does not match recommended " + recName + " profile.";
            r.diagnosis = "The active Windows power plan configuration may constrain frequencies or consume more power than optimal for the current profile.";
            r.recommendedAction = "Switch Windows power scheme to " + recName + ".";
            r.explanation = "Power plans govern CPU minimum and maximum clock percentages, core parking, and passive vs active cooling policies.";
            r.benefits = "Ensures hardware power delivery is synchronized with workload goals.";
            r.risksAndLimitations = "High performance raises idle wattage; power saver caps maximum burst frequencies.";
            r.level = RecommendationLevel::Suggestion;
            r.isActionable = true;
            r.actionPayload = "align_power_scheme";
            recs.push_back(r);
        }
    }

    return recs;
}

} // namespace pulse

