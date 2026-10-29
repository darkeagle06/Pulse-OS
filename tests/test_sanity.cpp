#include <cassert>
#include <iostream>
#include "Types.h"
#include "SettingsManager.h"

int main() {
    std::cout << "[TEST] Running PulseOS Sanity Checks...\n";

    // Test profile string conversion
    assert(pulse::profileToString(pulse::WorkloadProfile::Coding) == "Coding & Development");
    assert(pulse::profileToString(pulse::WorkloadProfile::Gaming) == "Gaming & High Performance");
    assert(pulse::profileToString(pulse::WorkloadProfile::BatterySaver) == "Battery Saving");

    // Test SettingsManager defaults
    pulse::SettingsManager settings;
    settings.resetToDefaults();
    assert(settings.getSamplingIntervalMs() == 1000);
    assert(settings.isMonitoringEnabled() == true);
    assert(settings.isAutoProfileDetectionEnabled() == true);
    assert(settings.isLaunchOnStartupEnabled() == false);

    std::cout << "[TEST] Sanity Checks Passed Successfully!\n";
    return 0;
}

