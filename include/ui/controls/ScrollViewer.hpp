/*
 * File: ScrollViewer.hpp
 * Purpose: Declares ScrollViewer control providing scrollable viewport, mouse wheel
 *          interaction, boundary clipping, and visual scrollbars.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>
#include <limits>
#include "compat/compat.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/input/PointerEventArgs.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 卷軸可見性策略列舉 (ScrollBarVisibility，對標 WPF ScrollBarVisibility)。
/// </summary>
enum class ScrollBarVisibility : uint32_t {
    Disabled = 0,
    Auto     = 1,
    Hidden   = 2,
    Visible  = 3
};

/// <summary>
/// 具備可視區域滾動、滑鼠滾輪支援與自繪卷軸軌道滑塊的滾動容器控制項 (ScrollViewer)。
/// </summary>
class ScrollViewer : public ContentControl {
public:
    using ScrollBarVisibility = cppui::controls::ScrollBarVisibility;

    /// <summary>
    /// 建構空白滾動容器控制項。
    /// </summary>
    ScrollViewer();

    /// <summary>
    /// 建構包含特定子內容之滾動容器控制項。
    /// </summary>
    /// <param name="content">子元件智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 content 為 nullptr 時擲出。</exception>
    explicit ScrollViewer(std::shared_ptr<core::UIElement> content);

    ~ScrollViewer() override = default;

    /// <summary>
    /// 取得當前垂直滾動偏移量 (像素)。
    /// </summary>
    /// <returns>垂直偏移像素值。</returns>
    COMPAT_NODISCARD float GetVerticalOffset() const noexcept { return m_verticalOffset; }

    /// <summary>
    /// 設定垂直滾動偏移量 (將自動限制於 [0, ExtentHeight - ViewportHeight])。
    /// </summary>
    /// <param name="offset">目標偏移像素值。</param>
    void SetVerticalOffset(float offset);

    /// <summary>
    /// 取得當前水平滾動偏移量 (像素)。
    /// </summary>
    /// <returns>水平偏移像素值。</returns>
    COMPAT_NODISCARD float GetHorizontalOffset() const noexcept { return m_horizontalOffset; }

    /// <summary>
    /// 設定水平滾動偏移量。
    /// </summary>
    /// <param name="offset">目標偏移像素值。</param>
    void SetHorizontalOffset(float offset);

    /// <summary>
    /// 取得內容延伸整體高度 (ExtentHeight)。
    /// </summary>
    /// <returns>內容總高度像素值。</returns>
    COMPAT_NODISCARD float GetExtentHeight() const noexcept { return m_extentHeight; }

    /// <summary>
    /// 取得內容延伸整體寬度 (ExtentWidth)。
    /// </summary>
    /// <returns>內容總寬度像素值。</returns>
    COMPAT_NODISCARD float GetExtentWidth() const noexcept { return m_extentWidth; }

    /// <summary>
    /// 取得當前可視視區高度 (ViewportHeight)。
    /// </summary>
    /// <returns>視區高度像素值。</returns>
    COMPAT_NODISCARD float GetViewportHeight() const noexcept { return m_viewportHeight; }

    /// <summary>
    /// 取得當前可視視區寬度 (ViewportWidth)。
    /// </summary>
    /// <returns>視區寬度像素值。</returns>
    COMPAT_NODISCARD float GetViewportWidth() const noexcept { return m_viewportWidth; }

    /// <summary>
    /// 取得單次滾輪或按鍵滾動之步進距離 (預設為 32 像素)。
    /// </summary>
    /// <returns>步進距離像素值。</returns>
    COMPAT_NODISCARD float GetScrollStep() const noexcept { return m_scrollStep; }

    /// <summary>
    /// 設定單次滾動步進距離。
    /// </summary>
    /// <param name="step">步進距離像素值。</param>
    void SetScrollStep(float step) noexcept;

    /// <summary>
    /// 取得垂直卷軸之寬度。
    /// </summary>
    /// <returns>卷軸寬度像素值。</returns>
    COMPAT_NODISCARD float GetScrollBarWidth() const noexcept { return m_scrollBarWidth; }

    /// <summary>
    /// 設定垂直卷軸寬度。
    /// </summary>
    /// <param name="width">卷軸寬度像素值。</param>
    void SetScrollBarWidth(float width) noexcept;

    /// <summary>
    /// 取得水平卷軸可見性策略。
    /// </summary>
    /// <returns>ScrollBarVisibility 列舉值。</returns>
    COMPAT_NODISCARD ScrollBarVisibility GetHorizontalScrollBarVisibility() const noexcept {
        return m_horizontalScrollBarVisibility;
    }

    /// <summary>
    /// 設定水平卷軸可見性策略。
    /// </summary>
    /// <param name="visibility">ScrollBarVisibility 列舉值。</param>
    void SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility) noexcept {
        m_horizontalScrollBarVisibility = visibility;
        InvalidateVisual();
    }

    /// <summary>
    /// 取得垂直卷軸可見性策略。
    /// </summary>
    /// <returns>ScrollBarVisibility 列舉值。</returns>
    COMPAT_NODISCARD ScrollBarVisibility GetVerticalScrollBarVisibility() const noexcept {
        return m_verticalScrollBarVisibility;
    }

    /// <summary>
    /// 設定垂直卷軸可見性策略。
    /// </summary>
    /// <param name="visibility">ScrollBarVisibility 列舉值。</param>
    void SetVerticalScrollBarVisibility(ScrollBarVisibility visibility) noexcept {
        m_verticalScrollBarVisibility = visibility;
        InvalidateVisual();
    }

    /// <summary>滾動至最頂端。</summary>
    void ScrollToHome();

    /// <summary>滾動至最底端。</summary>
    void ScrollToEnd();

    /// <summary>滾動至最頂端 (對標 WPF ScrollToTop)。</summary>
    void ScrollToTop() { ScrollToHome(); }

    /// <summary>滾動至最底端 (對標 WPF ScrollToBottom)。</summary>
    void ScrollToBottom() { ScrollToEnd(); }

    /// <summary>滾動至最左端 (對標 WPF ScrollToLeftEnd)。</summary>
    void ScrollToLeftEnd() { SetHorizontalOffset(0.0f); }

    /// <summary>滾動至最右端 (對標 WPF ScrollToRightEnd)。</summary>
    void ScrollToRightEnd() { SetHorizontalOffset(m_extentWidth - m_viewportWidth); }

    /// <summary>向上滾動一行步進量。</summary>
    void LineUp();

    /// <summary>向下滾動一行步進量。</summary>
    void LineDown();

    /// <summary>向左滾動一行步進量。</summary>
    void LineLeft() { SetHorizontalOffset(m_horizontalOffset - m_scrollStep); }

    /// <summary>向右滾動一行步進量。</summary>
    void LineRight() { SetHorizontalOffset(m_horizontalOffset + m_scrollStep); }

    /// <summary>向上滾動一整頁視區高度。</summary>
    void PageUp();

    /// <summary>向下滾動一整頁視區高度。</summary>
    void PageDown();

    /// <summary>
    /// 測量階段：允許內部子內容具有無限垂直高度以獲取真實延伸尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 排列階段：依據垂直偏移位置配置子內容，並計算可視區範圍。
    /// </summary>
    /// <param name="finalSize">容器最終尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

    /// <summary>
    /// 繪製容器外觀背景與邊框。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 後置繪製階段：於子內容上方呈現浮動卷軸軌道與滑塊。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnPostRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣至卷軸。
    /// </summary>
    /// <param name="theme">主題常數引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

protected:
    /// <summary>
    /// 計算卷軸軌道之區域矩形。
    /// </summary>
    /// <returns>軌道矩形。</returns>
    COMPAT_NODISCARD core::Rect GetTrackRect() const noexcept;

    /// <summary>
    /// 計算卷軸滑塊之區域矩形。
    /// </summary>
    /// <returns>滑塊矩形。</returns>
    COMPAT_NODISCARD core::Rect GetThumbRect() const noexcept;

private:
    void InitializeHandlers();

    float m_verticalOffset   = 0.0f;
    float m_horizontalOffset = 0.0f;
    float m_extentHeight     = 0.0f;
    float m_extentWidth      = 0.0f;
    float m_viewportHeight   = 0.0f;
    float m_viewportWidth    = 0.0f;
    float m_scrollStep       = 32.0f;
    float m_scrollBarWidth   = 10.0f;

    ScrollBarVisibility m_horizontalScrollBarVisibility = ScrollBarVisibility::Disabled;
    ScrollBarVisibility m_verticalScrollBarVisibility   = ScrollBarVisibility::Auto;

    bool  m_isDraggingThumb  = false;
    bool  m_isThumbHovered   = false;
    float m_dragStartMouseY  = 0.0f;
    float m_dragStartOffset  = 0.0f;

    core::Color m_trackColor{ 30, 34, 45, 140 };
    core::Color m_thumbColor{ 100, 115, 140, 180 };
    core::Color m_thumbHoverColor{ 130, 150, 180, 220 };
    core::Color m_thumbPressedColor{ 160, 180, 210, 255 };
};

} // namespace controls
} // namespace cppui
