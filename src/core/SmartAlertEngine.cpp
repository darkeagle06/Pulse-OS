#include "SmartAlertEngine.h"
#include "DiagnosticLog.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

namespace pulse {

namespace {

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return str;
}

std::string formatBytes(uint64_t bytes) {
    double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    if (mb >= 1024.0) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << (mb / 1024.0) << " GB";
        return ss.str();
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << mb << " MB";
    return ss.str();
}

std::string formatPercent(double val) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << val << "%";
    return ss.str();
}

} // anonymous namespace

SmartAlertEngine::SmartAlertEngine(AlertSettings settings)
    : m_settings(std::move(settings)) {
    // Add default ignore items
    if (m_settings.ignoreList.empty()) {
        m_settings.ignoreList = {
            "System", "System Idle Process", "Registry", "smss.exe", "csrss.exe",
            "wininit.exe", "services.exe", "lsass.exe", "svchost.exe", "PulseOS.exe"
        };
    }
}

void SmartAlertEngine::setSettings(const AlertSettings& settings) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_settings = settings;
}

AlertSettings SmartAlertEngine::getSettings() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_settings;
}

void SmartAlertEngine::addToIgnoreList(const std::string& processName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string lower = toLower(processName);
    for (const auto& item : m_settings.ignoreList) {
        if (toLower(item) == lower) return;
    }
    m_settings.ignoreList.push_back(processName);
    logDiagnostic("[SmartAlertEngine] Added to ignore list: " + processName);
}

void SmartAlertEngine::removeFromIgnoreList(const std::string& processName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string lower = toLower(processName);
    auto it = std::remove_if(m_settings.ignoreList.begin(), m_settings.ignoreList.end(),
                             [&lower](const std::string& item) { return toLower(item) == lower; });
    m_settings.ignoreList.erase(it, m_settings.ignoreList.end());
    logDiagnostic("[SmartAlertEngine] Removed from ignore list: " + processName);
}

bool SmartAlertEngine::isIgnored(const std::string& processName) const {
    std::string lower = toLower(processName);
    for (const auto& item : m_settings.ignoreList) {
        if (lower == toLower(item)) return true;
    }
    return false;
}

void SmartAlertEngine::snoozeAlert(const std::string& alertId, uint32_t durationMinutes) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto until = std::chrono::steady_clock::now() + std::chrono::minutes(durationMinutes);
    m_snoozeUntil[alertId] = until;
    logDiagnostic("[SmartAlertEngine] Snoozed alert " + alertId + " for " +
                  std::to_string(durationMinutes) + " minutes.");
}

void SmartAlertEngine::dismissAlert(const std::string& alertId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_dismissed.insert(alertId);
    logDiagnostic("[SmartAlertEngine] Dismissed alert: " + alertId);
}

void SmartAlertEngine::clearCooldowns() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cooldowns.clear();
    m_snoozeUntil.clear();
    m_dismissed.clear();
}

std::vector<ResourceAlert> SmartAlertEngine::getAlertHistory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_alertHistory;
}

void SmartAlertEngine::clearHistory() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_alertHistory.clear();
}

bool SmartAlertEngine::isCoolingDown(const std::string& alertKey, const std::chrono::steady_clock::time_point& now) {
    if (m_dismissed.find(alertKey) != m_dismissed.end()) {
        return true;
    }

    auto snzIt = m_snoozeUntil.find(alertKey);
    if (snzIt != m_snoozeUntil.end()) {
        if (now < snzIt->second) return true;
        m_snoozeUntil.erase(snzIt);
    }

    auto cdIt = m_cooldowns.find(alertKey);
    if (cdIt != m_cooldowns.end()) {
        auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(now - cdIt->second).count();
        if (elapsedSec < m_settings.cooldownDurationSeconds) {
            return true;
        }
    }
    return false;
}

void SmartAlertEngine::recordAlertTriggered(const std::string& alertKey, const std::chrono::steady_clock::time_point& now) {
    m_cooldowns[alertKey] = now;
}

