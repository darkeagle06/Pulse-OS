#include "Theme.h"

namespace pulse::ui {

QString Theme::getApplicationStyleSheet() {
    return QString(R"(
        /* Global Base Canvas */
        QWidget {
            background-color: %1;
            color: %2;
            font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, "Roboto", sans-serif;
            font-size: 13px;
            selection-background-color: %5;
            selection-color: %2;
        }

        QMainWindow {
            background-color: %1;
        }

        /* Sidebar Styling */
        #sidebarWidget {
            background-color: %3;
            border-right: 1px solid %4;
        }

        /* Sidebar Navigation Items */
        QPushButton.sidebarBtn {
            background-color: transparent;
            color: %6;
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 9px 14px;
            text-align: left;
            font-size: 13px;
            font-weight: 500;
        }

        QPushButton.sidebarBtn:hover {
            background-color: %5;
            color: %2;
            border-color: %4;
        }

        QPushButton.sidebarBtn:checked {
            background-color: %7;
            color: %8;
            font-weight: 600;
            border: 1px solid %4;
            border-left: 3px solid %8;
        }

        /* Standard Buttons */
        QPushButton.actionBtn {
            background-color: %7;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 6px 14px;
            font-size: 12px;
            font-weight: 500;
        }

        QPushButton.actionBtn:hover {
            background-color: %5;
            border-color: %9;
            color: %2;
        }

        QPushButton.actionBtn:pressed {
            background-color: %3;
        }

        QPushButton.primaryBtn {
            background-color: #0284c7;
            color: #ffffff;
            border: 1px solid #0369a1;
            border-radius: 6px;
            padding: 6px 16px;
            font-size: 12px;
            font-weight: 600;
        }

        QPushButton.primaryBtn:hover {
            background-color: #0ea5e9;
            border-color: #38bdf8;
        }

        QPushButton.primaryBtn:pressed {
            background-color: #0369a1;
        }

        QPushButton.dangerBtn {
            background-color: #9f1239;
            color: #ffffff;
            border: 1px solid #881337;
            border-radius: 6px;
            padding: 6px 16px;
            font-size: 12px;
            font-weight: 600;
        }

        QPushButton.dangerBtn:hover {
            background-color: #be123c;
        }

        /* Tables & Process Explorer */
        QTableWidget, QTableView {
            background-color: %10;
            border: 1px solid %4;
            border-radius: 6px;
            gridline-color: %4;
            alternate-background-color: %1;
            color: %2;
            outline: none;
        }

        QTableWidget::item, QTableView::item {
            padding: 5px 8px;
            border-bottom: 1px solid %4;
        }

        QTableWidget::item:selected, QTableView::item:selected {
            background-color: %5;
            color: %8;
        }

        QTableWidget::item:hover, QTableView::item:hover {
            background-color: %7;
        }

        QHeaderView::section {
            background-color: %3;
            color: %6;
            padding: 7px 10px;
            border: none;
            border-bottom: 1px solid %4;
            border-right: 1px solid %4;
            font-weight: 600;
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        /* Inputs and Search Filters */
        QLineEdit {
            background-color: %10;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 6px 12px;
            color: %2;
            font-size: 12px;
        }

        QLineEdit:hover {
            border-color: %9;
        }

        QLineEdit:focus {
            border: 1px solid %8;
            background-color: %7;
        }

        /* ComboBox */
        QComboBox {
            background-color: %7;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 5px 12px;
            color: %2;
            font-size: 12px;
            min-height: 20px;
        }

        QComboBox:hover {
            border-color: %9;
            background-color: %5;
        }

        QComboBox:focus {
            border-color: %8;
        }

        QComboBox::drop-down {
            border: none;
            padding-right: 8px;
            subcontrol-origin: padding;
            subcontrol-position: top right;
        }

        QComboBox QAbstractItemView {
            background-color: %7;
            border: 1px solid %9;
            border-radius: 6px;
            selection-background-color: %5;
            selection-color: %8;
            padding: 4px;
        }

        /* Checkboxes */
        QCheckBox {
            color: %2;
            font-size: 12px;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border-radius: 4px;
            border: 1px solid %9;
            background-color: %10;
        }

        QCheckBox::indicator:hover {
            border-color: %8;
        }

        QCheckBox::indicator:checked {
            background-color: %8;
            border-color: %8;
            image: none;
        }

        /* Sliders */
        QSlider::groove:horizontal {
            height: 4px;
            background: %7;
            border-radius: 2px;
        }

        QSlider::sub-page:horizontal {
            background: %8;
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            background: %2;
            border: 2px solid %8;
            width: 14px;
            margin-top: -5px;
            margin-bottom: -5px;
            border-radius: 7px;
        }

        QSlider::handle:horizontal:hover {
            background: %8;
            border-color: %2;
        }

        /* Progress Bars */
        QProgressBar {
            background-color: %7;
            border: none;
            border-radius: 3px;
            text-align: center;
        }

        QProgressBar::chunk {
            background-color: %8;
            border-radius: 3px;
        }

        /* Scrollbars (Ultra-clean subtle rail) */
        QScrollBar:vertical {
            background: transparent;
            width: 6px;
            margin: 0;
            border-radius: 3px;
        }

        QScrollBar::handle:vertical {
            background: %4;
            min-height: 24px;
            border-radius: 3px;
        }

        QScrollBar::handle:vertical:hover {
            background: %9;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
            height: 0px;
        }

        QScrollBar:horizontal {
            background: transparent;
            height: 6px;
            margin: 0;
            border-radius: 3px;
        }

        QScrollBar::handle:horizontal {
            background: %4;
            min-width: 24px;
            border-radius: 3px;
        }

        QScrollBar::handle:horizontal:hover {
            background: %9;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: none;
            width: 0px;
        }

        /* Group Box */
        QGroupBox {
            background-color: %10;
            border: 1px solid %4;
            border-radius: 8px;
            margin-top: 18px;
            font-weight: 600;
            padding: 16px 14px 14px 14px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0px 8px;
            color: %8;
        }
    )")
    .arg(BgWindow)       // %1
    .arg(TextPrimary)    // %2
    .arg(BgSidebar)      // %3
    .arg(BorderSubtle)   // %4
    .arg(BgHover)        // %5
    .arg(TextSecondary)  // %6
    .arg(BgSurfaceAlt)   // %7
    .arg(AccentCyan)     // %8
    .arg(BorderStrong)   // %9
    .arg(BgSurface);     // %10
}

QString Theme::panelStyleSheet(int radius) {
    return QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: %3px; }"
    ).arg(BgSurface).arg(BorderSubtle).arg(radius);
}

