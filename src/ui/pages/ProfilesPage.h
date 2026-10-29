#pragma once

#include <QWidget>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
#include <memory>
#include <map>

#include "../../core/Types.h"
#include "../../core/SettingsManager.h"
#include "../../core/strategies/OptimizationStrategy.h"

namespace pulse::ui {

class ProfilesPage : public QWidget {
    Q_OBJECT

public:
    explicit ProfilesPage(std::shared_ptr<SettingsManager> settings, QWidget* parent = nullptr);
    ~ProfilesPage() override = default;

    void updateDetection(const TaskDetectionResult& detection);
    void setActiveProfile(WorkloadProfile profile);

signals:
    void profileChanged(WorkloadProfile profile);

private slots:
    void onProfileButtonClicked(int id);
    void onAutoDetectToggled(bool checked);

private:
    void setupUi();
    void updateProfileDetails(WorkloadProfile profile);

    std::shared_ptr<SettingsManager> m_settings;
    WorkloadProfile m_currentProfile{WorkloadProfile::Coding};

    // Auto-detect UI
    QLabel* m_detectedProfileLabel{nullptr};
    QProgressBar* m_confidenceBar{nullptr};
    QLabel* m_confidenceLabel{nullptr};
    QLabel* m_evidenceLabel{nullptr};
    QLabel* m_factorsLabel{nullptr};
    QCheckBox* m_autoDetectCheckbox{nullptr};

    // Profile buttons
    QButtonGroup* m_profileBtnGroup{nullptr};

    // Inspector
    QLabel* m_profileNameLabel{nullptr};
    QLabel* m_profileDescLabel{nullptr};
    QLabel* m_profileRationaleLabel{nullptr};
    QLabel* m_powerSchemeLabel{nullptr};
};

} // namespace pulse::ui

