#include "UninstallConfirmDialog.h"
#include "../Theme.h"
#include "../../core/InstalledAppManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>

namespace pulse::ui {

UninstallConfirmDialog::UninstallConfirmDialog(const InstalledAppInfo& appInfo, QWidget* parent)
    : QDialog(parent), m_appInfo(appInfo) {
    setupUi();
}

void UninstallConfirmDialog::setupUi() {
    setWindowTitle("Uninstall Application — Confirmation");
    resize(560, 440);
    setModal(true);
    setStyleSheet(Theme::getApplicationStyleSheet());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    // Header badge
    auto* headerLayout = new QHBoxLayout();
    auto* badge = new QLabel("⚠ APPLICATION UNINSTALLATION", this);
    badge->setStyleSheet(Theme::pillStyleSheet(Theme::BgSurfaceAlt, Theme::AccentAmber));
    headerLayout->addWidget(badge);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    // App Title & Publisher
    auto* titleLabel = new QLabel(QString::fromStdString(m_appInfo.displayName), this);
    titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    QString pubVer = QString("Publisher: %1  •  Version: %2")
                         .arg(m_appInfo.publisher.empty() ? "Unknown" : QString::fromStdString(m_appInfo.publisher))
                         .arg(m_appInfo.displayVersion.empty() ? "N/A" : QString::fromStdString(m_appInfo.displayVersion));
    auto* subLabel = new QLabel(pubVer, this);
    subLabel->setStyleSheet(QString("font-size: 13px; color: %1;").arg(Theme::TextSecondary));
    layout->addWidget(subLabel);

    // Metadata Card
    auto* detailsFrame = new QFrame(this);
    detailsFrame->setStyleSheet(Theme::panelStyleSheet(8));
    auto* detailsLayout = new QVBoxLayout(detailsFrame);
    detailsLayout->setContentsMargins(14, 14, 14, 14);
    detailsLayout->setSpacing(8);

    auto* sizeLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Estimated Disk Size:</span> <span style='color:%2;'>%3</span>")
            .arg(Theme::TextSecondary)
            .arg(Theme::TextPrimary)
            .arg(QString::fromStdString(m_appInfo.formattedSize())), detailsFrame);
    detailsLayout->addWidget(sizeLabel);

    auto* dateLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Install Date:</span> <span style='color:%2;'>%3</span>")
            .arg(Theme::TextSecondary)
            .arg(Theme::TextPrimary)
            .arg(m_appInfo.installDate.empty() ? "Not recorded" : QString::fromStdString(m_appInfo.installDate)), detailsFrame);
    detailsLayout->addWidget(dateLabel);

    if (!m_appInfo.installLocation.empty()) {
        auto* locLabel = new QLabel(
            QString("<span style='color:%1; font-weight:600;'>Location:</span> <span style='color:%2;'>%3</span>")
                .arg(Theme::TextSecondary)
                .arg(Theme::TextMuted)
                .arg(QString::fromStdString(m_appInfo.installLocation)), detailsFrame);
        locLabel->setWordWrap(true);
        detailsLayout->addWidget(locLabel);
    }

    if (!m_appInfo.usageEvidence.empty()) {
        auto* evLabel = new QLabel(
            QString("<span style='color:%1; font-weight:600;'>Usage Evidence:</span> <span style='color:%2;'>%3</span>")
                .arg(Theme::AccentCyan)
                .arg(Theme::TextPrimary)
                .arg(QString::fromStdString(m_appInfo.usageEvidence)), detailsFrame);
        evLabel->setWordWrap(true);
        detailsLayout->addWidget(evLabel);
    }

    layout->addWidget(detailsFrame);

    // Safety and Elevation Notice Frame
    auto* noticeFrame = new QFrame(this);
    noticeFrame->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 8px;")
                                   .arg(Theme::BgSurfaceAlt)
                                   .arg(Theme::BorderSubtle));
    auto* noticeLayout = new QVBoxLayout(noticeFrame);
    noticeLayout->setContentsMargins(12, 12, 12, 12);
    noticeLayout->setSpacing(6);

    auto* noticeTitle = new QLabel("Safety Guarantee & Procedure", noticeFrame);
    noticeTitle->setStyleSheet(QString("font-size: 12px; font-weight: 700; color: %1;").arg(Theme::AccentGreen));
    noticeLayout->addWidget(noticeTitle);

    auto* noticeText = new QLabel(
        "• PulseOS uses the official vendor uninstaller registered in Windows.\n"
        "• It never deletes application files, folders, or registry entries directly.\n"
        "• Windows User Account Control (UAC) may request administrator approval.", noticeFrame);
    noticeText->setStyleSheet(QString("font-size: 12px; color: %1; line-height: 1.4;").arg(Theme::TextSecondary));
    noticeLayout->addWidget(noticeText);

    layout->addWidget(noticeFrame);
    layout->addStretch();

    // Action buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setProperty("class", "actionBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto* confirmBtn = new QPushButton("Proceed to Uninstall", this);
    confirmBtn->setStyleSheet(
        QString("background-color: #be123c; color: #ffffff; font-weight: 600; padding: 8px 18px; border-radius: 6px; border: none;")
    );
    confirmBtn->setCursor(Qt::PointingHandCursor);
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(confirmBtn);

    layout->addLayout(btnLayout);
}

} // namespace pulse::ui

