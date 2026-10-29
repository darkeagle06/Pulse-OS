#include "PerformanceChart.h"
#include "../Theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <algorithm>

namespace pulse::ui {

PerformanceChart::PerformanceChart(const QString& title, const QColor& lineColor, QWidget* parent)
    : QWidget(parent), m_title(title), m_lineColor(lineColor) {
    setMinimumHeight(140);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void PerformanceChart::updateData(const std::vector<double>& history, double currentVal) {
    m_data = history;
    m_currentVal = currentVal;
    update();
}

void PerformanceChart::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    const int topPadding = 30;
    const int bottomPadding = 20;
    const int leftPadding = 36;
    const int rightPadding = 16;

    const int plotW = w - leftPadding - rightPadding;
    const int plotH = h - topPadding - bottomPadding;

    if (plotW <= 10 || plotH <= 10) return;

    // Background card frame
    painter.setPen(QPen(QColor(Theme::BorderSubtle), 1));
    painter.setBrush(QColor(Theme::BgSurface));
    painter.drawRoundedRect(0, 0, w - 1, h - 1, 8, 8);

    // Header Title
    painter.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
    painter.setPen(QColor(Theme::TextSecondary));
    painter.drawText(16, 20, m_title);

    // Current Value badge in top right
    QString valStr = QString::number(m_currentVal, 'f', 1) + m_unit;
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(m_lineColor);
    painter.drawText(w - rightPadding - 80, 20, 80, 20, Qt::AlignRight, valStr);

    // Grid lines (0%, 25%, 50%, 75%, 100%)
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(QPen(QColor(Theme::BorderSubtle), 1, Qt::DotLine));

    const int steps = 4;
    for (int i = 0; i <= steps; ++i) {
        int y = topPadding + (plotH * (steps - i)) / steps;
        painter.drawLine(leftPadding, y, w - rightPadding, y);

        // Y-axis label
        painter.setPen(QColor(Theme::TextMuted));
        int labelVal = static_cast<int>((m_maxScale * i) / steps);
        painter.drawText(4, y - 6, leftPadding - 8, 14, Qt::AlignRight, QString::number(labelVal));
        painter.setPen(QPen(QColor(Theme::BorderSubtle), 1, Qt::DotLine));
    }

    if (m_data.empty()) return;

    // Build chart polyline and filled gradient area
    size_t count = m_data.size();
    if (count < 2) return;

    QPolygonF polyline;
    QPainterPath areaPath;

    double xStep = static_cast<double>(plotW) / static_cast<double>(count - 1);
    for (size_t i = 0; i < count; ++i) {
        double val = std::clamp(m_data[i], 0.0, m_maxScale);
        double normalized = val / m_maxScale;
        double x = leftPadding + i * xStep;
        double y = topPadding + plotH - (normalized * plotH);

        polyline << QPointF(x, y);
        if (i == 0) {
            areaPath.moveTo(x, topPadding + plotH);
            areaPath.lineTo(x, y);
        } else {
            areaPath.lineTo(x, y);
        }
    }

    areaPath.lineTo(leftPadding + (count - 1) * xStep, topPadding + plotH);
    areaPath.closeSubpath();

    // Fill gradient
    QLinearGradient grad(0, topPadding, 0, topPadding + plotH);
    QColor startGrad = m_lineColor;
    startGrad.setAlpha(60);
    QColor endGrad = m_lineColor;
    endGrad.setAlpha(5);
    grad.setColorAt(0.0, startGrad);
    grad.setColorAt(1.0, endGrad);

    painter.fillPath(areaPath, grad);

    // Draw main plot line
    QPen linePen(m_lineColor, 2.0);
    linePen.setCapStyle(Qt::RoundCap);
    linePen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(linePen);
    painter.drawPolyline(polyline);

    // Draw pulsating indicator on latest point
    if (!polyline.isEmpty()) {
        QPointF lastPt = polyline.last();
        painter.setBrush(m_lineColor);
        painter.setPen(QPen(Qt::white, 1.5));
        painter.drawEllipse(lastPt, 3.5, 3.5);
    }
}

} // namespace pulse::ui

