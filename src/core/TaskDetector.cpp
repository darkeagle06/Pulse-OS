#include "TaskDetector.h"
#include "DiagnosticLog.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>

namespace pulse {

TaskDetector::TaskDetector(std::shared_ptr<IProcessMonitor> procMonitor,
                           std::shared_ptr<ISystemMonitor> sysMonitor)
    : m_procMonitor(std::move(procMonitor)), m_sysMonitor(std::move(sysMonitor)) {}

bool TaskDetector::isRunningOnBattery() {
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        // ACLineStatus: 0 = Offline (Battery), 1 = Online (AC), 255 = Unknown
        return (sps.ACLineStatus == 0);
    }
    return false;
}

TaskDetectionResult TaskDetector::detect() {
    TaskDetectionResult result;
    result.timestamp = std::chrono::system_clock::now();

    if (!m_procMonitor || !m_sysMonitor) {
        result.detectedProfile = WorkloadProfile::Study;
        result.confidence = 0.2;
        result.primaryEvidence = "Monitors uninitialized; falling back to default profile.";
        return result;
    }

    // 1. Get foreground application
    auto fg = m_procMonitor->getForegroundProcessInfo();
    result.foregroundProcessName = fg.processName;
    result.foregroundWindowTitle = fg.windowTitle;

    std::string fgLower = fg.processName;
    std::transform(fgLower.begin(), fgLower.end(), fgLower.begin(), ::tolower);

    std::string titleLower = fg.windowTitle;
    std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);

    // 2. Query system metrics
    SystemMetrics sysMetrics = m_sysMonitor->getLastMetrics();
    bool onBattery = isRunningOnBattery();

    // 3. Heuristic Matching

    // A. Check for Battery condition first if on battery and low power state
    if (onBattery) {
        result.contributingFactors.push_back("System is operating on battery power (AC disconnected).");
    }

    // B. Check if PulseOS itself is the foreground application
    if (fgLower == "pulseos.exe" || fgLower == "test_profiles_navigation.exe") {
        result.detectedProfile = WorkloadProfile::AutoDetected;
        result.confidence = 0.50;
        result.primaryEvidence = "PulseOS Performance Assistant is currently the active foreground window.";
        result.contributingFactors.push_back("Select a workload profile card below or switch to an external app to auto-detect.");
        logDiagnostic("[TaskDetector] Detected foreground: PulseOS active (AutoDetected / retain manual profile)");
        return result;
    }

    // C. Check for Coding tools
    if (fgLower == "code.exe" || fgLower == "devenv.exe" || fgLower == "clion64.exe" ||
        fgLower == "idea64.exe" || fgLower == "pycharm64.exe" || fgLower == "windowsterminal.exe" ||
        fgLower == "powershell.exe" || fgLower == "cmd.exe" || fgLower == "qtcreator.exe" ||
        titleLower.find("visual studio") != std::string::npos ||
        titleLower.find(".cpp") != std::string::npos ||
        titleLower.find(".py") != std::string::npos) {

        result.detectedProfile = WorkloadProfile::Coding;
        result.confidence = 0.88;
        result.primaryEvidence = "Foreground process is an active IDE or development terminal (" + fg.processName + ").";
        result.contributingFactors.push_back("Window title: \"" + fg.windowTitle + "\"");
        logDiagnostic("[TaskDetector] Detected foreground: " + fg.processName + " -> Coding (88%)");
        return result;
    }

    // D. Check for Gaming
    if (fgLower == "steam.exe" || fgLower == "epicgameslauncher.exe" ||
        fgLower.find("game") != std::string::npos ||
        fgLower == "valorant.exe" || fgLower == "cs2.exe" || fgLower == "dota2.exe" ||
        fgLower == "minecraft.exe" || fgLower == "gta5.exe") {

        result.detectedProfile = WorkloadProfile::Gaming;
        result.confidence = 0.92;
        result.primaryEvidence = "Foreground process is a verified gaming application (" + fg.processName + ").";
        result.contributingFactors.push_back("High framerate graphic rendering profile appropriate.");
        logDiagnostic("[TaskDetector] Detected foreground: " + fg.processName + " -> Gaming (92%)");
        return result;
    }

    // E. Check for Video Editing & Rendering
    if (fgLower == "blender.exe" || fgLower == "adobe premiere pro.exe" ||
        fgLower == "afterfx.exe" || fgLower == "resolve.exe" || fgLower == "handbrake.exe") {

        result.detectedProfile = WorkloadProfile::Rendering;
        result.confidence = 0.90;
        result.primaryEvidence = "Foreground process is a creative rendering or video production suite (" + fg.processName + ").";
        if (sysMetrics.cpuUsagePercent > 60.0) {
            result.confidence = 0.95;
            result.contributingFactors.push_back("CPU utilization is high (" +
                std::to_string(static_cast<int>(sysMetrics.cpuUsagePercent)) + "%), consistent with active rendering.");
        }
        logDiagnostic("[TaskDetector] Detected foreground: " + fg.processName + " -> Rendering");
        return result;
    }

    // F. Check for Battery Saver profile if on battery with moderate/low load
    if (onBattery && sysMetrics.cpuUsagePercent < 30.0) {
        result.detectedProfile = WorkloadProfile::BatterySaver;
        result.confidence = 0.75;
        result.primaryEvidence = "Machine is running on battery with low-to-moderate computational demand.";
        result.contributingFactors.push_back("Power saving policies recommended to extend battery duration.");
        logDiagnostic("[TaskDetector] Detected state: On Battery -> BatterySaver (75%)");
        return result;
    }

    // G. Browser and Study Detection
    if (fgLower == "chrome.exe" || fgLower == "msedge.exe" || fgLower == "firefox.exe" ||
        fgLower == "brave.exe" || fgLower == "acrobat.exe" || fgLower == "notion.exe" ||
        fgLower == "onenote.exe") {

        result.detectedProfile = WorkloadProfile::Study;
        result.confidence = 0.70;
        result.primaryEvidence = "Foreground application is a document viewer or web browser (" + fg.processName + ").";
        result.contributingFactors.push_back("Window: \"" + fg.windowTitle + "\"");
        result.contributingFactors.push_back("Uncertainty: Web browsers may be used for coding, video streaming, or studying.");
        logDiagnostic("[TaskDetector] Detected foreground: " + fg.processName + " -> Study (70%)");
        return result;
    }

    // H. Default / Low confidence fallback
    result.detectedProfile = WorkloadProfile::Study;
    result.confidence = 0.45;
    result.primaryEvidence = "Unidentified or standard desktop application in foreground (" +
                             (fg.processName.empty() ? "Desktop/System Shell" : fg.processName) + ").";
    result.contributingFactors.push_back("Low confidence estimation; manual profile selection recommended.");
    logDiagnostic("[TaskDetector] Detected foreground: " + fg.processName + " -> Low confidence Study (45%)");
    return result;
}

} // namespace pulse

