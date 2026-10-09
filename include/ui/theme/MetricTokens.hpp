/*
 * File: MetricTokens.hpp
 * Purpose: Declares geometric, padding, and corner radius tokens for the CPPUI theme system.
 */

#pragma once

#include "ui/core/Geometry.hpp"

namespace cppui {
namespace theme {

/// <summary>
/// 集中式幾何間距與圓角代幣結構 (MetricTokens)
/// 提供統一的邊框粗細、圓角半徑、控制項內距與排版間距尺度。
/// </summary>
struct MetricTokens {
    // 圓角尺度 (Corner Radii)
    float cornerRadiusSmall = 3.0f;
    float cornerRadiusMedium = 6.0f;
    float cornerRadiusLarge = 8.0f;
    float cornerRadiusPill = 999.0f;

    // 邊框粗細 (Border Thickness)
    float borderThicknessDefault = 1.0f;
    float borderThicknessFocused = 2.0f;

    // 控制項常用內距 (Paddings)
    core::Thickness paddingButton{ 16.0f, 8.0f, 16.0f, 8.0f };
    core::Thickness paddingInput{ 10.0f, 6.0f, 10.0f, 6.0f };

    // 排版間距階層 (Spacing Scale)
    float spacingXs = 4.0f;
    float spacingSm = 8.0f;
    float spacingMd = 16.0f;
    float spacingLg = 24.0f;
    float spacingXl = 32.0f;
};

} // namespace theme
} // namespace cppui
