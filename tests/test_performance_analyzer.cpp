#include <cassert>
#include <iostream>
#include <cmath>

#include "PerformanceAnalyzer.h"

int main() {
    std::cout << "[TEST] Starting PerformanceAnalyzer Unit Tests...\n";

    pulse::PerformanceAnalyzer analyzer;

    // Initially no comparison should be available
    assert(!analyzer.compare().has_value());

    // 1. Capture Baseline
    pulse::SystemMetrics baselineMetrics;
    baselineMetrics.cpuUsagePercent = 45.0;
    baselineMetrics.ramUsagePercent = 75.0;
    baselineMetrics.usedRamBytes = 12ULL * 1024 * 1024 * 1024;

    std::vector<pulse::ProcessInfo> baselineProcs(100);
    analyzer.captureBaseline("Initial Baseline", baselineMetrics, baselineProcs, "Coding & Development");
    assert(analyzer.getBaseline().has_value());
    assert(!analyzer.compare().has_value());

    // 2. Capture Post-Optimization with measurable improvement
    pulse::SystemMetrics postMetrics;
    postMetrics.cpuUsagePercent = 38.0; // 7% reduction
    postMetrics.ramUsagePercent = 70.0; // 5% reduction
    postMetrics.usedRamBytes = 11ULL * 1024 * 1024 * 1024; // 1 GB freed

    std::vector<pulse::ProcessInfo> postProcs(102); // 2 process delta = comparable
    analyzer.capturePostOptimization("After Working Set Trim", postMetrics, postProcs, "Coding & Development");
    assert(analyzer.getPostOptimization().has_value());

    // 3. Evaluate Comparison
    auto comp = analyzer.compare();
    assert(comp.has_value());

    const auto& c = *comp;
    std::cout << "  Calculated CPU Delta: " << c.cpuDeltaPercent << "%\n";
    std::cout << "  Calculated RAM Delta: " << c.ramDeltaPercent << "%\n";
    std::cout << "  Calculated RAM Bytes Freed: " << (-c.ramDeltaBytes / (1024 * 1024)) << " MB\n";

    assert(std::abs(c.cpuDeltaPercent - (-7.0)) < 0.001);
    assert(std::abs(c.ramDeltaPercent - (-5.0)) < 0.001);
    assert(c.ramDeltaBytes < 0);
    assert(c.workloadAppearsComparable == true);
    assert(c.comparabilityWarning.empty());
    assert(!c.summaryMessage.empty());

    // 4. Test Incomparable Workload (e.g. 50 new processes launched during test)
    std::vector<pulse::ProcessInfo> divergentProcs(180);
    analyzer.capturePostOptimization("Incomparable Sample", postMetrics, divergentProcs, "Coding & Development");

    auto divergentComp = analyzer.compare();
    assert(divergentComp.has_value());
    assert(divergentComp->workloadAppearsComparable == false);
    assert(!divergentComp->comparabilityWarning.empty());
    std::cout << "  Successfully detected incomparable workload: " << divergentComp->comparabilityWarning << "\n";

    std::cout << "[TEST] All PerformanceAnalyzer Unit Tests PASSED successfully!\n";
    return 0;
}

