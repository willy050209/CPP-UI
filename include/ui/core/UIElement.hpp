#pragma once

#include <limits>
#include <algorithm>
#include "compat/compat.hpp"
#include "ui/core/Visual.hpp"
#include "ui/layout/LayoutTypes.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 具備佈局幾何約束、髒標記追蹤與雙階段排版管線的核心 UI 元件基底類別 (UIElement)
/// 繼承自 Visual，引入 Margin、Alignment、MeasureOverride 與 ArrangeOverride。
/// </summary>
class UIElement : public Visual {
public:
    UIElement() = default;
    ~UIElement() override = default;

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

protected:
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
