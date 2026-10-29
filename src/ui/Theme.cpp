#include "Theme.h"

namespace pulse::ui {

QString Theme::getApplicationStyleSheet() {
    return QString(R"(
        /* Global Window and Base Styling */
        QWidget {
            background-color: %1;
            color: %2;
            font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, "Roboto", sans-serif;
            font-size: 13px;
            selection-background-color: %3;
            selection-color: #0e1117;
        }

        /* Main Window */
        QMainWindow {
            background-color: %1;
        }

        /* Sidebar Styling */
        #sidebarWidget {
            background-color: %4;
            border-right: 1px solid %5;
        }

        /* Sidebar Navigation Buttons */
        QPushButton.sidebarBtn {
            background-color: transparent;
            color: %6;
            border: none;
            border-radius: 6px;
            padding: 10px 14px;
            text-align: left;
            font-size: 13px;
            font-weight: 500;
        }

        QPushButton.sidebarBtn:hover {
            background-color: %7;
            color: %2;
        }

        QPushButton.sidebarBtn:checked {
            background-color: %8;
            color: %3;
            font-weight: 600;
            border-left: 3px solid %3;
            border-top-left-radius: 0px;
            border-bottom-left-radius: 0px;
        }

        /* Standard Buttons */
        QPushButton.actionBtn {
            background-color: %8;
            color: %2;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 7px 15px;
            font-weight: 500;
        }

        QPushButton.actionBtn:hover {
            background-color: %7;
            border-color: %3;
        }

        QPushButton.primaryBtn {
            background-color: #0284c7;
            color: #ffffff;
            border: 1px solid #0369a1;
            border-radius: 6px;
            padding: 7px 16px;
            font-weight: 600;
        }

        QPushButton.primaryBtn:hover {
            background-color: #0ea5e9;
        }

        QPushButton.dangerBtn {
            background-color: #991b1b;
            color: #ffffff;
            border: 1px solid #7f1d1d;
            border-radius: 6px;
            padding: 7px 16px;
            font-weight: 600;
        }

        QPushButton.dangerBtn:hover {
            background-color: #b91c1c;
        }

        /* Tables & Process Explorer */
        QTableWidget, QTableView {
            background-color: %8;
            border: 1px solid %5;
            border-radius: 6px;
            gridline-color: %5;
            alternate-background-color: #1a202e;
            color: %2;
        }

        QTableWidget::item, QTableView::item {
            padding: 6px 10px;
            border-bottom: 1px solid #1c2231;
        }

        QTableWidget::item:selected, QTableView::item:selected {
            background-color: %7;
            color: %3;
        }

        QHeaderView::section {
            background-color: %4;
            color: %6;
            padding: 8px 10px;
            border: none;
            border-bottom: 1px solid %5;
            border-right: 1px solid %5;
            font-weight: 600;
            font-size: 12px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        /* Inputs and Search Filters */
        QLineEdit {
            background-color: %8;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 6px 12px;
            color: %2;
        }

        QLineEdit:focus {
            border: 1px solid %3;
        }

        /* ComboBox */
        QComboBox {
            background-color: %8;
            border: 1px solid %5;
            border-radius: 6px;
            padding: 6px 12px;
            color: %2;
        }

        QComboBox:hover {
            border-color: %3;
        }

        QComboBox::drop-down {
            border: none;
            padding-right: 8px;
        }

        QComboBox QAbstractItemView {
            background-color: %8;
            border: 1px solid %5;
            selection-background-color: %7;
            selection-color: %3;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            background: %1;
            width: 8px;
            margin: 0;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical {
            background: %5;
            min-height: 24px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background: %6;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
            height: 0px;
        }

        QScrollBar:horizontal {
            background: %1;
            height: 8px;
            margin: 0;
            border-radius: 4px;
        }

        QScrollBar::handle:horizontal {
            background: %5;
            min-width: 24px;
            border-radius: 4px;
        }

        /* Group Box & Frames */
        QGroupBox {
            background-color: %8;
            border: 1px solid %5;
            border-radius: 8px;
            margin-top: 18px;
            font-weight: 600;
            padding-top: 15px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 2px 8px;
            color: %3;
        }

        /* Tab Widget */
        QTabWidget::pane {
            border: 1px solid %5;
            border-radius: 6px;
            background-color: %8;
        }

        QTabBar::tab {
            background-color: %4;
            color: %6;
            padding: 8px 16px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            margin-right: 2px;
        }

        QTabBar::tab:selected {
            background-color: %8;
            color: %3;
            font-weight: 600;
        }
    )")
    .arg(BgDark)        // %1
    .arg(TextPrimary)   // %2
    .arg(AccentBlue)    // %3
    .arg(BgSidebar)     // %4
    .arg(BorderSubtle)  // %5
    .arg(TextSecondary) // %6
    .arg(BgHover)       // %7
    .arg(BgSurface);    // %8
}

QColor Theme::getUsageColor(double percent) {
    if (percent >= 85.0) return QColor(AccentRed);
    if (percent >= 65.0) return QColor(AccentAmber);
    return QColor(AccentGreen);
}

QString Theme::getUsageColorHex(double percent) {
    if (percent >= 85.0) return QString(AccentRed);
    if (percent >= 65.0) return QString(AccentAmber);
    return QString(AccentGreen);
}

} // namespace pulse::ui

