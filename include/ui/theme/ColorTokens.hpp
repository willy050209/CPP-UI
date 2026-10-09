/*
 * File: ColorTokens.hpp
 * Purpose: Declares semantic design color tokens for the CPPUI theme system.
 */

#pragma once

#include <cstdint>
#include "ui/core/Geometry.hpp"

namespace cppui {
namespace theme {

/// <summary>
/// 集中式語意色彩代幣結構 (ColorTokens)
/// 涵蓋視窗、介面容器表面、互動控制項、品牌強調色、排版文字及語意狀態色彩。
/// </summary>
struct ColorTokens {
    // --- 視窗與表面 (Window & Surface) ---
    core::Color windowBackground{ 24, 26, 32, 255 };
    core::Color surfaceBackground{ 32, 36, 46, 255 };
    core::Color surfaceBorder{ 50, 56, 70, 255 };
    core::Color cardBackground{ 40, 45, 58, 255 };

    // --- 互動控制項狀態 (Controls: Button, TextBox, CheckBox) ---
    core::Color controlBackground{ 45, 52, 70, 255 };
    core::Color controlHoverBackground{ 60, 75, 105, 255 };
    core::Color controlPressedBackground{ 32, 40, 58, 255 };
    core::Color controlDisabledBackground{ 35, 38, 45, 255 };

    core::Color controlBorder{ 70, 80, 100, 255 };
    core::Color controlHoverBorder{ 95, 110, 135, 255 };
    core::Color controlFocusedBorder{ 0, 160, 255, 255 };
    core::Color controlDisabledBorder{ 50, 55, 65, 255 };

    // --- 品牌與強調節點 (Accent & Brand) ---
    core::Color accentPrimary{ 0, 122, 255, 255 };
    core::Color accentHover{ 20, 140, 255, 255 };
    core::Color accentPressed{ 0, 100, 220, 255 };
    core::Color accentDisabled{ 0, 122, 255, 100 };
    core::Color accentText{ 255, 255, 255, 255 };

    // --- 文字與排版 (Typography & Foregrounds) ---
    core::Color textPrimary{ 255, 255, 255, 255 };
    core::Color textSecondary{ 160, 168, 184, 255 };
    core::Color textDisabled{ 90, 96, 110, 255 };
    core::Color textPlaceholder{ 120, 130, 150, 255 };
    core::Color textOnAccent{ 255, 255, 255, 255 };

    // --- 語意狀態色彩 (Semantic Feedback) ---
    core::Color success{ 40, 167, 69, 255 };
    core::Color warning{ 255, 193, 7, 255 };
    core::Color error{ 220, 53, 69, 255 };
    core::Color info{ 23, 162, 184, 255 };
};

} // namespace theme
} // namespace cppui
