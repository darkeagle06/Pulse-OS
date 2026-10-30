#pragma once

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <memory>

#include "../../core/Types.h"
#include "../../core/SettingsManager.h"
#include "../../core/ISystemMonitor.h"
#include "../../core/IProcessMonitor.h"
#include "../../core/SystemOptimizer.h"
#include "../../core/SmartAlertEngine.h"

namespace pulse::ui {

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(
        std::shared_ptr<SettingsManager> settings,
        std::shared_ptr<ISystemMonitor> sysMonitor,
        std::shared_ptr<IProcessMonitor> procMonitor,
        std::shared_ptr<SystemOptimizer> optimizer,
        std::shared_ptr<SmartAlertEngine> alertEngine = nullptr,
        QWidget* parent = nullptr);
    ~SettingsPage() override = default;

signals:
    void monitoringIntervalChanged(uint32_t intervalMs);
    void monitoringToggled(bool enabled);
    void alertSettingsChanged(const pulse::AlertSettings& settings);

private slots:
    void onSliderValueChanged(int val);
    void onMonitoringToggleClicked();
    void onAutoDetectToggled(bool checked);
    void onPrivacyToggled(bool checked);
    void onStartupToggled(bool checked);
    void onExportCsvClicked();
    void onExportAuditReportClicked();
    void onResetDefaultsClicked();

    // Alert settings slots
    void onAlertToggled();
    void onAlertSliderChanged();
    void onAddIgnoreClicked();
    void onRemoveIgnoreClicked();
    void onClearAlertHistoryClicked();

private:
    void setupUi();
    void populateAlertSettings();
    void refreshIgnoreList();

    std::shared_ptr<SettingsManager> m_settings;
    std::shared_ptr<ISystemMonitor> m_sysMonitor;
    std::shared_ptr<IProcessMonitor> m_procMonitor;
    std::shared_ptr<SystemOptimizer> m_optimizer;
    std::shared_ptr<SmartAlertEngine> m_alertEngine;

    // Telemetry controls
    QSlider* m_intervalSlider{nullptr};
    QLabel* m_intervalLabel{nullptr};
    QPushButton* m_monitorToggleBtn{nullptr};
    QCheckBox* m_autoDetectCheckbox{nullptr};
    QCheckBox* m_privacyCheckbox{nullptr};
    QCheckBox* m_startupCheckbox{nullptr};
    QLabel* m_exportStatusLabel{nullptr};

    // Alert settings controls
    QCheckBox* m_chkAlertHighMem{nullptr};
    QCheckBox* m_chkAlertHighCpu{nullptr};
    QCheckBox* m_chkAlertHeavyBg{nullptr};
    QCheckBox* m_chkAlertIdleBg{nullptr};
    QCheckBox* m_chkAlertPressure{nullptr};

    QSlider* m_sliderRamThreshold{nullptr};
    QLabel* m_lblRamThreshold{nullptr};
    QSlider* m_sliderCpuThreshold{nullptr};
    QLabel* m_lblCpuThreshold{nullptr};
    QSlider* m_sliderCooldown{nullptr};
    QLabel* m_lblCooldown{nullptr};

    QListWidget* m_ignoreListWidget{nullptr};
    QLineEdit* m_ignoreInput{nullptr};
    QPushButton* m_btnRemoveIgnore{nullptr};
    QLabel* m_alertHistoryStatus{nullptr};
};

} // namespace pulse::ui
