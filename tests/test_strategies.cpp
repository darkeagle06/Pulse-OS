#include <cassert>
#include <iostream>
#include <memory>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "strategies/CodingStrategy.h"
#include "strategies/GamingStrategy.h"
#include "strategies/StudyStrategy.h"
#include "strategies/RenderingStrategy.h"
#include "strategies/BatteryStrategy.h"

int main() {
    std::cout << "[TEST] Starting Strategy Hierarchy Unit Tests...\n";

    pulse::CodingStrategy coding;
    pulse::GamingStrategy gaming;
    pulse::StudyStrategy study;
    pulse::RenderingStrategy rendering;
    pulse::BatteryStrategy battery;

    // 1. Verify profile names and types
    assert(coding.getProfileType() == pulse::WorkloadProfile::Coding);
    assert(gaming.getProfileType() == pulse::WorkloadProfile::Gaming);
    assert(study.getProfileType() == pulse::WorkloadProfile::Study);
    assert(rendering.getProfileType() == pulse::WorkloadProfile::Rendering);
    assert(battery.getProfileType() == pulse::WorkloadProfile::BatterySaver);

    assert(!coding.getRationale().empty());
    assert(!gaming.getRationale().empty());
    assert(!study.getRationale().empty());
    assert(!rendering.getRationale().empty());
    assert(!battery.getRationale().empty());

    // 2. Verify power scheme GUIDs
    assert(!coding.getRecommendedPowerScheme().empty());
    assert(!gaming.getRecommendedPowerScheme().empty());
    assert(!study.getRecommendedPowerScheme().empty());
    assert(!rendering.getRecommendedPowerScheme().empty());
    assert(!battery.getRecommendedPowerScheme().empty());

    // 3. Test Evaluation on simulated metrics
    pulse::SystemMetrics highRamMetrics;
    highRamMetrics.ramUsagePercent = 90.0;
    highRamMetrics.cpuUsagePercent = 20.0;
    highRamMetrics.totalRamBytes = 16ULL * 1024 * 1024 * 1024;
    highRamMetrics.availableRamBytes = 16ULL * 1024 * 1024 * 102; // low available

    std::vector<pulse::ProcessInfo> processes;
    pulse::ProcessInfo compilerProc;
    compilerProc.pid = 4321;
    compilerProc.name = "g++.exe";
    compilerProc.cpuUsagePercent = 75.0;
    compilerProc.workingSetBytes = 500 * 1024 * 1024;
    processes.push_back(compilerProc);

    auto codingRecs = coding.evaluate(highRamMetrics, processes);
    assert(!codingRecs.empty());
    bool foundBuildRec = false;
    bool foundRamRec = false;
    for (const auto& r : codingRecs) {
        if (r.id.find("coding_active_build") != std::string::npos) foundBuildRec = true;
        if (r.id == "coding_ram_pressure") foundRamRec = true;
    }
    assert(foundBuildRec);
    assert(foundRamRec);

    // 4. Test Gaming Strategy evaluation
    pulse::ProcessInfo indexer;
    indexer.pid = 999;
    indexer.name = "SearchIndexer.exe";
    indexer.cpuUsagePercent = 15.0;
    indexer.priorityClass = NORMAL_PRIORITY_CLASS;
    processes.push_back(indexer);

    auto gamingRecs = gaming.evaluate(highRamMetrics, processes);
    bool foundContention = false;
    for (const auto& r : gamingRecs) {
        if (r.id.find("gaming_bg_interference") != std::string::npos) foundContention = true;
    }
    assert(foundContention);

    // 5. Test Available Actions
    assert(!coding.getAvailableActions().empty());
    assert(!gaming.getAvailableActions().empty());
    assert(!rendering.getAvailableActions().empty());
    assert(!battery.getAvailableActions().empty());

    std::cout << "[TEST] All Strategy Hierarchy Unit Tests PASSED successfully!\n";
    return 0;
}
