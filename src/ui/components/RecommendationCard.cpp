#include "RecommendationCard.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

namespace pulse::ui {

RecommendationCard::RecommendationCard(const Recommendation& rec, QWidget* parent)
    : QFrame(parent), m_rec(rec) {
    setupUi();
}

void RecommendationCard::setupUi() {
    QString leftAccent;
    QString badgeText, badgeBg, badgeFg;

    switch (m_rec.level) {
        case RecommendationLevel::Critical:
            badgeText = "CRITICAL";
            badgeBg = "#3b1219";
            badgeFg = "#f43f5e";
            leftAccent = Theme::AccentRose;
            break;
        case RecommendationLevel::Warning:
            badgeText = "WARNING";
            badgeBg = "#38240b";
            badgeFg = "#f59e0b";
            leftAccent = Theme::AccentAmber;
            break;
        case RecommendationLevel::Suggestion:
            badgeText = "SUGGESTION";
            badgeBg = "#102a45";
            badgeFg = "#38bdf8";
            leftAccent = Theme::AccentCyan;
            break;
        case RecommendationLevel::Info:
        default:
            badgeText = "INFO";
            badgeBg = "#1f2638";
            badgeFg = "#94a3b8";
            leftAccent = Theme::BorderSubtle;
            break;
    }

    setObjectName("recCard");
    setStyleSheet(QString(
        "#recCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-left: 3px solid %3;"
        "  border-radius: 8px;"
        "}"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle).arg(leftAccent));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(10);

    // Header layout: Level Badge + Title + Action Button
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    // Level Badge
    auto* badge = new QLabel(this);
    badge->setText(badgeText);
    badge->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 10px; font-weight: 700; "
        "border-radius: 4px; padding: 3px 8px; letter-spacing: 0.5px;"
    ).arg(badgeBg).arg(badgeFg));
    headerLayout->addWidget(badge);

    // Title
    auto* titleLabel = new QLabel(QString::fromStdString(m_rec.title), this);
    titleLabel->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1;").arg(Theme::TextPrimary));
    headerLayout->addWidget(titleLabel, 1);

    // Action button if actionable
    if (m_rec.isActionable) {
        m_actionBtn = new QPushButton("Apply Optimization", this);
        m_actionBtn->setProperty("class", "primaryBtn");
        m_actionBtn->setCursor(Qt::PointingHandCursor);
        connect(m_actionBtn, &QPushButton::clicked, this, [this]() {
            emit actionRequested(m_rec);
        });
        headerLayout->addWidget(m_actionBtn);
    }

    layout->addLayout(headerLayout);

    // Section 1: Objective Observation & Technical Diagnosis
    auto* telemetryLayout = new QVBoxLayout();
    telemetryLayout->setSpacing(4);

    auto* obsLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600; font-size:11px; text-transform:uppercase; letter-spacing:0.5px;'>Observation:</span> "
                "<span style='color:%2; font-size:12px;'>%3</span>")
            .arg(Theme::AccentCyan)
            .arg(Theme::TextPrimary)
            .arg(QString::fromStdString(m_rec.observation)), this);
    obsLabel->setWordWrap(true);
    telemetryLayout->addWidget(obsLabel);

    auto* diagLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600; font-size:11px; text-transform:uppercase; letter-spacing:0.5px;'>Diagnosis:</span> "
                "<span style='color:%2; font-size:12px;'>%3</span>")
            .arg(Theme::AccentAmber)
            .arg(Theme::TextSecondary)
            .arg(QString::fromStdString(m_rec.diagnosis)), this);
    diagLabel->setWordWrap(true);
    telemetryLayout->addWidget(diagLabel);

    layout->addLayout(telemetryLayout);

    // Section 2: Recommended Action & Explanation
    auto* actionLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600; font-size:11px; text-transform:uppercase; letter-spacing:0.5px;'>Action:</span> "
                "<span style='color:%2; font-size:13px; font-weight:600;'>%3</span>")
            .arg(Theme::TextSecondary)
            .arg(Theme::TextPrimary)
            .arg(QString::fromStdString(m_rec.recommendedAction)), this);
    actionLabel->setWordWrap(true);
    layout->addWidget(actionLabel);

    if (!m_rec.explanation.empty()) {
        auto* explLabel = new QLabel(QString::fromStdString(m_rec.explanation), this);
        explLabel->setStyleSheet(QString("font-size: 12px; color: %1; line-height: 1.4;").arg(Theme::TextSecondary));
        explLabel->setWordWrap(true);
        layout->addWidget(explLabel);
    }

    // Section 3: Benefits & Risks/Limitations
    auto* tradeOffs = new QHBoxLayout();
    tradeOffs->setSpacing(16);

    if (!m_rec.benefits.empty()) {
        auto* benLabel = new QLabel(
            QString("<span style='color:%1; font-weight:600;'>✓ Benefit:</span> %2")
                .arg(Theme::AccentGreen)
                .arg(QString::fromStdString(m_rec.benefits)), this);
        benLabel->setStyleSheet(QString("font-size: 11px; color: %1;").arg(Theme::TextSecondary));
        benLabel->setWordWrap(true);
        tradeOffs->addWidget(benLabel, 1);
    }

    if (!m_rec.risksAndLimitations.empty()) {
        auto* riskLabel = new QLabel(
            QString("<span style='color:%1; font-weight:600;'>⚠ Trade-off:</span> %2")
                .arg(Theme::AccentAmber)
                .arg(QString::fromStdString(m_rec.risksAndLimitations)), this);
        riskLabel->setStyleSheet(QString("font-size: 11px; color: %1;").arg(Theme::TextSecondary));
        riskLabel->setWordWrap(true);
        tradeOffs->addWidget(riskLabel, 1);
    }

    layout->addLayout(tradeOffs);
}

} // namespace pulse::ui

