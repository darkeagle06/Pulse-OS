#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

#include "../../core/Types.h"

namespace pulse::ui {

class ActionConfirmDialog : public QDialog {
    Q_OBJECT

public:
    explicit ActionConfirmDialog(const OptimizationAction& action, QWidget* parent = nullptr);
    ~ActionConfirmDialog() override = default;

private:
    void setupUi();

    OptimizationAction m_action;
};

} // namespace pulse::ui

