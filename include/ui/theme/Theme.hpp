/*
 * File: Theme.hpp
 * Purpose: Declares the Theme class encapsulating Color, Typography, and Metric tokens.
 */

#pragma once

#include "compat/compat.hpp"
#include "ThemeTypes.hpp"
#include "ColorTokens.hpp"
#include "TypographyTokens.hpp"
#include "MetricTokens.hpp"

namespace cppui {
namespace theme {

/// <summary>
/// 集中式設計規範與主題實體類別 (Theme)
/// 封裝全域色彩、字級排版與幾何尺寸代幣，支援原廠暗色/亮色主題與鏈式客製設定。
/// </summary>
class Theme {
public:
    /// <summary>
    /// 建立預設暗色主題實例 (Dark Theme)。
    /// </summary>
    /// <returns>設定為暗色模式的 Theme 實例。</returns>
    COMPAT_NODISCARD static Theme Dark();

    /// <summary>
    /// 建立預設亮色主題實例 (Light Theme)。
    /// </summary>
    /// <returns>設定為亮色模式的 Theme 實例。</returns>
    COMPAT_NODISCARD static Theme Light();

    /// <summary>
    /// 建立高對比輔助主題實例 (High Contrast Theme)。
    /// </summary>
    /// <returns>設定為高對比模式的 Theme 實例。</returns>
    COMPAT_NODISCARD static Theme HighContrast();

    /// <summary>
    /// 預設建構子 (預設為暗色主題)。
    /// </summary>
    Theme();

    /// <summary>
    /// 取得主題運作模式。
    /// </summary>
    /// <returns>ThemeMode 列舉值。</returns>
    COMPAT_NODISCARD ThemeMode GetMode() const noexcept { return m_mode; }

    /// <summary>
    /// 設定主題運作模式。
    /// </summary>
    /// <param name="mode">ThemeMode 列舉值。</param>
    void SetMode(ThemeMode mode) noexcept { m_mode = mode; }

    /// <summary>
    /// 取得色彩代幣常數引用。
    /// </summary>
    /// <returns>ColorTokens 引用。</returns>
    COMPAT_NODISCARD const ColorTokens& Colors() const noexcept { return m_colors; }

    /// <summary>
    /// 取得可修改之色彩代幣引用。
    /// </summary>
    /// <returns>ColorTokens 引用。</returns>
    COMPAT_NODISCARD ColorTokens& Colors() noexcept { return m_colors; }

    /// <summary>
    /// 取得排版代幣常數引用。
    /// </summary>
    /// <returns>TypographyTokens 引用。</returns>
    COMPAT_NODISCARD const TypographyTokens& Typography() const noexcept { return m_typography; }

    /// <summary>
    /// 取得可修改之排版代幣引用。
    /// </summary>
    /// <returns>TypographyTokens 引用。</returns>
    COMPAT_NODISCARD TypographyTokens& Typography() noexcept { return m_typography; }

    /// <summary>
    /// 取得幾何度量代幣常數引用。
    /// </summary>
    /// <returns>MetricTokens 引用。</returns>
    COMPAT_NODISCARD const MetricTokens& Metrics() const noexcept { return m_metrics; }

    /// <summary>
    /// 取得可修改之幾何度量代幣引用。
    /// </summary>
    /// <returns>MetricTokens 引用。</returns>
    COMPAT_NODISCARD MetricTokens& Metrics() noexcept { return m_metrics; }

    // --- 鏈式配置方法 (Fluent Chaining) ---

    /// <summary>
    /// 覆寫主要品牌突顯色彩。
    /// </summary>
    /// <param name="primary">主要強調色。</param>
    /// <param name="hover">懸停強調色。</param>
    /// <param name="pressed">按下強調色。</param>
    /// <returns>自身引用支援鏈式調用。</returns>
    Theme& WithAccent(core::Color primary, core::Color hover, core::Color pressed);

    /// <summary>
    /// 覆寫全域預設 UI 字型家族名稱。
    /// </summary>
    /// <param name="family">字型家族名稱視圖，不可為空。</param>
    /// <returns>自身引用支援鏈式調用。</returns>
    /// <exception cref="std::invalid_argument">當 family 為空時擲出。</exception>
    Theme& WithFontFamily(compat::string_view family);

    /// <summary>
    /// 覆寫中型圓角半徑規格。
    /// </summary>
    /// <param name="mediumRadius">中型圓角像素半徑。</param>
    /// <returns>自身引用支援鏈式調用。</returns>
    Theme& WithCornerRadius(float mediumRadius);

private:
    ThemeMode        m_mode = ThemeMode::Dark;
    ColorTokens      m_colors;
    TypographyTokens m_typography;
    MetricTokens     m_metrics;
};

} // namespace theme
} // namespace cppui
