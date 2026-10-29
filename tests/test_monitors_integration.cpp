#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

#include "SystemMonitor.h"
#include "ProcessMonitor.h"

int main() {
    std::cout << "[INTEGRATION TEST] Starting Windows Monitors Integration Test...\n";

    // 1. Test SystemMonitor
    pulse::SystemMonitor sysMonitor(60);

    // Initial hardware query verification
    std::string cpuName = sysMonitor.getCpuName();
    uint32_t physicalCores = sysMonitor.getPhysicalCoreCount();
    uint32_t logicalProcessors = sysMonitor.getLogicalProcessorCount();
    uint64_t totalRam = sysMonitor.getTotalPhysicalMemory();

    std::cout << "  Detected CPU: " << cpuName << "\n";
    std::cout << "  Physical Cores: " << physicalCores << ", Logical Processors: " << logicalProcessors << "\n";
    std::cout << "  Total Physical RAM: " << (totalRam / (1024 * 1024 * 1024)) << " GB\n";

    assert(logicalProcessors >= 1);
    assert(physicalCores >= 1);
    assert(totalRam > 0);

    // Sleep for 200ms to allow measuring delta
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    pulse::SystemMetrics metrics = sysMonitor.sample();
    std::cout << "  Live CPU Usage: " << metrics.cpuUsagePercent << "%\n";
    std::cout << "  Live RAM Usage: " << metrics.ramUsagePercent << "%\n";
    std::cout << "  Active Processes: " << metrics.activeProcessCount << ", Threads: " << metrics.activeThreadCount << "\n";
    std::cout << "  System Uptime: " << metrics.uptimeSeconds << " seconds\n";

    assert(metrics.isValid == true);
    assert(metrics.ramUsagePercent >= 0.0 && metrics.ramUsagePercent <= 100.0);
    assert(metrics.cpuUsagePercent >= 0.0 && metrics.cpuUsagePercent <= 100.0);
    assert(metrics.activeProcessCount > 0);

    // Verify history buffer
    auto cpuHistory = sysMonitor.getCpuHistory();
    auto ramHistory = sysMonitor.getRamHistory();
    assert(!cpuHistory.empty());
    assert(!ramHistory.empty());

    // 2. Test ProcessMonitor
    pulse::ProcessMonitor procMonitor;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto processes = procMonitor.sampleProcesses();
    std::cout << "  Enumerated " << processes.size() << " real Windows processes.\n";

    assert(!processes.empty());

    // Check that we have valid PIDs and names
    bool foundSelf = false;
    uint32_t currentPid = GetCurrentProcessId();

    for (const auto& p : processes) {
        if (p.pid == currentPid) {
            foundSelf = true;
            std::cout << "  Located self process: PID " << p.pid << " (" << p.name << ")\n";
            std::cout << "  Self Working Set: " << (p.workingSetBytes / 1024) << " KB\n";
            assert(p.workingSetBytes > 0);
            assert(p.accessStatus == pulse::ProcessAccessStatus::Accessible);
        }
    }
    assert(foundSelf);

    // Test filter helper
    auto filtered = pulse::ProcessMonitor::filterByName(processes, "pulse");
    assert(!filtered.empty());

    // Test sort helper
    pulse::ProcessMonitor::sortProcesses(processes, pulse::ProcessMonitor::SortField::MemoryDesc);
    assert(processes.front().workingSetBytes >= processes.back().workingSetBytes);

    std::cout << "[INTEGRATION TEST] All System and Process Monitor tests PASSED successfully!\n";
    return 0;
}

