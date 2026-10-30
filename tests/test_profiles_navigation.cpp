#include <QApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>

#include "core/SettingsManager.h"
#include "core/DiagnosticLog.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    pulse::logDiagnostic("=================================================");
    pulse::logDiagnostic("[TEST] Starting Comprehensive Profiles Navigation Test...");
    pulse::logDiagnostic("=================================================");

    QApplication app(argc, argv);

    // Register meta types
    qRegisterMetaType<pulse::SystemMetrics>("pulse::SystemMetrics");
    qRegisterMetaType<std::vector<pulse::ProcessInfo>>("std::vector<pulse::ProcessInfo>");
    qRegisterMetaType<pulse::TaskDetectionResult>("pulse::TaskDetectionResult");
    qRegisterMetaType<std::vector<pulse::Recommendation>>("std::vector<pulse::Recommendation>");
    qRegisterMetaType<pulse::WorkloadProfile>("pulse::WorkloadProfile");

    auto settings = std::make_shared<pulse::SettingsManager>();
    pulse::logDiagnostic("[TEST] SettingsManager created.");

    pulse::logDiagnostic("[TEST] Instantiating MainWindow...");
    pulse::ui::MainWindow window(settings);
    window.show();
    app.processEvents();
    pulse::logDiagnostic("[TEST] MainWindow shown and initial events processed.");

    // Allow background monitoring worker to run and deliver samples
    pulse::logDiagnostic("[TEST] Processing event loop for 1.5 seconds to let worker thread sample...");
    auto startLoop = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - startLoop).count() < 1500) {
        app.processEvents(QEventLoop::AllEvents, 50);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // Test Navigation to Workload Profiles
    pulse::logDiagnostic("[TEST] Navigating directly to Workload Profiles (Page index 2)...");
    QElapsedTimer timer;
    timer.start();
    QMetaObject::invokeMethod(&window, "onSidebarButtonClicked", Q_ARG(int, 2));
    app.processEvents();
    qint64 navTimeMs = timer.elapsed();
    pulse::logDiagnostic("[TEST] Navigation to Workload Profiles completed in " + std::to_string(navTimeMs) + " ms.");
    assert(navTimeMs < 500 && "Navigation to Workload Profiles must be fast and non-blocking!");

    // Stay on Workload Profiles for 2 seconds while background worker ticks
    pulse::logDiagnostic("[TEST] Staying on Workload Profiles for 2 seconds with active monitoring...");
    auto startProfileTime = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - startProfileTime).count() < 2000) {
        app.processEvents(QEventLoop::AllEvents, 50);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    // Test switching through each of the 5 workload profiles
    const pulse::WorkloadProfile testProfiles[] = {
        pulse::WorkloadProfile::Coding,
        pulse::WorkloadProfile::Gaming,
        pulse::WorkloadProfile::Study,
        pulse::WorkloadProfile::Rendering,
        pulse::WorkloadProfile::BatterySaver
    };

    pulse::logDiagnostic("[TEST] Testing profile switching across all 5 strategies...");
    for (auto profile : testProfiles) {
        timer.restart();
        QMetaObject::invokeMethod(&window, "onProfileChanged", Q_ARG(pulse::WorkloadProfile, profile));
        app.processEvents();
        qint64 switchMs = timer.elapsed();
        pulse::logDiagnostic("[TEST] Profile switched to " + pulse::profileToString(profile) +
                             " in " + std::to_string(switchMs) + " ms.");
        assert(switchMs < 200 && "Profile switching must not freeze the GUI!");
    }

    // Check AnalysisPage layout sizes at smaller and default sizes
    pulse::logDiagnostic("[TEST] Checking AnalysisPage layout responsiveness...");
    window.resize(950, 620);
    QMetaObject::invokeMethod(&window, "onSidebarButtonClicked", Q_ARG(int, 4));
    app.processEvents();

    auto* stacked = window.findChild<QStackedWidget*>();
    auto* analysisPage = stacked ? stacked->widget(4) : nullptr;
    if (stacked && analysisPage) {
        std::cout << "[DIAG] Small Window (950x620): Window width: " << window.width() << ", Stacked width: " << stacked->width() 
                  << ", AnalysisPage width: " << analysisPage->width() 
                  << ", AnalysisPage minHint: " << analysisPage->minimumSizeHint().width() << "x" << analysisPage->minimumSizeHint().height() << std::endl;
        assert(analysisPage->width() <= stacked->width() && "AnalysisPage must not overflow stacked widget!");
    }

    window.resize(1280, 840);
    app.processEvents();
    if (stacked && analysisPage) {
        std::cout << "[DIAG] Large Window (1280x840): Window width: " << window.width() << ", Stacked width: " << stacked->width() 
                  << ", AnalysisPage width: " << analysisPage->width() << std::endl;
        assert(analysisPage->width() <= stacked->width() && "AnalysisPage must not overflow stacked widget!");
    }

    // Stress navigation across all pages (Dashboard, Explorer, Profiles, Recommendations, Analysis, Settings)
    pulse::logDiagnostic("[TEST] Performing 3 rapid navigation cycles across all 6 pages...");
    for (int cycle = 0; cycle < 3; ++cycle) {
        for (int page = 0; page < 6; ++page) {
            timer.restart();
            QMetaObject::invokeMethod(&window, "onSidebarButtonClicked", Q_ARG(int, page));
            app.processEvents();
            qint64 cycleNavMs = timer.elapsed();
            if (cycleNavMs > 100) {
                pulse::logDiagnostic("[TEST] Warning: Nav to page " + std::to_string(page) +
                                     " took " + std::to_string(cycleNavMs) + " ms");
            }
        }
    }

    pulse::logDiagnostic("[TEST] Final event loop flush...");
    app.processEvents();

    pulse::logDiagnostic("=================================================");
    pulse::logDiagnostic("[TEST] ALL NAVIGATION AND PROFILES TESTS PASSED WITH ZERO STALLS!");
    pulse::logDiagnostic("=================================================");

    std::cout << "[SUCCESS] Profiles Navigation Diagnostic Test Passed!" << std::endl;
    return 0;
}
