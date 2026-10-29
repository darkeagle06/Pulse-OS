#pragma once

#include <QFrame>
#include <QLabel>
#include <QPushButton>

#include "../../core/Types.h"

namespace pulse::ui {

class RecommendationCard : public QFrame {
    Q_OBJECT

public:
    explicit RecommendationCard(const Recommendation& rec, QWidget* parent = nullptr);
    ~RecommendationCard() override = default;

    [[nodiscard]] const Recommendation& getRecommendation() const { return m_rec; }

signals:
    void actionRequested(const Recommendation& rec);

private:
    void setupUi();

    Recommendation m_rec;
    QPushButton* m_actionBtn{nullptr};
};

} // namespace pulse::ui

