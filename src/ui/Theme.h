#pragma once

#include <QString>
#include <QColor>
#include <QFont>

namespace pulse::ui {

class Theme {
public:
    // Palette constants
    static constexpr const char* BgDark        = "#0e1117";
    static constexpr const char* BgSidebar     = "#131722";
    static constexpr const char* BgSurface     = "#181d2a";
    static constexpr const char* BgSurfaceAlt  = "#202637";
    static constexpr const char* BgHover       = "#283046";
    static constexpr const char* BorderSubtle  = "#2a3245";
    static constexpr const char* BorderFocus   = "#38bdf8";

    static constexpr const char* TextPrimary   = "#f1f5f9";
    static constexpr const char* TextSecondary = "#94a3b8";
    static constexpr const char* TextMuted     = "#64748b";

    static constexpr const char* AccentBlue    = "#38bdf8";
    static constexpr const char* AccentGreen   = "#34d399";
    static constexpr const char* AccentAmber   = "#fbbf24";
    static constexpr const char* AccentRed     = "#f87171";
    static constexpr const char* AccentPurple  = "#a78bfa";

    // Standard QColors
    static QColor colorBgDark() { return QColor(BgDark); }
    static QColor colorBgSurface() { return QColor(BgSurface); }
    static QColor colorBorder() { return QColor(BorderSubtle); }
    static QColor colorAccentBlue() { return QColor(AccentBlue); }
    static QColor colorAccentGreen() { return QColor(AccentGreen); }
    static QColor colorAccentAmber() { return QColor(AccentAmber); }
    static QColor colorAccentRed() { return QColor(AccentRed); }
    static QColor colorTextPrimary() { return QColor(TextPrimary); }
    static QColor colorTextSecondary() { return QColor(TextSecondary); }

    // Global application stylesheet
    static QString getApplicationStyleSheet();

    // Metric color selector based on percentage
    static QColor getUsageColor(double percent);
    static QString getUsageColorHex(double percent);
};

} // namespace pulse::ui

