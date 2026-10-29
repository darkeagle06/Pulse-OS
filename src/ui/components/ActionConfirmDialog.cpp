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
    shieldBadge->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::AccentGreen));
    headerLayout->addWidget(shieldBadge);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    auto* titleLabel = new QLabel(QString::fromStdString(m_action.title), this);
    titleLabel->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    auto* descLabel = new QLabel(QString::fromStdString(m_action.description), this);
    descLabel->setStyleSheet(QString("font-size: 13px; color: %1; line-height: 1.4;").arg(Theme::TextSecondary));
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    // Details Frame
    auto* detailsFrame = new QFrame(this);
    detailsFrame->setStyleSheet(Theme::panelStyleSheet(8));

    auto* detailsLayout = new QVBoxLayout(detailsFrame);
    detailsLayout->setContentsMargins(14, 14, 14, 14);
    detailsLayout->setSpacing(10);

    auto* impactLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Expected Impact:</span> "
                "<span style='color:%2;'>%3</span>")
            .arg(Theme::AccentCyan)
            .arg(Theme::TextPrimary)
            .arg(QString::fromStdString(m_action.expectedImpact)), detailsFrame);
    impactLabel->setWordWrap(true);
    detailsLayout->addWidget(impactLabel);

    auto* rollbackLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Rollback Support:</span> "
                "<span style='color:%2;'>%3</span>")
            .arg(Theme::AccentGreen)
            .arg(Theme::TextSecondary)
            .arg(m_action.canRollback ? QString::fromStdString(m_action.rollbackDescription) : "Automatic dynamic restoration by Windows."),
        detailsFrame);
    rollbackLabel->setWordWrap(true);
    detailsLayout->addWidget(rollbackLabel);

    auto* safetyLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Safety Guarantee:</span> "
                "<span style='color:%2;'>Uses documented Windows APIs only. No kernel tampering, no critical service alteration.</span>")
            .arg(Theme::AccentPurple)
            .arg(Theme::TextSecondary), detailsFrame);
    safetyLabel->setWordWrap(true);
    detailsLayout->addWidget(safetyLabel);

    layout->addWidget(detailsFrame);

    layout->addStretch();

    // Dialog buttons: Cancel & Confirm
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
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

