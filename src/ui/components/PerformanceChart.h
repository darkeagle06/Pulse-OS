#pragma once

#include <QWidget>
#include <vector>
#include <QString>
#include <QColor>

namespace pulse::ui {

class PerformanceChart : public QWidget {
    Q_OBJECT

public:
    explicit PerformanceChart(const QString& title, const QColor& lineColor, QWidget* parent = nullptr);
    ~PerformanceChart() override = default;

    void updateData(const std::vector<double>& history, double currentVal);
    void setUnit(const QString& unit) { m_unit = unit; }
    void setMaxScale(double maxVal) { m_maxScale = maxVal; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_title;
    QString m_unit{"%"};
    QColor m_lineColor;
    std::vector<double> m_data;
    double m_currentVal{0.0};
    double m_maxScale{100.0};
};

} // namespace pulse::ui

