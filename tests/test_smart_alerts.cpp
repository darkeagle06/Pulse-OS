#include <iostream>
#include <cassert>
#include <vector>
#include <thread>
#include <chrono>
#include "../src/core/SmartAlertEngine.h"

int main() {
    std::cout << "[TestSmartAlerts] Starting Smart Resource Alerts unit tests..." << std::endl;

    pulse::AlertSettings settings;
    settings.systemRamThresholdPercent = 80.0;
    settings.processCpuThresholdPercent = 25.0;
    settings.sustainedDurationSeconds = 0; // immediate for testing
    settings.cooldownDurationSeconds = 10;
    settings.ignoreList = {"System", "svchost.exe", "whitelisted_worker.exe"};

    pulse::SmartAlertEngine engine(settings);

    pulse::SystemMetrics nominalMetrics{};
    nominalMetrics.cpuUsagePercent = 20.0;
    nominalMetrics.ramUsagePercent = 50.0;
    nominalMetrics.totalRamBytes = 16ULL * 1024 * 1024 * 1024;
    nominalMetrics.usedRamBytes = 8ULL * 1024 * 1024 * 1024;
    nominalMetrics.availableRamBytes = 8ULL * 1024 * 1024 * 1024;

    std::vector<pulse::ProcessInfo> processes;
    pulse::TaskDetectionResult task{};

    // 1. Nominal System Test: No alerts triggered
    {
        auto [active, newAlerts] = engine.evaluate(nominalMetrics, processes, task);
        assert(active.empty());
        assert(newAlerts.empty());
        std::cout << "  ✓ Nominal metrics produce 0 alerts." << std::endl;
    }

    // 2. High System RAM Test: Triggers HighMemoryUsage alert
    {
        pulse::SystemMetrics highRamMetrics = nominalMetrics;
        highRamMetrics.ramUsagePercent = 92.0;
        highRamMetrics.availableRamBytes = 1ULL * 1024 * 1024 * 1024;

        auto [active, newAlerts] = engine.evaluate(highRamMetrics, processes, task);
        assert(!active.empty());
        assert(!newAlerts.empty());
        assert(active[0].type == pulse::AlertType::HighMemoryUsage);
        std::cout << "  ✓ High RAM condition correctly triggered HighMemoryUsage alert." << std::endl;

        // Immediately evaluating again should keep alert active but not trigger as "new" due to cooldown
        auto [active2, newAlerts2] = engine.evaluate(highRamMetrics, processes, task);
        assert(!active2.empty());
        assert(newAlerts2.empty());
        std::cout << "  ✓ Cooldown prevents duplicate notification dispatch." << std::endl;
    }

    // 3. Ignore List Test: Ignored process with high CPU does NOT alert
    {
        engine.clearCooldowns();
        pulse::ProcessInfo ignoredProc{};
        ignoredProc.pid = 999;
        ignoredProc.name = "whitelisted_worker.exe";
        ignoredProc.cpuUsagePercent = 60.0; // very high

        auto [active, newAlerts] = engine.evaluate(nominalMetrics, {ignoredProc}, task);
        assert(active.empty());
        assert(newAlerts.empty());
        std::cout << "  ✓ Ignore list successfully suppressed alert for whitelisted_worker.exe." << std::endl;
    }

    // 4. Non-ignored High CPU Process: Triggers HighCpuProcess alert
    {
        engine.clearCooldowns();
        pulse::ProcessInfo heavyProc{};
        heavyProc.pid = 4321;
        heavyProc.name = "rogue_task.exe";
        heavyProc.cpuUsagePercent = 45.0; // above 25% threshold

        auto [active, newAlerts] = engine.evaluate(nominalMetrics, {heavyProc}, task);
        assert(!active.empty());
        assert(!newAlerts.empty());
        bool found = false;
        for (const auto& a : active) {
            if (a.targetProcessName == "rogue_task.exe") {
                found = true;
                assert(a.type == pulse::AlertType::HighCpuProcess);
            }
        }
        assert(found);
        std::cout << "  ✓ Non-whitelisted high CPU process correctly detected." << std::endl;
    }

    // 5. Dismiss Alert Test
    {
        engine.dismissAlert("PROC_CPU_rogue_task.exe");
        pulse::ProcessInfo heavyProc{};
        heavyProc.pid = 4321;
        heavyProc.name = "rogue_task.exe";
        heavyProc.cpuUsagePercent = 45.0;

        auto [active, newAlerts] = engine.evaluate(nominalMetrics, {heavyProc}, task);
        for (const auto& a : active) {
            assert(a.targetProcessName != "rogue_task.exe");
        }
        std::cout << "  ✓ Dismissed alert is suppressed from subsequent reports." << std::endl;
    }

    // 6. History Tracking Test
    {
        auto history = engine.getAlertHistory();
        assert(!history.empty());
        std::cout << "  ✓ Alert history captured " << history.size() << " past events." << std::endl;
    }

    std::cout << "[TestSmartAlerts] ALL TESTS PASSED SUCCESSFULLY!" << std::endl;
    return 0;
}