QString Theme::bannerStyleSheet(const QString& borderAccent) {
    if (!borderAccent.isEmpty()) {
        return QString(
            "QFrame { background-color: %1; border: 1px solid %2; border-left: 4px solid %3; border-radius: 8px; padding: 12px 16px; }"
        ).arg(BgSurface).arg(BorderSubtle).arg(borderAccent);
    }
    return QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 12px 16px; }"
    ).arg(BgSurface).arg(BorderSubtle);
}

QString Theme::pillStyleSheet(const QString& bgHex, const QString& fgHex) {
    return QString(
        "background-color: %1; color: %2; border: 1px solid %3; border-radius: 11px; padding: 3px 10px; font-weight: 600; font-size: 11px;"
    ).arg(bgHex).arg(fgHex).arg(BorderSubtle);
}

QString Theme::sectionTitleStyleSheet() {
    return QString("font-size: 14px; font-weight: 600; color: %1;").arg(TextPrimary);
}

QString Theme::pageTitleStyleSheet() {
    return QString("font-size: 18px; font-weight: 700; color: %1; letter-spacing: -0.2px;").arg(TextPrimary);
}

QColor Theme::getUsageColor(double percent) {
    if (percent >= 85.0) return QColor(AccentRose);
    if (percent >= 65.0) return QColor(AccentAmber);
    return QColor(AccentGreen);
}

QString Theme::getUsageColorHex(double percent) {
    if (percent >= 85.0) return QString(AccentRose);
    if (percent >= 65.0) return QString(AccentAmber);
    return QString(AccentGreen);
}

} // namespace pulse::ui
