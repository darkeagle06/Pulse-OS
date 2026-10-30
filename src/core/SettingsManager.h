#pragma once

#include "Types.h"
#include <string>
#include <cstdint>

namespace pulse {

class SettingsManager {
public:
    SettingsManager();
    ~SettingsManager() = default;

    void load();
    void save();
    void resetToDefaults();

    // Sampling interval in milliseconds (500ms - 5000ms)
    [[nodiscard]] uint32_t getSamplingIntervalMs() const { return m_samplingIntervalMs; }
    void setSamplingIntervalMs(uint32_t ms);

    // Monitoring enabled state
    [[nodiscard]] bool isMonitoringEnabled() const { return m_monitoringEnabled; }
    void setMonitoringEnabled(bool enabled);

    // Auto-detect workload profile
    [[nodiscard]] bool isAutoProfileDetectionEnabled() const { return m_autoProfileDetection; }
    void setAutoProfileDetectionEnabled(bool enabled);

    // Active workload profile manual selection
    [[nodiscard]] WorkloadProfile getSelectedProfile() const { return m_selectedProfile; }
    void setSelectedProfile(WorkloadProfile profile);

    // Privacy & data collection settings
    [[nodiscard]] bool isDataCollectionTransparent() const { return m_transparentCollection; }
    void setDataCollectionTransparent(bool enabled);

    // Launch on Windows startup (disabled by default)
    [[nodiscard]] bool isLaunchOnStartupEnabled() const { return m_launchOnStartup; }
    void setLaunchOnStartupEnabled(bool enabled);

    // History chart buffer size (samples)
    [[nodiscard]] size_t getHistoryBufferSize() const { return m_historyBufferSize; }
    void setHistoryBufferSize(size_t size);

    // Default export directory
    [[nodiscard]] std::string getExportDirectory() const { return m_exportDirectory; }
    void setExportDirectory(const std::string& path);

    // Smart Resource Alerts Configuration
    [[nodiscard]] AlertSettings getAlertSettings() const { return m_alertSettings; }
    void setAlertSettings(const AlertSettings& settings);

private:
    uint32_t m_samplingIntervalMs{1000};
    bool m_monitoringEnabled{true};
    bool m_autoProfileDetection{true};
    WorkloadProfile m_selectedProfile{WorkloadProfile::Coding};
    bool m_transparentCollection{true};
    bool m_launchOnStartup{false};
    size_t m_historyBufferSize{60}; // 60 samples = 60s at 1s interval
    std::string m_exportDirectory;
    AlertSettings m_alertSettings;
};

} // namespace pulse
