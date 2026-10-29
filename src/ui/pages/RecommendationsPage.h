#pragma once

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <vector>

#include "../../core/Types.h"
#include "../components/RecommendationCard.h"

namespace pulse::ui {

class RecommendationsPage : public QWidget {
    Q_OBJECT

public:
    explicit RecommendationsPage(QWidget* parent = nullptr);
    ~RecommendationsPage() override = default;

    void updateRecommendations(const std::vector<Recommendation>& recs);

signals:
    void actionTriggered(const Recommendation& rec);

private slots:
    void onFilterChanged(int index);

private:
    void setupUi();
    void renderCards();

    QComboBox* m_filterCombo{nullptr};
    QLabel* m_countLabel{nullptr};
    QScrollArea* m_scrollArea{nullptr};
    QWidget* m_cardsContainer{nullptr};
    QVBoxLayout* m_cardsLayout{nullptr};
    QLabel* m_emptyLabel{nullptr};

    std::vector<Recommendation> m_allRecs;
    int m_filterIndex{0}; // 0 = All, 1 = Warnings & Critical, 2 = Actionable
};

} // namespace pulse::ui

