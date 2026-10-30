# PulseOS — Adaptive CPU Performance Assistant

**PulseOS** is an adaptive Windows desktop performance assistant engineered in modern C++20 and Qt 6. It collects genuine system metrics from Windows APIs, provides live system diagnostics and process exploration, implements 5 specialized workload optimization strategies, delivers transparent rule-based recommendations via a Safe Action Center, and performs before-and-after empirical verification.

---

## Key Highlights

- **Genuine Measurements:** Directly queries Windows APIs (`GetSystemTimes`, `GlobalMemoryStatusEx`, `CreateToolhelp32Snapshot`, `GetProcessTimes`, `GetProcessMemoryInfo`, `PowerGetActiveScheme`, Registry). **Zero fabricated metrics or placeholder telemetry.**
- **Dedicated Background Worker:** Multi-threaded architecture isolates all heavy Windows API sampling onto a background `QThread` (`MonitoringWorker`), delivering live data to the GUI thread via Qt queued signals. The UI thread never locks or stalls.
- **Safe Action Center:** All optimizations use strictly documented, safe Windows APIs with preview, confirmation, and full rollback capability. Never modifies kernel settings, never kills critical tasks, never requires administrator privileges for standard tuning.
- **Clear Separation of Observations vs. Diagnoses:** Distinguishes objective measured facts from interpretive hypotheses.
- **Empirical Before-and-After Analysis:** Measures actual observed deltas and validates background workload consistency before making claims of causation.
- **Privacy First:** 100% local operation. Telemetry and process information never leave the machine.

---

## Architecture & Object-Oriented Design

PulseOS follows strict object-oriented principles, RAII, smart pointers, const-correctness, and interface segregation. Windows-specific APIs are isolated behind clean, testable interfaces.

### Class Architecture Diagram

```
                             +-------------------+
                             |  ISystemMonitor   |
                             +-------------------+
                                       ^
                                       | implements
                             +-------------------+
                             |   SystemMonitor   | (Win32 GetSystemTimes, GlobalMemoryStatusEx)
                             +-------------------+
                                       ^
                                       | uses
+-------------------+        +-------------------+        +----------------------+
|  IProcessMonitor  |<-------|    MainWindow     |------->| PerformanceAnalyzer  |
+-------------------+        +-------------------+        +----------------------+
          ^                            |                             |
          | implements                 |                             | before/after
+-------------------+                  v                             v
|  ProcessMonitor   |        +-------------------+        +----------------------+
+-------------------+        |  SystemOptimizer  |        | ComparisonResult     |
                             +-------------------+        +----------------------+
                                       |
                   +-------------------+-------------------+
                   |                   |                   |
                   v                   v                   v
            [Power Schemes]    [Working Set Trim]  [Process Priority]
                   ^
                   | (evaluates & recommends)
+--------------------------------------------------------------+
|                    RecommendationEngine                      |
+--------------------------------------------------------------+
                               |
                               v
+--------------------------------------------------------------+
|                 OptimizationStrategy (Base)                  |
+--------------------------------------------------------------+
        ^              ^              ^            ^          ^
        |              |              |            |          |
+---------------+ +----------+ +-------------+ +-------+ +----------+
|CodingStrategy | | Gaming   | |StudyStrategy| |Render | |Battery   |
+---------------+ +----------+ +-------------+ +-------+ +----------+
```

### Core Components

| Component | Responsibility |
|---|---|
| `ISystemMonitor` / `SystemMonitor` | Collects genuine CPU utilization deltas, physical/available RAM, uptime, processor hardware details, and circular history buffers. |
| `IProcessMonitor` / `ProcessMonitor` | Enumerates active processes via Toolhelp32 snapshot, computes per-process CPU percentage across sampling deltas, queries working sets, priority classes, and foreground window info. |
| `TaskDetector` | Observes the active foreground application and system load patterns to automatically detect workload context with explicit confidence scoring and contributing factors. |
| `OptimizationStrategy` | Abstract strategy base defining workload profiles, rationale, power scheme recommendations, and custom evaluation criteria. |
| `CodingStrategy` | Tailored for compiler bursts, parallel builds, and IDE responsiveness. |
| `GamingStrategy` | Minimizes background scheduling contention and pins core clocks to reduce frametime variance. |
| `StudyStrategy` | Emphasizes cool thermals, quiet fan acoustics, and low baseline consumption for reading and research. |
| `RenderingStrategy` | Sustains 100% all-core throughput for exports/bakes and asserts Windows execution state to prevent unwanted sleep. |
| `BatteryStrategy` | Curbs turbo clock scaling and promotes deep CPU sleep states (C-states) on laptop battery. |
| `RecommendationEngine` | Evaluates sustained CPU load rules, memory pressure thresholds, runaway background processes, and autostart registry items. |
| `SystemOptimizer` | Safe Action Center engine executing verified Win32 power scheme switching, working set trimming, process priority adjustments, and sleep assertion with rollback tracking. |
| `PerformanceAnalyzer` | Captures baseline and post-optimization snapshots, validates workload comparability, and calculates verified deltas. |
| `ReportExporter` | Exports structured CSV metrics audits and formatted Markdown system reports. |
| `InstalledAppManager` | Scans registered Windows applications across 64-bit, 32-bit (Wow6432Node), and HKCU hives, evaluates verifiable usage evidence, and manages safe vendor uninstaller invocation with UAC elevation. |
| `SmartAlertEngine` | Evaluates sustained multi-second resource conditions (High Memory, High CPU processes, Heavy background apps, Idle background apps, Memory pressure) with repeat cooldowns, dismiss/snooze controls, and ignore-list filtering. |
| `SettingsManager` | Manages persistent user preferences (sampling rates, auto-detection, privacy, startup configuration, and alert thresholds). |