std::pair<std::vector<ResourceAlert>, std::vector<ResourceAlert>> SmartAlertEngine::evaluate(
    const SystemMetrics& sysMetrics,
    const std::vector<ProcessInfo>& processes,
    const TaskDetectionResult& taskDetection) {

    std::lock_guard<std::mutex> lock(m_mutex);
    auto nowSteady = std::chrono::steady_clock::now();
    auto nowSystem = std::chrono::system_clock::now();

    std::vector<ResourceAlert> activeAlerts;
    std::vector<ResourceAlert> newAlerts;

    // Helper lambda to dispatch an alert
    auto triggerAlert = [&](const std::string& alertKey,
                            AlertType type,
                            AlertSeverity severity,
                            const std::string& title,
                            const std::string& observation,
                            const std::string& diagnosis,
                            const std::string& suggestedAction,
                            const std::string& targetProcess,
                            uint32_t pid,
                            double triggerVal,
                            uint32_t sustainedSec) {

        ResourceAlert alert;
        alert.id = alertKey;
        alert.type = type;
        alert.severity = severity;
        alert.title = title;
        alert.observation = observation;
        alert.diagnosis = diagnosis;
        alert.suggestedAction = suggestedAction;
        alert.targetProcessName = targetProcess;
        alert.targetPid = pid;
        alert.triggerValue = triggerVal;
        alert.timestamp = nowSystem;
        alert.sustainedSeconds = sustainedSec;

        activeAlerts.push_back(alert);

        if (!isCoolingDown(alertKey, nowSteady)) {
            recordAlertTriggered(alertKey, nowSteady);
            newAlerts.push_back(alert);

            // Add to history (capped at 100 items)
            m_alertHistory.insert(m_alertHistory.begin(), alert);
            if (m_alertHistory.size() > 100) {
                m_alertHistory.pop_back();
            }

            logDiagnostic("[SmartAlertEngine] ALERT TRIGGERED: " + title + " (" + observation + ")");
        }
    };

    // 1. High System Memory Usage
    if (m_settings.enableHighMemory && sysMetrics.isValid) {
        if (sysMetrics.ramUsagePercent >= m_settings.systemRamThresholdPercent) {
            if (m_highRamCount == 0) m_highRamFirstSeen = nowSteady;
            m_highRamCount++;
            uint32_t sustainedSec = static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::seconds>(nowSteady - m_highRamFirstSeen).count());

            if (sustainedSec >= m_settings.sustainedDurationSeconds) {
                triggerAlert("SYS_HIGH_RAM",
                             AlertType::HighMemoryUsage,
                             AlertSeverity::Warning,
                             "High System RAM Utilization (" + formatPercent(sysMetrics.ramUsagePercent) + ")",
                             "Total physical RAM utilization has been " + formatPercent(sysMetrics.ramUsagePercent) +
                                 " for " + std::to_string(sustainedSec) + "s (configured threshold: " +
                                 formatPercent(m_settings.systemRamThresholdPercent) + ").",
                             "Multiple processes or high working sets are placing heavy demand on physical memory.",
                             "Review top memory consumers in Process Explorer or apply memory trimming in Safe Action Center.",
                             "", 0, sysMetrics.ramUsagePercent, sustainedSec);
            }
        } else {
            m_highRamCount = 0;
        }
    }

    // 2. Critical System Memory Pressure
    if (m_settings.enableMemoryPressure && sysMetrics.isValid) {
        uint64_t criticalThreshold = 1024ULL * 1024 * 1024; // 1 GB
        if (sysMetrics.availableRamBytes < criticalThreshold ||
            (sysMetrics.totalRamBytes > 0 && (static_cast<double>(sysMetrics.availableRamBytes) / sysMetrics.totalRamBytes) < 0.10)) {

            triggerAlert("SYS_MEM_PRESSURE",
                         AlertType::MemoryPressure,
                         AlertSeverity::Critical,
                         "Critical System Memory Pressure",
                         "Available physical memory has dropped to " + formatBytes(sysMetrics.availableRamBytes) +
                             " (less than 1 GB unreserved buffer remaining).",
                         "System is approaching physical memory exhaustion, which causes severe disk pagefile thrashing.",
                         "Close unnecessary applications or tabs immediately to prevent OS responsiveness degradation.",
                         "", 0, sysMetrics.ramUsagePercent, 5);
        }
    }

    // 3. Process-level evaluations
    std::set<std::string> activeTrackerKeys;
    std::string lowerFg = toLower(taskDetection.foregroundProcessName);

    for (const auto& proc : processes) {
        if (isIgnored(proc.name)) continue;

        std::string trackerKey = proc.name + "_" + std::to_string(proc.pid);
        activeTrackerKeys.insert(trackerKey);

        auto& tracker = m_processTrackers[trackerKey];
        tracker.pid = proc.pid;
        tracker.name = proc.name;

        bool isForeground = (!lowerFg.empty() && toLower(proc.name).find(lowerFg) != std::string::npos);

        // A. High CPU Consumption
        if (m_settings.enableHighCpuProcess) {
            if (proc.cpuUsagePercent >= m_settings.processCpuThresholdPercent) {
                if (tracker.highCpuCount == 0) tracker.highCpuFirstSeen = nowSteady;
                tracker.highCpuCount++;
                uint32_t sustainedSec = static_cast<uint32_t>(
                    std::chrono::duration_cast<std::chrono::seconds>(nowSteady - tracker.highCpuFirstSeen).count());

                if (sustainedSec >= m_settings.sustainedDurationSeconds) {
                    triggerAlert("PROC_CPU_" + proc.name,
                                 AlertType::HighCpuProcess,
                                 AlertSeverity::Warning,
                                 "Sustained High CPU: " + proc.name,
                                 proc.name + " (PID " + std::to_string(proc.pid) + ") has consumed " +
                                     formatPercent(proc.cpuUsagePercent) + " CPU for " + std::to_string(sustainedSec) + "s.",
                                 "Application is executing intensive continuous computation or experiencing a high-load loop.",
                                 "Inspect process details in Process Explorer or consider setting lower priority.",
                                 proc.name, proc.pid, proc.cpuUsagePercent, sustainedSec);
                }
            } else {
                tracker.highCpuCount = 0;
            }
        }

        // B. Resource-Heavy Background Application
        if (m_settings.enableHeavyBackground && !isForeground) {
            bool heavyCpu = (proc.cpuUsagePercent >= m_settings.heavyBackgroundCpuPercent);
            bool heavyRam = (proc.workingSetBytes >= m_settings.heavyBackgroundRamBytes);

            if (heavyCpu || heavyRam) {
                if (tracker.heavyBgCount == 0) tracker.heavyBgFirstSeen = nowSteady;
                tracker.heavyBgCount++;
                uint32_t sustainedSec = static_cast<uint32_t>(
                    std::chrono::duration_cast<std::chrono::seconds>(nowSteady - tracker.heavyBgFirstSeen).count());

                if (sustainedSec >= m_settings.sustainedDurationSeconds) {
                    std::string resourceDetails;
                    if (heavyCpu && heavyRam) {
                        resourceDetails = formatPercent(proc.cpuUsagePercent) + " CPU and " + formatBytes(proc.workingSetBytes) + " RAM";
                    } else if (heavyCpu) {
                        resourceDetails = formatPercent(proc.cpuUsagePercent) + " CPU";
                    } else {
                        resourceDetails = formatBytes(proc.workingSetBytes) + " RAM";
                    }

                    triggerAlert("PROC_HEAVY_BG_" + proc.name,
                                 AlertType::HeavyBackground,
                                 AlertSeverity::Info,
                                 "Heavy Background Activity: " + proc.name,
                                 proc.name + " is running in background consuming " + resourceDetails + " for " +
                                     std::to_string(sustainedSec) + "s.",
                                 "Background workload may compete with foreground tasks for CPU scheduling and cache.",
                                 "Check if this background activity is intended or adjust workload profile.",
                                 proc.name, proc.pid, proc.cpuUsagePercent, sustainedSec);
                }
            } else {
                tracker.heavyBgCount = 0;
            }
        }

        // C. Potentially Idle Background Application
        if (m_settings.enableIdleBackground && !isForeground) {
            uint64_t idleRamThreshold = 500ULL * 1024 * 1024; // 500 MB
            if (proc.workingSetBytes >= idleRamThreshold && proc.cpuUsagePercent < 0.5) {
                if (tracker.idleCount == 0) tracker.idleFirstSeen = nowSteady;
                tracker.idleCount++;
                uint32_t sustainedSec = static_cast<uint32_t>(
                    std::chrono::duration_cast<std::chrono::seconds>(nowSteady - tracker.idleFirstSeen).count());

                if (sustainedSec >= 45) { // 45s sustained idle holding > 500 MB
                    triggerAlert("PROC_IDLE_BG_" + proc.name,
                                 AlertType::IdleBackground,
                                 AlertSeverity::Info,
                                 "Dormant Memory Allocation: " + proc.name,
                                 proc.name + " has been dormant in the background for " + std::to_string(sustainedSec) +
                                     "s while retaining " + formatBytes(proc.workingSetBytes) + " of physical RAM.",
                                 "Application has not consumed CPU cycles recently but retains a large resident memory footprint.",
                                 "Consider trimming working sets in the Safe Action Center or closing if no longer in use.",
                                 proc.name, proc.pid, proc.cpuUsagePercent, sustainedSec);
                }
            } else {
                tracker.idleCount = 0;
            }
        }
    }

    // Clean up stale process trackers
    for (auto it = m_processTrackers.begin(); it != m_processTrackers.end(); ) {
        if (activeTrackerKeys.find(it->first) == activeTrackerKeys.end()) {
            it = m_processTrackers.erase(it);
        } else {
            ++it;
        }
    }

    m_activeAlerts = activeAlerts;
    return {activeAlerts, newAlerts};
}

} // namespace pulse

