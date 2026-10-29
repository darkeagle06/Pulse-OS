#pragma once

#include <QString>
#include <QColor>
#include <QFont>

namespace pulse::ui {

class Theme {
public:
    // Core Deep Graphite Palette
    static constexpr const char* BgWindow       = "#111318"; // Main window canvas (deep graphite)
    static constexpr const char* BgDark         = "#111318"; // Backward compatibility alias
    static constexpr const char* BgSidebar      = "#151821"; // Sidebar container
    static constexpr const char* BgSurface      = "#1a1e29"; // Primary card & panel surface
    static constexpr const char* BgSurfaceAlt   = "#212635"; // Elevated sub-panels, headers
    static constexpr const char* BgHover        = "#293042"; // Interactive hover background
    static constexpr const char* BgActive       = "#2c3347"; // Active / selected item background
    static constexpr const char* BorderSubtle   = "#272e3f"; // Hairline panel borders
    static constexpr const char* BorderStrong   = "#384259"; // Emphasized borders
    static constexpr const char* BorderFocus    = "#38bdf8"; // Focus ring color

    // Text & Content Hierarchy
    static constexpr const char* TextPrimary    = "#f1f5f9"; // High-contrast primary reading text
    static constexpr const char* TextSecondary  = "#94a3b8"; // Muted labels, secondary descriptions
    static constexpr const char* TextMuted      = "#64748b"; // Subtext, timestamps, footnotes

    // Restrained Technical Accents
    static constexpr const char* AccentCyan     = "#38bdf8"; // Primary active accent / live data
    static constexpr const char* AccentBlue     = "#38bdf8"; // Alias for AccentCyan
    static constexpr const char* AccentGreen    = "#22c55e"; // Healthy / nominal / optimal
    static constexpr const char* AccentAmber    = "#f59e0b"; // Warning / moderate load
    static constexpr const char* AccentRose     = "#f43f5e"; // High / critical / alert
    static constexpr const char* AccentRed      = "#f43f5e"; // Alias for AccentRose
    static constexpr const char* AccentPurple   = "#a855f7"; // Workload profile branding

    // Standard QColors
    static QColor colorBgWindow()       { return QColor(BgWindow); }
    static QColor colorBgDark()         { return QColor(BgDark); }
    static QColor colorBgSidebar()      { return QColor(BgSidebar); }
    static QColor colorBgSurface()      { return QColor(BgSurface); }
    static QColor colorBgSurfaceAlt()   { return QColor(BgSurfaceAlt); }
    static QColor colorBorder()         { return QColor(BorderSubtle); }
    static QColor colorBorderStrong()   { return QColor(BorderStrong); }
    static QColor colorAccentCyan()     { return QColor(AccentCyan); }
    static QColor colorAccentBlue()     { return QColor(AccentBlue); }
    static QColor colorAccentGreen()    { return QColor(AccentGreen); }
    static QColor colorAccentAmber()    { return QColor(AccentAmber); }
    static QColor colorAccentRose()     { return QColor(AccentRose); }
    static QColor colorAccentRed()      { return QColor(AccentRed); }
    static QColor colorAccentPurple()   { return QColor(AccentPurple); }
    static QColor colorTextPrimary()    { return QColor(TextPrimary); }
    static QColor colorTextSecondary()  { return QColor(TextSecondary); }
    static QColor colorTextMuted()      { return QColor(TextMuted); }

    // Global application stylesheet
    static QString getApplicationStyleSheet();

    // Standardized UI component style generators
    static QString panelStyleSheet(int radius = 8);
    static QString bannerStyleSheet(const QString& borderAccent = QString());
    static QString pillStyleSheet(const QString& bgHex, const QString& fgHex);
    static QString sectionTitleStyleSheet();
    static QString pageTitleStyleSheet();

    // Metric color selector based on percentage
    static QColor getUsageColor(double percent);
    static QString getUsageColorHex(double percent);
};

} // namespace pulse::ui
