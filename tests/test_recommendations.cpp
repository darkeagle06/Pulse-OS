#include <cassert>
#include <iostream>
#include <memory>

#include "RecommendationEngine.h"
#include "strategies/CodingStrategy.h"

int main() {
    std::cout << "[TEST] Starting RecommendationEngine Unit Tests...\n";

    pulse::RecommendationEngine engine;
    auto strategy = std::make_shared<pulse::CodingStrategy>();

    // Test 1: Memory Pressure Rule
    pulse::SystemMetrics memPressureMetrics;
    memPressureMetrics.ramUsagePercent = 89.0;
    memPressureMetrics.availableRamBytes = 1ULL * 1024 * 1024 * 1024;
    memPressureMetrics.totalRamBytes = 16ULL * 1024 * 1024 * 1024;
    memPressureMetrics.cpuUsagePercent = 25.0;

    std::vector<pulse::ProcessInfo> emptyProcesses;
    auto recs = engine.generateRecommendations(memPressureMetrics, emptyProcesses, strategy);

    bool foundMemRule = false;
    for (const auto& r : recs) {
        if (r.id == "rule_high_ram_pressure") {
            foundMemRule = true;
            assert(!r.observation.empty());
            assert(!r.diagnosis.empty());
            assert(!r.recommendedAction.empty());
            assert(r.isActionable);
            assert(r.level == pulse::RecommendationLevel::Warning);
        }
    }
    assert(foundMemRule);

    // Test 2: Runaway Process Rule
    pulse::ProcessInfo runaway;
    runaway.pid = 8888;
    runaway.name = "unconstrained_worker.exe";
    runaway.cpuUsagePercent = 45.0;
    std::vector<pulse::ProcessInfo> procList = { runaway };

    pulse::SystemMetrics normalMetrics;
    normalMetrics.ramUsagePercent = 40.0;
    normalMetrics.cpuUsagePercent = 50.0;

    auto recsWithRunaway = engine.generateRecommendations(normalMetrics, procList, strategy);
    bool foundRunawayRule = false;
    for (const auto& r : recsWithRunaway) {
        if (r.id == "rule_heavy_proc_8888") {
            foundRunawayRule = true;
            assert(r.observation.find("unconstrained_worker.exe") != std::string::npos);
        }
    }
    assert(foundRunawayRule);

    // Test 3: Startup Applications Query
    auto startupApps = pulse::RecommendationEngine::queryStartupApplications();
    std::cout << "  Enumerated " << startupApps.size() << " Windows startup applications.\n";
    // System may have 0 or more, but the function should succeed without crash

    std::cout << "[TEST] All RecommendationEngine Unit Tests PASSED successfully!\n";
    return 0;
}

