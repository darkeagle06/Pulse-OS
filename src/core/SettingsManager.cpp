#include "SettingsManager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
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

} // namespace pulse

