#include "SettingsManager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QStringList>
#include <algorithm>

namespace pulse {

SettingsManager::SettingsManager() {
    m_exportDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toStdString();
    load();
}

void SettingsManager::load() {
    QSettings settings("PulseOS", "PulseOS");

    m_samplingIntervalMs = settings.value("Monitoring/SamplingIntervalMs", 1000).toUInt();
    m_samplingIntervalMs = std::clamp(m_samplingIntervalMs, 500u, 10000u);

    m_monitoringEnabled = settings.value("Monitoring/Enabled", true).toBool();
    m_autoProfileDetection = settings.value("Profiles/AutoDetection", true).toBool();

    int profileVal = settings.value("Profiles/Selected", static_cast<int>(WorkloadProfile::Coding)).toInt();
    if (profileVal >= 0 && profileVal <= static_cast<int>(WorkloadProfile::BatterySaver)) {
        m_selectedProfile = static_cast<WorkloadProfile>(profileVal);
    }

    m_transparentCollection = settings.value("Privacy/TransparentCollection", true).toBool();
    m_launchOnStartup = settings.value("General/LaunchOnStartup", false).toBool();
    m_historyBufferSize = settings.value("Monitoring/HistoryBufferSize", 60).toULongLong();

    QString exportDir = settings.value("General/ExportDirectory", "").toString();
    if (!exportDir.isEmpty() && QDir(exportDir).exists()) {
        m_exportDirectory = exportDir.toStdString();
    }

    // Smart Resource Alerts Settings
    m_alertSettings.enableHighMemory = settings.value("Alerts/EnableHighMemory", true).toBool();
    m_alertSettings.enableHighCpuProcess = settings.value("Alerts/EnableHighCpuProcess", true).toBool();
    m_alertSettings.enableHeavyBackground = settings.value("Alerts/EnableHeavyBackground", true).toBool();
    m_alertSettings.enableIdleBackground = settings.value("Alerts/EnableIdleBackground", true).toBool();
    m_alertSettings.enableMemoryPressure = settings.value("Alerts/EnableMemoryPressure", true).toBool();

    m_alertSettings.systemRamThresholdPercent = settings.value("Alerts/SystemRamThresholdPercent", 85.0).toDouble();
    m_alertSettings.processCpuThresholdPercent = settings.value("Alerts/ProcessCpuThresholdPercent", 30.0).toDouble();
    m_alertSettings.heavyBackgroundCpuPercent = settings.value("Alerts/HeavyBackgroundCpuPercent", 15.0).toDouble();
    m_alertSettings.sustainedDurationSeconds = settings.value("Alerts/SustainedDurationSeconds", 15).toUInt();
    m_alertSettings.cooldownDurationSeconds = settings.value("Alerts/CooldownDurationSeconds", 300).toUInt();

    QStringList ignoreList = settings.value("Alerts/IgnoreList").toStringList();
    if (!ignoreList.isEmpty()) {
        m_alertSettings.ignoreList.clear();
        for (const auto& item : ignoreList) {
            m_alertSettings.ignoreList.push_back(item.toStdString());
        }
    } else {
        m_alertSettings.ignoreList = {
            "System", "System Idle Process", "Registry", "smss.exe", "csrss.exe",
            "wininit.exe", "services.exe", "lsass.exe", "svchost.exe", "PulseOS.exe"
        };
    }
}

void SettingsManager::save() {
    QSettings settings("PulseOS", "PulseOS");

    settings.setValue("Monitoring/SamplingIntervalMs", m_samplingIntervalMs);
    settings.setValue("Monitoring/Enabled", m_monitoringEnabled);
    settings.setValue("Profiles/AutoDetection", m_autoProfileDetection);
    settings.setValue("Profiles/Selected", static_cast<int>(m_selectedProfile));
    settings.setValue("Privacy/TransparentCollection", m_transparentCollection);
    settings.setValue("General/LaunchOnStartup", m_launchOnStartup);
    settings.setValue("Monitoring/HistoryBufferSize", static_cast<qulonglong>(m_historyBufferSize));
    settings.setValue("General/ExportDirectory", QString::fromStdString(m_exportDirectory));

    // Smart Alerts
    settings.setValue("Alerts/EnableHighMemory", m_alertSettings.enableHighMemory);
    settings.setValue("Alerts/EnableHighCpuProcess", m_alertSettings.enableHighCpuProcess);
    settings.setValue("Alerts/EnableHeavyBackground", m_alertSettings.enableHeavyBackground);
    settings.setValue("Alerts/EnableIdleBackground", m_alertSettings.enableIdleBackground);
    settings.setValue("Alerts/EnableMemoryPressure", m_alertSettings.enableMemoryPressure);

    settings.setValue("Alerts/SystemRamThresholdPercent", m_alertSettings.systemRamThresholdPercent);
    settings.setValue("Alerts/ProcessCpuThresholdPercent", m_alertSettings.processCpuThresholdPercent);
    settings.setValue("Alerts/HeavyBackgroundCpuPercent", m_alertSettings.heavyBackgroundCpuPercent);
    settings.setValue("Alerts/SustainedDurationSeconds", m_alertSettings.sustainedDurationSeconds);
    settings.setValue("Alerts/CooldownDurationSeconds", m_alertSettings.cooldownDurationSeconds);

    QStringList qIgnoreList;
    for (const auto& item : m_alertSettings.ignoreList) {
        qIgnoreList.append(QString::fromStdString(item));
    }
    settings.setValue("Alerts/IgnoreList", qIgnoreList);

    settings.sync();
}

void SettingsManager::resetToDefaults() {
    m_samplingIntervalMs = 1000;
    m_monitoringEnabled = true;
    m_autoProfileDetection = true;
    m_selectedProfile = WorkloadProfile::Coding;
    m_transparentCollection = true;
    m_launchOnStartup = false;
    m_historyBufferSize = 60;
    m_exportDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toStdString();
    m_alertSettings = AlertSettings{};
    save();
}

void SettingsManager::setSamplingIntervalMs(uint32_t ms) {
    m_samplingIntervalMs = std::clamp(ms, 500u, 10000u);
    save();
}

void SettingsManager::setMonitoringEnabled(bool enabled) {
    m_monitoringEnabled = enabled;
    save();
}

void SettingsManager::setAutoProfileDetectionEnabled(bool enabled) {
    m_autoProfileDetection = enabled;
    save();
}

void SettingsManager::setSelectedProfile(WorkloadProfile profile) {
    m_selectedProfile = profile;
    save();
}

void SettingsManager::setDataCollectionTransparent(bool enabled) {
    m_transparentCollection = enabled;
    save();
}

void SettingsManager::setLaunchOnStartupEnabled(bool enabled) {
    m_launchOnStartup = enabled;
    save();
}

void SettingsManager::setHistoryBufferSize(size_t size) {
    m_historyBufferSize = std::clamp(size, size_t{30}, size_t{300});
    save();
}

void SettingsManager::setExportDirectory(const std::string& path) {
    m_exportDirectory = path;
    save();
}

void SettingsManager::setAlertSettings(const AlertSettings& settings) {
    m_alertSettings = settings;
    save();
}

} // namespace pulse