---

## Workload Profiles

| Profile | Target Power Scheme | Strategy Rationale |
|---|---|---|
| **Coding & Development** | Balanced (`381b4222-...`) | Low scheduling latency during editing, full turbo boost for build jobs, working set management for language servers. |
| **Gaming & High Performance** | High Performance (`8c5e7fda-...`) | Eliminates CPU clock down-stepping latency, minimizes background scheduling interruptions. |
| **Browsing & Studying** | Balanced (`381b4222-...`) | Suppresses unnecessary background spikes to maintain quiet acoustic fan curves. |
| **Video Editing & Rendering** | High Performance (`8c5e7fda-...`) | Uninterrupted rendering throughput; asserts thread execution state to prevent sleep during exports. |
| **Battery Saving** | Power Saver (`a1841308-...`) | Curbs power-hungry turbo frequencies to extend laptop battery life by 20–40%. |

---

## Safe Action Center Guarantees

1. **Pre-execution Preview:** Every optimization displays a modal dialog explaining what will change, expected impact, and rollback terms.
2. **Documented APIs Only:**
   - Power Schemes: `PowerSetActiveScheme` / `PowerGetActiveScheme`
   - Memory Management: `EmptyWorkingSet`
   - Process Priority: `SetPriorityClass`
   - Sleep Suppression: `SetThreadExecutionState`
3. **Rollback Stack:** Maintains a stack of applied actions. Power schemes and priority changes can be rolled back to their exact previous state with one click.
4. **Non-destructive:** Critical system processes (PID 0, 4) and security services are strictly protected and never touched.

---

## Building and Running

### Prerequisites

- **OS:** Windows 10/11 (64-bit)
- **Compiler:** C++20 compliant compiler (GCC 13+ / MinGW-w64 or MSVC 2022)
- **Build System:** CMake 3.22+ and Ninja
- **Framework:** Qt 6.8+ (Core, Gui, Widgets, Charts)

### Build Instructions

```powershell
# 1. Clone or open the repository directory
cd "PulseOS"

# 2. Configure build with CMake & Ninja
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# 3. Build executable and tests
cmake --build build

# 4. Run automated test suite
ctest --test-dir build --output-on-failure
```

### Running the Application

- **From Build:** `.\build\PulseOS.exe`
- **From Standalone Bundle:** `.\PulseOS_Run.bat` or `.\dist\PulseOS.exe`

---

## Automated Test Suite

PulseOS includes 8 automated unit and integration test targets:

1. `SanityTest`: Validates type conversions, defaults, and configuration persistence.
2. `MonitorsIntegrationTest`: Queries live Windows APIs on the host machine to verify CPU delta math, memory status, and process snapshotting.
3. `StrategiesTest`: Verifies strategy polymorphism, rationale strings, and recommendation evaluation on simulated loads.
4. `RecommendationsTest`: Verifies rule-based engines (memory pressure, runaway process, and startup registry queries).
5. `PerformanceAnalyzerTest`: Verifies before-and-after comparison math, delta calculations, and workload divergence detection.
6. `ProfilesNavigationTest`: Stress tests 7-page navigation, multi-threaded worker sampling, rapid profile switches, and validates zero UI stalls.
7. `InstalledAppsTest`: Validates registry discovery, metadata parsing, size formatting, category filtering, sorting, and safety invariants.
8. `SmartAlertsTest`: Validates sustained resource alerts, cooldown timers, ignore-list filtering, dismiss/snooze controls, and history tracking.

Run all tests:
```powershell
ctest --test-dir build --output-on-failure
```

---

## Feature Status

### ✅ Implemented Features
- [x] Genuine live total CPU utilization calculation via `GetSystemTimes` deltas
- [x] Genuine physical/available RAM tracking via `GlobalMemoryStatusEx`
- [x] Real-time CPU & RAM historical activity charts (`PerformanceChart`)
- [x] Hardware architecture inspector (Processor model, core counts, RAM)
- [x] Real Windows process explorer with PID, CPU%, memory, threads, priority, and access status
- [x] Process filtering, searching, and multi-field sorting
- [x] Graceful handling of restricted / access-denied system processes
- [x] Foreground window and task auto-detection with explicit confidence scoring (`TaskDetector`)
- [x] 5 complete OOP workload strategies (`Coding`, `Gaming`, `Study`, `Rendering`, `BatterySaver`)
- [x] Rule-based recommendation engine with objective observations vs. diagnostic hypotheses
- [x] Safe Action Center with preview modal, explicit consent, and rollback stack
- [x] Documented Windows power plan switching, working set trimming, process deprioritization, and sleep assertion
- [x] Before-and-after empirical performance snapshot analyzer with comparability validation
- [x] **Installed Application Manager**: Complete Windows registry scanner (64-bit, 32-bit, user keys), factual usage evidence tracking, categories (Recently Installed, Potentially Unused, Large Apps), and safe uninstaller invocation with UAC elevation
- [x] **Smart Resource Alerts & System Notifications**: Sustained high RAM, memory pressure, sustained high CPU per process, heavy and idle background app detection, customizable thresholds, repeat cooldowns, ignore-list management, and native Windows tray notifications
- [x] Structured CSV and Markdown audit report exporter
- [x] Persistent preferences management via `QSettings`
- [x] Polished dark desktop theme with responsive layouts
- [x] Automated unit and integration test suite (8 passing test suites, 100% pass rate)
- [x] Standalone distribution bundle deployed via `windeployqt`

### 📋 Planned Enhancements
- [ ] Per-core CPU load breakdown graph
- [ ] GPU utilization tracking via DXGI / NVML
- [ ] Scheduled recurring performance audit exporter

