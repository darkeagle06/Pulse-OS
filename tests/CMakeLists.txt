cmake_minimum_required(VERSION 3.22)

# 1. Sanity Test
add_executable(test_sanity
    test_sanity.cpp
    ../src/core/SettingsManager.cpp
)
target_include_directories(test_sanity PRIVATE ../src ../src/core)
target_link_libraries(test_sanity PRIVATE Qt6::Core)
add_test(NAME SanityTest COMMAND test_sanity)

# 2. Monitors Integration Test (Real Windows APIs)
add_executable(test_monitors_integration
    test_monitors_integration.cpp
    ../src/core/SystemMonitor.cpp
    ../src/core/ProcessMonitor.cpp
)
target_include_directories(test_monitors_integration PRIVATE ../src ../src/core)
target_link_libraries(test_monitors_integration PRIVATE
    Qt6::Core
    psapi
    pdh
    powrprof
)
add_test(NAME MonitorsIntegrationTest COMMAND test_monitors_integration)

# 3. Strategies Unit Test
add_executable(test_strategies
    test_strategies.cpp
    ../src/core/strategies/CodingStrategy.cpp
    ../src/core/strategies/GamingStrategy.cpp
    ../src/core/strategies/StudyStrategy.cpp
    ../src/core/strategies/RenderingStrategy.cpp
    ../src/core/strategies/BatteryStrategy.cpp
)
target_include_directories(test_strategies PRIVATE ../src ../src/core)
target_link_libraries(test_strategies PRIVATE Qt6::Core)
add_test(NAME StrategiesTest COMMAND test_strategies)

# 4. Recommendations Unit Test
add_executable(test_recommendations
    test_recommendations.cpp
    ../src/core/RecommendationEngine.cpp
    ../src/core/strategies/CodingStrategy.cpp
    ../src/core/strategies/GamingStrategy.cpp
    ../src/core/strategies/StudyStrategy.cpp
    ../src/core/strategies/RenderingStrategy.cpp
    ../src/core/strategies/BatteryStrategy.cpp
)
target_include_directories(test_recommendations PRIVATE ../src ../src/core)
target_link_libraries(test_recommendations PRIVATE
    Qt6::Core
    powrprof
)
add_test(NAME RecommendationsTest COMMAND test_recommendations)

# 5. Performance Analyzer Unit Test
add_executable(test_performance_analyzer
    test_performance_analyzer.cpp
    ../src/core/PerformanceAnalyzer.cpp
)
target_include_directories(test_performance_analyzer PRIVATE ../src ../src/core)
target_link_libraries(test_performance_analyzer PRIVATE Qt6::Core)
add_test(NAME PerformanceAnalyzerTest COMMAND test_performance_analyzer)

# 6. Profiles Navigation Diagnostic Test
add_executable(test_profiles_navigation
    test_profiles_navigation.cpp
    ../src/core/SettingsManager.cpp
    ../src/core/SystemMonitor.cpp
    ../src/core/ProcessMonitor.cpp
    ../src/core/TaskDetector.cpp
    ../src/core/RecommendationEngine.cpp
    ../src/core/SystemOptimizer.cpp
    ../src/core/PerformanceAnalyzer.cpp
    ../src/core/ReportExporter.cpp
    ../src/core/MonitoringWorker.cpp
    ../src/core/InstalledAppManager.cpp
    ../src/core/SmartAlertEngine.cpp
    ../src/core/strategies/CodingStrategy.cpp
    ../src/core/strategies/GamingStrategy.cpp
    ../src/core/strategies/StudyStrategy.cpp
    ../src/core/strategies/RenderingStrategy.cpp
    ../src/core/strategies/BatteryStrategy.cpp
    ../src/ui/Theme.cpp
    ../src/ui/MainWindow.cpp
    ../src/ui/components/MetricCard.cpp
    ../src/ui/components/PerformanceChart.cpp
    ../src/ui/components/ProcessTableView.cpp
    ../src/ui/components/RecommendationCard.cpp
    ../src/ui/components/ActionConfirmDialog.cpp
    ../src/ui/components/UninstallConfirmDialog.cpp
    ../src/ui/pages/DashboardPage.cpp
    ../src/ui/pages/ProcessesPage.cpp
    ../src/ui/pages/ApplicationsPage.cpp
    ../src/ui/pages/ProfilesPage.cpp
    ../src/ui/pages/RecommendationsPage.cpp
    ../src/ui/pages/AnalysisPage.cpp
    ../src/ui/pages/SettingsPage.cpp
)
target_include_directories(test_profiles_navigation PRIVATE
    ../src
    ../src/core
    ../src/ui
)
target_link_libraries(test_profiles_navigation PRIVATE
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
add_test(NAME ProfilesNavigationTest COMMAND test_profiles_navigation)

# 7. Installed Application Manager Unit Test
add_executable(test_installed_apps
    test_installed_apps.cpp
    ../src/core/InstalledAppManager.cpp
)
target_include_directories(test_installed_apps PRIVATE ../src ../src/core)
target_link_libraries(test_installed_apps PRIVATE
    Qt6::Core
    advapi32
    shell32
)
add_test(NAME InstalledAppsTest COMMAND test_installed_apps)

# 8. Smart Resource Alerts Unit Test
add_executable(test_smart_alerts
    test_smart_alerts.cpp
    ../src/core/SmartAlertEngine.cpp
)
target_include_directories(test_smart_alerts PRIVATE ../src ../src/core)
target_link_libraries(test_smart_alerts PRIVATE
    Qt6::Core
)
add_test(NAME SmartAlertsTest COMMAND test_smart_alerts)
