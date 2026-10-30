cmake_minimum_required(VERSION 3.22)
project(PulseOS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable automatic Qt code generators
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6 Packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts)

# Core Sources
set(PULSEOS_CORE_SOURCES
    src/core/SettingsManager.cpp
    src/core/SystemMonitor.cpp
    src/core/ProcessMonitor.cpp
    src/core/TaskDetector.cpp
    src/core/RecommendationEngine.cpp
    src/core/SystemOptimizer.cpp
    src/core/PerformanceAnalyzer.cpp
    src/core/ReportExporter.cpp
    src/core/MonitoringWorker.cpp
    src/core/InstalledAppManager.cpp
    src/core/SmartAlertEngine.cpp
    src/core/strategies/CodingStrategy.cpp
    src/core/strategies/GamingStrategy.cpp
    src/core/strategies/StudyStrategy.cpp
    src/core/strategies/RenderingStrategy.cpp
    src/core/strategies/BatteryStrategy.cpp
)

set(PULSEOS_CORE_HEADERS
    src/core/Types.h
    src/core/ISystemMonitor.h
    src/core/IProcessMonitor.h
    src/core/SystemMonitor.h
    src/core/ProcessMonitor.h
    src/core/TaskDetector.h
    src/core/RecommendationEngine.h
    src/core/SystemOptimizer.h
    src/core/PerformanceAnalyzer.h
    src/core/ReportExporter.h
    src/core/MonitoringWorker.h
    src/core/InstalledAppManager.h
    src/core/SmartAlertEngine.h
    src/core/DiagnosticLog.h
    src/core/strategies/OptimizationStrategy.h
    src/core/strategies/CodingStrategy.h
    src/core/strategies/GamingStrategy.h
    src/core/strategies/StudyStrategy.h
    src/core/strategies/RenderingStrategy.h
    src/core/strategies/BatteryStrategy.h
    src/core/SettingsManager.h
)

# UI Sources
set(PULSEOS_UI_SOURCES
    src/ui/Theme.cpp
    src/ui/MainWindow.cpp
    src/ui/components/MetricCard.cpp
    src/ui/components/PerformanceChart.cpp
    src/ui/components/ProcessTableView.cpp
    src/ui/components/RecommendationCard.cpp
    src/ui/components/ActionConfirmDialog.cpp
    src/ui/components/UninstallConfirmDialog.cpp
    src/ui/pages/DashboardPage.cpp
    src/ui/pages/ProcessesPage.cpp
    src/ui/pages/ApplicationsPage.cpp
    src/ui/pages/ProfilesPage.cpp
    src/ui/pages/RecommendationsPage.cpp
    src/ui/pages/AnalysisPage.cpp
    src/ui/pages/SettingsPage.cpp
)

set(PULSEOS_UI_HEADERS
    src/ui/Theme.h
    src/ui/MainWindow.h
    src/ui/components/MetricCard.h
    src/ui/components/PerformanceChart.h
    src/ui/components/ProcessTableView.h
    src/ui/components/RecommendationCard.h
    src/ui/components/ActionConfirmDialog.h
    src/ui/components/UninstallConfirmDialog.h
    src/ui/pages/DashboardPage.h
    src/ui/pages/ProcessesPage.h
    src/ui/pages/ApplicationsPage.h
    src/ui/pages/ProfilesPage.h
    src/ui/pages/RecommendationsPage.h
    src/ui/pages/AnalysisPage.h
    src/ui/pages/SettingsPage.h
)

# Executable target
add_executable(PulseOS
    src/main.cpp
    ${PULSEOS_CORE_SOURCES}
    ${PULSEOS_CORE_HEADERS}
    ${PULSEOS_UI_SOURCES}
    ${PULSEOS_UI_HEADERS}
)

target_include_directories(PulseOS PRIVATE
    src
    src/core
    src/ui
)

target_link_libraries(PulseOS PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
    psapi
    pdh
    powrprof
    version
    wtsapi32
    advapi32
    shell32
)

# Enable Testing
enable_testing()
add_subdirectory(tests)
