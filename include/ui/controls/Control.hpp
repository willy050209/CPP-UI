#pragma once

#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/rendering/DrawingContext.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 基礎控制項類別 (Control)
/// 所有具備外觀樣式（背景、前景、內邊距、邊框樣式、圓角）之互動控制項的基底類別。
/// </summary>
class Control : public core::UIElement {
public:
    Control();
    ~Control() override = default;

    // --- 幾何與樣式屬性 ---

    COMPAT_NODISCARD core::Thickness GetPadding() const noexcept { return m_padding; }
    void SetPadding(const core::Thickness& padding);
    void SetPadding(float uniform) { SetPadding(core::Thickness(uniform)); }
    void SetPadding(float h, float v) { SetPadding(core::Thickness(h, v, h, v)); }

    COMPAT_NODISCARD core::Color GetBackground() const noexcept { return m_background; }
    void SetBackground(core::Color bg);

    COMPAT_NODISCARD core::Color GetForeground() const noexcept { return m_foreground; }
    void SetForeground(core::Color fg);

    COMPAT_NODISCARD core::Color GetBorderBrush() const noexcept { return m_borderBrush; }
    void SetBorderBrush(core::Color brush);

    COMPAT_NODISCARD core::Thickness GetBorderThickness() const noexcept { return m_borderThickness; }
    void SetBorderThickness(const core::Thickness& thickness);
    void SetBorderThickness(float uniform) { SetBorderThickness(core::Thickness(uniform)); }

    COMPAT_NODISCARD float GetCornerRadius() const noexcept { return m_cornerRadius; }
    void SetCornerRadius(float radius);

    /// <summary>
    /// 取得控制項左上角相對於容器左上角的座標 (對標 WinForms Control.Location)
    /// </summary>
    COMPAT_NODISCARD core::Point GetLocation() const noexcept;

    /// <summary>
    /// 設定控制項左上角相對於容器左上角的座標
    /// </summary>
    void SetLocation(core::Point location);
    void SetLocation(float x, float y);

protected:
    /// <summary>
    /// 繪製背景與外圍邊框之標準輔助函式
    /// </summary>
    void RenderBackgroundAndBorder(rendering::DrawingContext& dc, const core::Rect& bounds);

    /// <summary>
    /// 計算扣除 Padding 與 BorderThickness 後可供子元件佈局的尺寸
    /// </summary>
    COMPAT_NODISCARD core::Size DeflateSize(core::Size size) const noexcept;

    /// <summary>
    /// 計算加上 Padding 與 BorderThickness 後的外圍尺寸
    /// </summary>
    COMPAT_NODISCARD core::Size InflateSize(core::Size size) const noexcept;

    core::Thickness m_padding{ 0.0f };
    core::Color     m_background{ 0, 0, 0, 0 };
    core::Color     m_foreground{ 255, 255, 255, 255 };
    core::Color     m_borderBrush{ 0, 0, 0, 0 };
    core::Thickness m_borderThickness{ 0.0f };
    float           m_cornerRadius = 0.0f;
    core::Point     m_location{ 0.0f, 0.0f };
};

} // namespace controls
} // namespace cppui
