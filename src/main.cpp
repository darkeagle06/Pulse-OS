#include <QApplication>
#include <QStyleFactory>
#include <memory>

#include "core/SettingsManager.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    // Enable High DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("PulseOS");
    app.setOrganizationName("PulseOS");
    app.setApplicationVersion("1.0.0");

    // Register custom types for safe cross-thread Qt QueuedConnection delivery
    qRegisterMetaType<pulse::SystemMetrics>("pulse::SystemMetrics");
    qRegisterMetaType<std::vector<pulse::ProcessInfo>>("std::vector<pulse::ProcessInfo>");
    qRegisterMetaType<pulse::TaskDetectionResult>("pulse::TaskDetectionResult");
    qRegisterMetaType<std::vector<pulse::Recommendation>>("std::vector<pulse::Recommendation>");
    qRegisterMetaType<pulse::WorkloadProfile>("pulse::WorkloadProfile");
    qRegisterMetaType<pulse::InstalledAppInfo>("pulse::InstalledAppInfo");
    qRegisterMetaType<std::vector<pulse::InstalledAppInfo>>("std::vector<pulse::InstalledAppInfo>");
    qRegisterMetaType<pulse::ResourceAlert>("pulse::ResourceAlert");
    qRegisterMetaType<std::vector<pulse::ResourceAlert>>("std::vector<pulse::ResourceAlert>");

    // Initialize configuration
    auto settings = std::make_shared<pulse::SettingsManager>();

    // Instantiate and show Main Window
    pulse::ui::MainWindow window(settings);
    window.show();

    return app.exec();
}

