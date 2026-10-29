#pragma once

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <memory>

#include "../../core/Types.h"
#include "../../core/SettingsManager.h"
#include "../../core/ISystemMonitor.h"
#include "../../core/IProcessMonitor.h"
#include "../../core/SystemOptimizer.h"

namespace pulse::ui {

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(
        std::shared_ptr<SettingsManager> settings,
        std::shared_ptr<ISystemMonitor> sysMonitor,
        std::shared_ptr<IProcessMonitor> procMonitor,
        std::shared_ptr<SystemOptimizer> optimizer,
        QWidget* parent = nullptr);
    ~SettingsPage() override = default;

signals:
    void monitoringIntervalChanged(uint32_t intervalMs);
    void monitoringToggled(bool enabled);

private slots:
    void onSliderValueChanged(int val);
    void onMonitoringToggleClicked();
    void onAutoDetectToggled(bool checked);
    void onPrivacyToggled(bool checked);
    void onStartupToggled(bool checked);
    void onExportCsvClicked();
    void onExportAuditReportClicked();
    void onResetDefaultsClicked();

private:
    void setupUi();

    std::shared_ptr<SettingsManager> m_settings;
    std::shared_ptr<ISystemMonitor> m_sysMonitor;
    std::shared_ptr<IProcessMonitor> m_procMonitor;
    std::shared_ptr<SystemOptimizer> m_optimizer;

    QSlider* m_intervalSlider{nullptr};
    QLabel* m_intervalLabel{nullptr};
    QPushButton* m_monitorToggleBtn{nullptr};
    QCheckBox* m_autoDetectCheckbox{nullptr};
    QCheckBox* m_privacyCheckbox{nullptr};
    QCheckBox* m_startupCheckbox{nullptr};
    QLabel* m_exportStatusLabel{nullptr};
};

} // namespace pulse::ui

