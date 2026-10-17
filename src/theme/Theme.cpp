/*
 * File: Theme.cpp
 * Purpose: Implements the default Dark, Light, and HighContrast themes and fluent configuration.
 */

#include "ui/theme/Theme.hpp"
#include <stdexcept>

namespace cppui {
namespace theme {

Theme::Theme() : m_mode(ThemeMode::Dark) {
    // 預設構造即採用 Dark 主題代幣
}

Theme Theme::Dark() {
    Theme theme;
    theme.m_mode = ThemeMode::Dark;

    // 暗色表面與視窗
    theme.m_colors.windowBackground  = core::Color(24, 26, 32, 255);
    theme.m_colors.surfaceBackground = core::Color(32, 36, 46, 255);
    theme.m_colors.surfaceBorder     = core::Color(50, 56, 70, 255);
    theme.m_colors.cardBackground    = core::Color(40, 45, 58, 255);

    // 控制項狀態色彩
    theme.m_colors.controlBackground          = core::Color(45, 52, 70, 255);
    theme.m_colors.controlHoverBackground     = core::Color(60, 75, 105, 255);
    theme.m_colors.controlPressedBackground   = core::Color(32, 40, 58, 255);
    theme.m_colors.controlDisabledBackground  = core::Color(35, 38, 45, 255);
    theme.m_colors.controlBorder              = core::Color(70, 80, 100, 255);
    theme.m_colors.controlHoverBorder         = core::Color(95, 110, 135, 255);
    theme.m_colors.controlFocusedBorder       = core::Color(0, 160, 255, 255);
    theme.m_colors.controlDisabledBorder      = core::Color(50, 55, 65, 255);

    // 品牌與突顯色彩
    theme.m_colors.accentPrimary  = core::Color(0, 122, 255, 255);
    theme.m_colors.accentHover    = core::Color(20, 140, 255, 255);
    theme.m_colors.accentPressed  = core::Color(0, 100, 220, 255);
    theme.m_colors.accentDisabled = core::Color(0, 122, 255, 100);
    theme.m_colors.accentText     = core::Color(255, 255, 255, 255);

    // 排版與文字色彩
    theme.m_colors.textPrimary     = core::Color(255, 255, 255, 255);
    theme.m_colors.textSecondary   = core::Color(160, 168, 184, 255);
    theme.m_colors.textDisabled    = core::Color(90, 96, 110, 255);
    theme.m_colors.textPlaceholder = core::Color(120, 130, 150, 255);
    theme.m_colors.textOnAccent    = core::Color(255, 255, 255, 255);

    return theme;
}

Theme Theme::Light() {
    Theme theme;
    theme.m_mode = ThemeMode::Light;

    // 亮色表面與視窗（柔和淺灰色調，避免過度亮白刺眼）
    theme.m_colors.windowBackground  = core::Color(234, 236, 240, 255);
    theme.m_colors.surfaceBackground = core::Color(242, 244, 248, 255);
    theme.m_colors.surfaceBorder     = core::Color(214, 219, 228, 255);
    theme.m_colors.cardBackground    = core::Color(248, 249, 250, 255);

    // 控制項狀態色彩
    theme.m_colors.controlBackground          = core::Color(228, 232, 240, 255);
    theme.m_colors.controlHoverBackground     = core::Color(218, 224, 234, 255);
    theme.m_colors.controlPressedBackground   = core::Color(206, 213, 224, 255);
    theme.m_colors.controlDisabledBackground  = core::Color(232, 235, 239, 255);
    theme.m_colors.controlBorder              = core::Color(190, 198, 212, 255);
    theme.m_colors.controlHoverBorder         = core::Color(155, 166, 188, 255);
    theme.m_colors.controlFocusedBorder       = core::Color(0, 120, 215, 255);
    theme.m_colors.controlDisabledBorder      = core::Color(210, 215, 222, 255);

    // 品牌與突顯色彩
    theme.m_colors.accentPrimary  = core::Color(0, 120, 215, 255);
    theme.m_colors.accentHover    = core::Color(16, 110, 190, 255);
    theme.m_colors.accentPressed  = core::Color(0, 90, 158, 255);
    theme.m_colors.accentDisabled = core::Color(0, 120, 215, 100);
    theme.m_colors.accentText     = core::Color(255, 255, 255, 255);

    // 排版與文字色彩
    theme.m_colors.textPrimary     = core::Color(25, 30, 40, 255);
    theme.m_colors.textSecondary   = core::Color(90, 100, 115, 255);
    theme.m_colors.textDisabled    = core::Color(150, 156, 168, 255);
    theme.m_colors.textPlaceholder = core::Color(140, 150, 165, 255);
    theme.m_colors.textOnAccent    = core::Color(255, 255, 255, 255);

    return theme;
}

Theme Theme::HighContrast() {
    Theme theme;
    theme.m_mode = ThemeMode::HighContrast;

    // 高對比黑底白字與明豔強烈邊界
    theme.m_colors.windowBackground  = core::Color(0, 0, 0, 255);
    theme.m_colors.surfaceBackground = core::Color(0, 0, 0, 255);
    theme.m_colors.surfaceBorder     = core::Color(255, 255, 255, 255);
    theme.m_colors.cardBackground    = core::Color(0, 0, 0, 255);

    theme.m_colors.controlBackground          = core::Color(0, 0, 0, 255);
    theme.m_colors.controlHoverBackground     = core::Color(30, 30, 30, 255);
    theme.m_colors.controlPressedBackground   = core::Color(60, 60, 60, 255);
    theme.m_colors.controlDisabledBackground  = core::Color(20, 20, 20, 255);
    theme.m_colors.controlBorder              = core::Color(255, 255, 255, 255);
    theme.m_colors.controlHoverBorder         = core::Color(0, 255, 255, 255);
    theme.m_colors.controlFocusedBorder       = core::Color(255, 255, 0, 255);
    theme.m_colors.controlDisabledBorder      = core::Color(128, 128, 128, 255);

    theme.m_colors.accentPrimary  = core::Color(0, 255, 255, 255);
    theme.m_colors.accentHover    = core::Color(128, 255, 255, 255);
    theme.m_colors.accentPressed  = core::Color(0, 200, 200, 255);
    theme.m_colors.accentDisabled = core::Color(0, 128, 128, 255);
    theme.m_colors.accentText     = core::Color(0, 0, 0, 255);

    theme.m_colors.textPrimary     = core::Color(255, 255, 255, 255);
    theme.m_colors.textSecondary   = core::Color(200, 200, 200, 255);
    theme.m_colors.textDisabled    = core::Color(128, 128, 128, 255);
    theme.m_colors.textPlaceholder = core::Color(160, 160, 160, 255);
    theme.m_colors.textOnAccent    = core::Color(0, 0, 0, 255);

    return theme;
}

Theme& Theme::WithAccent(core::Color primary, core::Color hover, core::Color pressed) {
    m_colors.accentPrimary = primary;
    m_colors.accentHover   = hover;
    m_colors.accentPressed = pressed;
    return *this;
}

Theme& Theme::WithFontFamily(compat::string_view family) {
    if (family.empty()) {
        throw std::invalid_argument("Theme::WithFontFamily: family name cannot be empty.");
    }
    m_typography.fontFamily = std::string(family.data(), family.size());
    return *this;
}

Theme& Theme::WithCornerRadius(float mediumRadius) {
    m_metrics.cornerRadiusMedium = mediumRadius;
    m_metrics.cornerRadiusSmall  = mediumRadius * 0.5f;
    m_metrics.cornerRadiusLarge  = mediumRadius * 1.5f;
    return *this;
}

} // namespace theme
} // namespace cppui
