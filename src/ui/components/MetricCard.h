#pragma once

#include <QFrame>
#include <QLabel>
#include <QProgressBar>

namespace pulse::ui {

class MetricCard : public QFrame {
    Q_OBJECT

public:
    explicit MetricCard(const QString& title, const QString& unit = "", QWidget* parent = nullptr);
    ~MetricCard() override = default;

    void setValue(double value, int precision = 1);
    void setValueString(const QString& text);
    void setSubtext(const QString& text);
    void setAccentColor(const QColor& color);
    void setShowProgressBar(bool show);
    void setProgress(int percentage);

private:
    QString m_unit;
    QLabel* m_titleLabel{nullptr};
    QLabel* m_valueLabel{nullptr};
    QLabel* m_subtextLabel{nullptr};
    QProgressBar* m_progressBar{nullptr};
};

} // namespace pulse::ui

