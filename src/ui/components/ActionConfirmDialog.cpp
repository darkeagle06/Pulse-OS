#include "ActionConfirmDialog.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

namespace pulse::ui {

ActionConfirmDialog::ActionConfirmDialog(const OptimizationAction& action, QWidget* parent)
    : QDialog(parent), m_action(action) {
    setupUi();
}

void ActionConfirmDialog::setupUi() {
    setWindowTitle("Safe Action Center — Optimization Preview");
    resize(520, 380);
    setModal(true);

    setStyleSheet(Theme::getApplicationStyleSheet());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    // Title & Badge
    auto* headerLayout = new QHBoxLayout();
    auto* shieldBadge = new QLabel("🛡 SAFE ACTION PREVIEW", this);
    shieldBadge->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 11px; font-weight: 700; "
        "border-radius: 4px; padding: 4px 8px;"
    ).arg(Theme::BgSurfaceAlt).arg(Theme::AccentGreen));
    headerLayout->addWidget(shieldBadge);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    auto* titleLabel = new QLabel(QString::fromStdString(m_action.title), this);
    titleLabel->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    auto* descLabel = new QLabel(QString::fromStdString(m_action.description), this);
    descLabel->setStyleSheet(QString("font-size: 13px; color: %1;").arg(Theme::TextSecondary));
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    // Details Frame
    auto* detailsFrame = new QFrame(this);
    detailsFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; padding: 12px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));

    auto* detailsLayout = new QVBoxLayout(detailsFrame);
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    detailsLayout->setSpacing(8);

    auto* impactLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Expected Impact:</span> %2")
            .arg(Theme::AccentBlue)
            .arg(QString::fromStdString(m_action.expectedImpact)), detailsFrame);
    impactLabel->setWordWrap(true);
    detailsLayout->addWidget(impactLabel);

    auto* rollbackLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Rollback Support:</span> %2")
            .arg(Theme::AccentGreen)
            .arg(m_action.canRollback ? QString::fromStdString(m_action.rollbackDescription) : "Automatic dynamic restoration by Windows."),
        detailsFrame);
    rollbackLabel->setWordWrap(true);
    detailsLayout->addWidget(rollbackLabel);

    auto* safetyLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Safety Guarantee:</span> Uses documented Windows APIs only. No kernel tampering, no critical service alteration.")
            .arg(Theme::AccentPurple), detailsFrame);
    safetyLabel->setWordWrap(true);
    detailsLayout->addWidget(safetyLabel);

    layout->addWidget(detailsFrame);

    layout->addStretch();

    // Dialog buttons: Cancel & Confirm
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setProperty("class", "actionBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto* confirmBtn = new QPushButton("Confirm & Apply Optimization", this);
    confirmBtn->setProperty("class", "primaryBtn");
    confirmBtn->setCursor(Qt::PointingHandCursor);
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(confirmBtn);

    layout->addLayout(btnLayout);
}

} // namespace pulse::ui

