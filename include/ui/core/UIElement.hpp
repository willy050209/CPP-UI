#pragma once

#include <limits>
#include <algorithm>
#include <functional>
#include <vector>
#include "compat/compat.hpp"
#include "ui/core/Visual.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/FocusEventArgs.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 具備佈局幾何約束、髒標記追蹤、雙階段排版管線與路由事件分發的核心 UI 元件基底類別 (UIElement)
/// 繼承自 Visual，引入 Margin、Alignment、Measure/Arrange 及 RoutedEvent 事件處理。
/// </summary>
class UIElement : public Visual {
public:
    using RoutedEventHandler    = std::function<void(input::RoutedEventArgs&)>;
    using PointerEventHandler   = std::function<void(input::PointerEventArgs&)>;
    using KeyEventHandler       = std::function<void(input::KeyEventArgs&)>;
    using TextInputEventHandler = std::function<void(input::TextInputEventArgs&)>;
    using FocusEventHandler     = std::function<void(input::FocusEventArgs&)>;

    UIElement() = default;
    ~UIElement() override;

    // --- 佈局幾何約束屬性 ---
    COMPAT_NODISCARD Thickness GetMargin() const noexcept { return m_margin; }
    void SetMargin(const Thickness& margin);

    COMPAT_NODISCARD float GetWidth() const noexcept { return m_width; }
    void SetWidth(float width);

    COMPAT_NODISCARD float GetHeight() const noexcept { return m_height; }
    void SetHeight(float height);

    COMPAT_NODISCARD float GetMinWidth() const noexcept { return m_minWidth; }
    void SetMinWidth(float minWidth);

    COMPAT_NODISCARD float GetMaxWidth() const noexcept { return m_maxWidth; }
    void SetMaxWidth(float maxWidth);

    COMPAT_NODISCARD float GetMinHeight() const noexcept { return m_minHeight; }
    void SetMinHeight(float minHeight);

    COMPAT_NODISCARD float GetMaxHeight() const noexcept { return m_maxHeight; }
    void SetMaxHeight(float maxHeight);

    COMPAT_NODISCARD layout::HorizontalAlignment GetHorizontalAlignment() const noexcept { return m_horizontalAlignment; }
    void SetHorizontalAlignment(layout::HorizontalAlignment alignment);

    COMPAT_NODISCARD layout::VerticalAlignment GetVerticalAlignment() const noexcept { return m_verticalAlignment; }
    void SetVerticalAlignment(layout::VerticalAlignment alignment);

    COMPAT_NODISCARD layout::Visibility GetVisibility() const noexcept { return m_visibility; }
    void SetVisibility(layout::Visibility visibility);

    // --- 排版結果與邊界快取 ---
    COMPAT_NODISCARD Size GetDesiredSize() const noexcept { return m_desiredSize; }
    COMPAT_NODISCARD Rect GetRenderBounds() const noexcept { return m_renderBounds; }

    // --- 髒標記狀態與控制 ---
    COMPAT_NODISCARD bool IsMeasureValid() const noexcept { return m_isMeasureValid; }
    COMPAT_NODISCARD bool IsArrangeValid() const noexcept { return m_isArrangeValid; }
    COMPAT_NODISCARD bool IsVisualValid() const noexcept { return m_isVisualValid; }

    /// <summary>標記尺寸為髒區，並向上冒泡通知父容器重新排版</summary>
    void InvalidateMeasure();

    /// <summary>標記排列為髒區，通知重新配置子節點位置</summary>
    void InvalidateArrange();

    /// <summary>標記視覺為髒區，僅通知重繪，完全不重跑 Measure 與 Arrange</summary>
    void InvalidateVisual();

    // --- 雙階段排版管線入口 ---
    void Measure(Size availableSize);
    void Arrange(Rect finalRect);

    // --- 子類別重寫之排版實作點 ---
    virtual Size MeasureOverride(Size availableSize);
    virtual void ArrangeOverride(Size finalSize);

    COMPAT_NODISCARD UIElement* GetParentUIElement() const noexcept;

    // --- 路由事件註冊與派發 ---
    void AddHandler(const input::RoutedEvent& routedEvent, RoutedEventHandler handler, bool handledEventsToo = false);
    void RemoveHandler(const input::RoutedEvent& routedEvent);
    void RaiseEvent(input::RoutedEventArgs& e);
    void RaiseEventInternal(const input::RoutedEvent& routedEvent, input::RoutedEventArgs& e);

