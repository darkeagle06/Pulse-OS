#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

#include "../../core/Types.h"

namespace pulse::ui {

class UninstallConfirmDialog : public QDialog {
    Q_OBJECT

public:
    explicit UninstallConfirmDialog(const InstalledAppInfo& appInfo, QWidget* parent = nullptr);
    ~UninstallConfirmDialog() override = default;

private:
    void setupUi();

    InstalledAppInfo m_appInfo;
};

} // namespace pulse::ui

