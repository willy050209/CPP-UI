/**
 * @file Control.hpp
 * @brief 基礎控制項基底類別，提供背景、前景、內邊距、邊框樣式、圓角與幾何座標定位
 */

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
    /// <summary>
    /// 建構基礎控制項實例。
    /// </summary>
    Control();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~Control() override = default;

    // --- 幾何與樣式屬性 ---

    /// <summary>
    /// 取得內部留白厚度 (Padding)。
    /// </summary>
    /// <returns>Thickness 物件。</returns>
    COMPAT_NODISCARD core::Thickness GetPadding() const noexcept { return m_padding; }

    /// <summary>
    /// 設定內部留白厚度。
    /// </summary>
    /// <param name="padding">四方向內距厚度。</param>
    void SetPadding(const core::Thickness& padding);

    /// <summary>
    /// 設定四邊等寬的內部留白厚度。
    /// </summary>
    /// <param name="uniform">均勻內距像素值。</param>
    void SetPadding(float uniform) { SetPadding(core::Thickness(uniform)); }

    /// <summary>
    /// 設定水平與垂直對稱的內部留白厚度。
    /// </summary>
    /// <param name="h">水平內距像素值。</param>
    /// <param name="v">垂直內距像素值。</param>
    void SetPadding(float h, float v) { SetPadding(core::Thickness(h, v, h, v)); }

    /// <summary>
    /// 取得背景填色色彩。
    /// </summary>
    /// <returns>背景色彩。</returns>
    COMPAT_NODISCARD core::Color GetBackground() const noexcept { return m_background; }

    /// <summary>
    /// 設定背景填色色彩。
    /// </summary>
    /// <param name="bg">背景色彩。</param>
    void SetBackground(core::Color bg);

    /// <summary>
    /// 取得前景筆刷色彩。
    /// </summary>
    /// <returns>前景色彩。</returns>
    COMPAT_NODISCARD core::Color GetForeground() const noexcept { return m_foreground; }

    /// <summary>
    /// 設定前景筆刷色彩。
    /// </summary>
    /// <param name="fg">前景色彩。</param>
    void SetForeground(core::Color fg);

    /// <summary>
    /// 取得邊框筆刷色彩。
    /// </summary>
    /// <returns>邊框筆刷色彩。</returns>
    COMPAT_NODISCARD core::Color GetBorderBrush() const noexcept { return m_borderBrush; }

    /// <summary>
    /// 設定邊框筆刷色彩。
    /// </summary>
    /// <param name="brush">邊框筆刷色彩。</param>
    void SetBorderBrush(core::Color brush);

    /// <summary>
    /// 取得邊框厚度。
    /// </summary>
    /// <returns>Thickness 物件。</returns>
    COMPAT_NODISCARD core::Thickness GetBorderThickness() const noexcept { return m_borderThickness; }

    /// <summary>
    /// 設定邊框厚度。
    /// </summary>
    /// <param name="thickness">邊框四邊厚度。</param>
    void SetBorderThickness(const core::Thickness& thickness);

    /// <summary>
    /// 設定四邊等寬之邊框厚度。
    /// </summary>
    /// <param name="uniform">均勻邊框寬度。</param>
    void SetBorderThickness(float uniform) { SetBorderThickness(core::Thickness(uniform)); }

    /// <summary>
    /// 取得邊框圓角半徑。
    /// </summary>
    /// <returns>圓角半徑像素值。</returns>
    COMPAT_NODISCARD float GetCornerRadius() const noexcept { return m_cornerRadius; }

    /// <summary>
    /// 設定邊框圓角半徑。
    /// </summary>
    /// <param name="radius">圓角半徑像素值。</param>
    void SetCornerRadius(float radius);

    /// <summary>
    /// 取得控制項左上角相對於容器左上角的座標 (對標 WinForms Control.Location)。
    /// </summary>
    /// <returns>相對於父容器之左上角座標點。</returns>
    COMPAT_NODISCARD core::Point GetLocation() const noexcept;

    /// <summary>
    /// 設定控制項左上角相對於容器左上角的座標。
    /// </summary>
    /// <param name="location">左上角座標點。</param>
    void SetLocation(core::Point location);

    /// <summary>
    /// 設定控制項左上角相對於容器左上角的座標。
    /// </summary>
    /// <param name="x">X 軸像素座標。</param>
    /// <param name="y">Y 軸像素座標。</param>
    void SetLocation(float x, float y);

protected:
    /// <summary>
    /// 繪製背景與外圍邊框之標準輔助函式。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    /// <param name="bounds">繪製矩形邊界。</param>
    void RenderBackgroundAndBorder(rendering::DrawingContext& dc, const core::Rect& bounds);

    /// <summary>
    /// 計算扣除 Padding 與 BorderThickness 後可供子元件佈局的尺寸。
    /// </summary>
    /// <param name="size">外圍原始尺寸。</param>
    /// <returns>扣除邊框與內距後之內容尺寸。</returns>
    COMPAT_NODISCARD core::Size DeflateSize(core::Size size) const noexcept;

    /// <summary>
    /// 計算加上 Padding 與 BorderThickness 後的外圍尺寸。
    /// </summary>
    /// <param name="size">內部內容尺寸。</param>
    /// <returns>加上邊框與內距後之外圍尺寸。</returns>
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
