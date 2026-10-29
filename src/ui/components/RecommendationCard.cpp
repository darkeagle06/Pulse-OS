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
    setObjectName("recCard");
    setStyleSheet(QString(
        "#recCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "  padding: 14px 16px;"
        "}"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    // Header layout: Level Badge + Title + Action Button
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    // Level Badge
    auto* badge = new QLabel(this);
    QString badgeText;
    QString badgeBg;
    QString badgeFg;

    switch (m_rec.level) {
        case RecommendationLevel::Critical:
            badgeText = "CRITICAL";
            badgeBg = "#7f1d1d";
            badgeFg = "#fca5a5";
            break;
        case RecommendationLevel::Warning:
            badgeText = "WARNING";
            badgeBg = "#78350f";
            badgeFg = "#fde68a";
            break;
        case RecommendationLevel::Suggestion:
            badgeText = "SUGGESTION";
            badgeBg = "#1e3a5f";
            badgeFg = "#bae6fd";
            break;
        case RecommendationLevel::Info:
        default:
            badgeText = "INFO";
            badgeBg = "#1f2937";
            badgeFg = "#9ca3af";
            break;
    }

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

    // Section 1: Objective Observation vs Diagnosis Box
    auto* obsFrame = new QFrame(this);
    obsFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border-radius: 6px; padding: 10px; }"
    ).arg(Theme::BgSurfaceAlt));

    auto* obsLayout = new QVBoxLayout(obsFrame);
    obsLayout->setContentsMargins(0, 0, 0, 0);
    obsLayout->setSpacing(6);

    auto* obsLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Observation:</span> %2")
            .arg(Theme::AccentBlue)
            .arg(QString::fromStdString(m_rec.observation)), obsFrame);
    obsLabel->setWordWrap(true);
    obsLayout->addWidget(obsLabel);

    auto* diagLabel = new QLabel(
        QString("<span style='color:%1; font-weight:600;'>Diagnosis:</span> %2")
            .arg(Theme::AccentAmber)
            .arg(QString::fromStdString(m_rec.diagnosis)), obsFrame);
    diagLabel->setWordWrap(true);
    obsLayout->addWidget(diagLabel);

    layout->addWidget(obsFrame);

    // Section 2: Recommended Action & Explanation
    auto* actionLabel = new QLabel(
        QString("<span style='font-weight:600; color:%1;'>Recommended Action:</span> %2")
            .arg(Theme::TextPrimary)
            .arg(QString::fromStdString(m_rec.recommendedAction)), this);
    actionLabel->setWordWrap(true);
    layout->addWidget(actionLabel);

    if (!m_rec.explanation.empty()) {
        auto* explLabel = new QLabel(
            QString("<span style='font-size:11px; color:%1;'>%2</span>")
                .arg(Theme::TextMuted)
                .arg(QString::fromStdString(m_rec.explanation)), this);
        explLabel->setWordWrap(true);
        layout->addWidget(explLabel);
    }

    // Section 3: Benefits & Risks/Limitations
    auto* tradeOffs = new QHBoxLayout();
    tradeOffs->setSpacing(16);

    if (!m_rec.benefits.empty()) {
        auto* benLabel = new QLabel(
            QString("<span style='color:%1; font-weight:600;'>Benefits:</span> %2")
                .arg(Theme::AccentGreen)
                .arg(QString::fromStdString(m_rec.benefits)), this);
        benLabel->setStyleSheet("font-size: 11px;");
        benLabel->setWordWrap(true);
        tradeOffs->addWidget(benLabel, 1);
    }

    if (!m_rec.risksAndLimitations.empty()) {
        auto* riskLabel = new QLabel(
            QString("<span style='color:%1; font-weight:600;'>Risks/Limitations:</span> %2")
                .arg(Theme::AccentAmber)
                .arg(QString::fromStdString(m_rec.risksAndLimitations)), this);
        riskLabel->setStyleSheet("font-size: 11px;");
        riskLabel->setWordWrap(true);
        tradeOffs->addWidget(riskLabel, 1);
    }

    layout->addLayout(tradeOffs);
}

} // namespace pulse::ui

