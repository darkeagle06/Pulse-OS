#include "MetricCard.h"
#include "../Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

namespace pulse::ui {

MetricCard::MetricCard(const QString& title, const QString& unit, QWidget* parent)
    : QFrame(parent), m_unit(unit) {
    setObjectName("metricCard");
    setStyleSheet(QString(
        "#metricCard {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "  padding: 12px 14px;"
        "}"
    ).arg(Theme::BgSurface).arg(Theme::BorderSubtle));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    // Title label
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setStyleSheet(QString(
        "font-size: 12px; font-weight: 600; color: %1; text-transform: uppercase; letter-spacing: 0.5px;"
    ).arg(Theme::TextSecondary));
    layout->addWidget(m_titleLabel);

    // Value label
    m_valueLabel = new QLabel("--" + m_unit, this);
    m_valueLabel->setStyleSheet(QString(
        "font-size: 26px; font-weight: 700; color: %1;"
    ).arg(Theme::TextPrimary));
    layout->addWidget(m_valueLabel);

    // Optional progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setStyleSheet(QString(
        "QProgressBar {"
        "  background-color: %1;"
        "  border: none;"
        "  border-radius: 3px;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: %2;"
        "  border-radius: 3px;"
        "}"
    ).arg(Theme::BgSurfaceAlt).arg(Theme::AccentBlue));
    m_progressBar->hide();
    layout->addWidget(m_progressBar);

    // Subtext label
    m_subtextLabel = new QLabel("", this);
    m_subtextLabel->setStyleSheet(QString(
        "font-size: 11px; color: %1;"
    ).arg(Theme::TextMuted));
    layout->addWidget(m_subtextLabel);
}

void MetricCard::setValue(double value, int precision) {
    m_valueLabel->setText(QString::number(value, 'f', precision) + m_unit);
}

void MetricCard::setValueString(const QString& text) {
    m_valueLabel->setText(text);
}

void MetricCard::setSubtext(const QString& text) {
    m_subtextLabel->setText(text);
}

void MetricCard::setAccentColor(const QColor& color) {
    m_valueLabel->setStyleSheet(QString(
        "font-size: 26px; font-weight: 700; color: %1;"
    ).arg(color.name()));

    m_progressBar->setStyleSheet(QString(
        "QProgressBar {"
        "  background-color: %1;"
        "  border: none;"
        "  border-radius: 3px;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: %2;"
        "  border-radius: 3px;"
        "}"
    ).arg(Theme::BgSurfaceAlt).arg(color.name()));
}

void MetricCard::setShowProgressBar(bool show) {
    m_progressBar->setVisible(show);
}

void MetricCard::setProgress(int percentage) {
    m_progressBar->setValue(percentage);
}

} // namespace pulse::ui

