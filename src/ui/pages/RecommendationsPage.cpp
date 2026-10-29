#include "RecommendationsPage.h"
#include "../Theme.h"

#include <QHBoxLayout>
#include <QFrame>

namespace pulse::ui {

RecommendationsPage::RecommendationsPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void RecommendationsPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    // Top Controls Bar
    auto* topLayout = new QHBoxLayout();
    topLayout->setSpacing(10);

    auto* titleLabel = new QLabel("System Performance Recommendations", this);
    titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: 700; color: %1;").arg(Theme::TextPrimary));
    topLayout->addWidget(titleLabel);

    topLayout->addStretch();

    auto* filterLabel = new QLabel("Filter:", this);
    filterLabel->setStyleSheet(QString("color: %1; font-weight: 500;").arg(Theme::TextSecondary));
    topLayout->addWidget(filterLabel);

    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem("All Insights & Alerts", 0);
    m_filterCombo->addItem("Warnings & Critical Only", 1);
    m_filterCombo->addItem("Actionable Optimizations", 2);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RecommendationsPage::onFilterChanged);
    topLayout->addWidget(m_filterCombo);

    m_countLabel = new QLabel("(0 active)", this);
    m_countLabel->setStyleSheet(QString("color: %1;").arg(Theme::TextMuted));
    topLayout->addWidget(m_countLabel);

    mainLayout->addLayout(topLayout);

    // Explanatory Banner
    auto* noticeFrame = new QFrame(this);
    noticeFrame->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; padding: 8px 12px; }"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));
    auto* noticeLayout = new QHBoxLayout(noticeFrame);
    noticeLayout->setContentsMargins(0, 0, 0, 0);

    auto* noticeText = new QLabel("Rule-based analysis clearly distinguishes <b>measured observations</b> from <b>diagnostic hypotheses</b>. Actions require explicit confirmation in the Safe Action Center.", noticeFrame);
    noticeText->setStyleSheet(QString("color: %1; font-size: 12px;").arg(Theme::TextSecondary));
    noticeLayout->addWidget(noticeText);
    mainLayout->addWidget(noticeFrame);

    // Scrollable cards container
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("background: transparent;");

    m_cardsContainer = new QWidget(m_scrollArea);
    m_cardsLayout = new QVBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(10);
    m_cardsLayout->setAlignment(Qt::AlignTop);

    m_emptyLabel = new QLabel("No active recommendations for current metrics. System running within target parameters.", m_cardsContainer);
    m_emptyLabel->setStyleSheet(QString("font-size: 13px; color: %1; padding: 40px;").arg(Theme::TextMuted));
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_cardsLayout->addWidget(m_emptyLabel);

    m_scrollArea->setWidget(m_cardsContainer);
    mainLayout->addWidget(m_scrollArea, 1);
}

void RecommendationsPage::updateRecommendations(const std::vector<Recommendation>& recs) {
    m_allRecs = recs;
    m_countLabel->setText(QString("(%1 active)").arg(recs.size()));
    renderCards();
}

void RecommendationsPage::onFilterChanged(int index) {
    m_filterIndex = index;
    renderCards();
}

void RecommendationsPage::renderCards() {
    // Clear existing cards
    QLayoutItem* item;
    while ((item = m_cardsLayout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != m_emptyLabel) {
            delete item->widget();
        }
        delete item;
    }

    std::vector<Recommendation> filtered;
    for (const auto& r : m_allRecs) {
        if (m_filterIndex == 1) { // Warnings & Critical only
            if (r.level != RecommendationLevel::Warning && r.level != RecommendationLevel::Critical) {
                continue;
            }
        } else if (m_filterIndex == 2) { // Actionable only
            if (!r.isActionable) continue;
        }
        filtered.push_back(r);
    }

    if (filtered.empty()) {
        m_cardsLayout->addWidget(m_emptyLabel);
        m_emptyLabel->show();
    } else {
        m_emptyLabel->hide();
        for (const auto& r : filtered) {
            auto* card = new RecommendationCard(r, m_cardsContainer);
            connect(card, &RecommendationCard::actionRequested, this, &RecommendationsPage::actionTriggered);
            m_cardsLayout->addWidget(card);
        }
    }
}

} // namespace pulse::ui