    // 便捷事件掛載語法糖
    void OnPointerPressed(PointerEventHandler handler);
    void OnPointerReleased(PointerEventHandler handler);
    void OnPointerMoved(PointerEventHandler handler);
    void OnPointerEntered(PointerEventHandler handler);
    void OnPointerExited(PointerEventHandler handler);
    void OnKeyDown(KeyEventHandler handler);
    void OnKeyUp(KeyEventHandler handler);
    void OnTextInput(TextInputEventHandler handler);
    void OnGotFocus(FocusEventHandler handler);
    void OnLostFocus(FocusEventHandler handler);

    // 子類別事件處理虛擬回呼點
    virtual void OnPointerPressedCore(input::PointerEventArgs& e) { (void)e; }
    virtual void OnPointerReleasedCore(input::PointerEventArgs& e) { (void)e; }
    virtual void OnPointerMovedCore(input::PointerEventArgs& e) { (void)e; }
    virtual void OnPointerEnteredCore(input::PointerEventArgs& e) { (void)e; }
    virtual void OnPointerExitedCore(input::PointerEventArgs& e) { (void)e; }
    virtual void OnKeyDownCore(input::KeyEventArgs& e) { (void)e; }
    virtual void OnKeyUpCore(input::KeyEventArgs& e) { (void)e; }
    virtual void OnTextInputCore(input::TextInputEventArgs& e) { (void)e; }
    virtual void OnGotFocusCore(input::FocusEventArgs& e) { (void)e; }
    virtual void OnLostFocusCore(input::FocusEventArgs& e) { (void)e; }

    // --- 命中測試與焦點屬性 ---
    COMPAT_NODISCARD Visual* HitTest(Point localPoint) override;

    COMPAT_NODISCARD bool IsHitTestVisible() const noexcept { return m_isHitTestVisible; }
    void SetHitTestVisible(bool visible) noexcept { m_isHitTestVisible = visible; }

    COMPAT_NODISCARD bool IsFocusable() const noexcept { return m_isFocusable; }
    void SetFocusable(bool focusable) noexcept { m_isFocusable = focusable; }

    COMPAT_NODISCARD bool IsFocused() const noexcept { return m_isFocused; }
    void SetFocusedInternal(bool focused) noexcept;

    COMPAT_NODISCARD int GetTabIndex() const noexcept { return m_tabIndex; }
    void SetTabIndex(int index) noexcept { m_tabIndex = index; }

    COMPAT_NODISCARD bool IsTabStop() const noexcept { return m_isTabStop; }
    void SetTabStop(bool tabStop) noexcept { m_isTabStop = tabStop; }

protected:
    struct HandlerEntry {
        uint32_t           eventId = 0;
        RoutedEventHandler handler;
        bool               handledEventsToo = false;
    };
    std::vector<HandlerEntry> m_eventHandlers;

    bool                        m_isHitTestVisible = true;
    bool                        m_isFocusable = false;
    bool                        m_isFocused = false;
    int                         m_tabIndex = 0;
    bool                        m_isTabStop = true;

    Thickness                   m_margin{ 0.0f, 0.0f, 0.0f, 0.0f };
    float                       m_width = -1.0f;  // < 0 表示 Auto (未約束)
    float                       m_height = -1.0f; // < 0 表示 Auto
    float                       m_minWidth = 0.0f;
    float                       m_maxWidth = 1000000.0f;
    float                       m_minHeight = 0.0f;
    float                       m_maxHeight = 1000000.0f;
    layout::HorizontalAlignment m_horizontalAlignment = layout::HorizontalAlignment::Stretch;
    layout::VerticalAlignment   m_verticalAlignment = layout::VerticalAlignment::Stretch;
    layout::Visibility          m_visibility = layout::Visibility::Visible;

    // 排版狀態快取
    Size                        m_desiredSize{ 0.0f, 0.0f };
    Rect                        m_renderBounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    Size                        m_previousAvailableSize{ -1.0f, -1.0f };
    Rect                        m_previousFinalRect{ -1.0f, -1.0f, -1.0f, -1.0f };

    bool                        m_isMeasureValid = false;
    bool                        m_isArrangeValid = false;
    bool                        m_isVisualValid = false;
};

} // namespace core
} // namespace cppui
