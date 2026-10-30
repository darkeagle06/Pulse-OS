#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <chrono>
#include <mutex>
#include "Types.h"

namespace pulse {

class SmartAlertEngine {
public:
    explicit SmartAlertEngine(AlertSettings settings = {});
    virtual ~SmartAlertEngine() = default;

    // Evaluates live metrics, processes and task detection
    // Returns pair of (activeAlerts, newAlertsTriggeredThisTick)
    [[nodiscard]] virtual std::pair<std::vector<ResourceAlert>, std::vector<ResourceAlert>> evaluate(
        const SystemMetrics& sysMetrics,
        const std::vector<ProcessInfo>& processes,
        const TaskDetectionResult& taskDetection);

    // Configuration
    void setSettings(const AlertSettings& settings);
    [[nodiscard]] AlertSettings getSettings() const;

    // Ignore-list management
    void addToIgnoreList(const std::string& processName);
    void removeFromIgnoreList(const std::string& processName);
    [[nodiscard]] bool isIgnored(const std::string& processName) const;

    // Snooze / Dismiss controls
    void snoozeAlert(const std::string& alertId, uint32_t durationMinutes = 15);
    void dismissAlert(const std::string& alertId);
    void clearCooldowns();

    // In-memory alert history log
    [[nodiscard]] std::vector<ResourceAlert> getAlertHistory() const;
    void clearHistory();

private:
    struct ProcessTracker {
        uint32_t pid{0};
        std::string name;
        std::chrono::steady_clock::time_point highCpuFirstSeen;
        uint32_t highCpuCount{0};
        std::chrono::steady_clock::time_point heavyBgFirstSeen;
        uint32_t heavyBgCount{0};
        std::chrono::steady_clock::time_point idleFirstSeen;
        uint32_t idleCount{0};
    };

    bool isCoolingDown(const std::string& alertKey, const std::chrono::steady_clock::time_point& now);
    void recordAlertTriggered(const std::string& alertKey, const std::chrono::steady_clock::time_point& now);

    mutable std::mutex m_mutex;
    AlertSettings m_settings;

    // Sustained tracking
    std::map<std::string, ProcessTracker> m_processTrackers; // Keyed by process name + pid
    std::chrono::steady_clock::time_point m_highRamFirstSeen;
    uint32_t m_highRamCount{0};

    // Cooldown, snooze and dismiss state
    std::map<std::string, std::chrono::steady_clock::time_point> m_cooldowns;
    std::map<std::string, std::chrono::steady_clock::time_point> m_snoozeUntil;
    std::set<std::string> m_dismissed;

    // History and active alerts
    std::vector<ResourceAlert> m_alertHistory;
    std::vector<ResourceAlert> m_activeAlerts;
};

} // namespace pulse

