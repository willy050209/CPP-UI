#include "ui/core/UIElement.hpp"
#include <algorithm>

namespace cppui {
namespace core {

void UIElement::SetMargin(const Thickness& margin) {
    if (m_margin.left != margin.left || m_margin.top != margin.top ||
        m_margin.right != margin.right || m_margin.bottom != margin.bottom) {
        m_margin = margin;
        InvalidateMeasure();
    }
}

void UIElement::SetWidth(float width) {
    if (m_width != width) {
        m_width = width;
        InvalidateMeasure();
    }
}

void UIElement::SetHeight(float height) {
    if (m_height != height) {
        m_height = height;
        InvalidateMeasure();
    }
}

void UIElement::SetMinWidth(float minWidth) {
    if (m_minWidth != minWidth) {
        m_minWidth = minWidth;
        InvalidateMeasure();
    }
}

void UIElement::SetMaxWidth(float maxWidth) {
    if (m_maxWidth != maxWidth) {
        m_maxWidth = maxWidth;
        InvalidateMeasure();
    }
}

void UIElement::SetMinHeight(float minHeight) {
    if (m_minHeight != minHeight) {
        m_minHeight = minHeight;
        InvalidateMeasure();
    }
}

void UIElement::SetMaxHeight(float maxHeight) {
    if (m_maxHeight != maxHeight) {
        m_maxHeight = maxHeight;
        InvalidateMeasure();
    }
}

void UIElement::SetHorizontalAlignment(layout::HorizontalAlignment alignment) {
    if (m_horizontalAlignment != alignment) {
        m_horizontalAlignment = alignment;
        InvalidateArrange();
    }
}

void UIElement::SetVerticalAlignment(layout::VerticalAlignment alignment) {
    if (m_verticalAlignment != alignment) {
        m_verticalAlignment = alignment;
        InvalidateArrange();
    }
}

void UIElement::SetVisibility(layout::Visibility visibility) {
    if (m_visibility != visibility) {
        m_visibility = visibility;
        SetVisible(visibility == layout::Visibility::Visible);
        InvalidateMeasure();
    }
}

UIElement* UIElement::GetParentUIElement() const noexcept {
    return dynamic_cast<UIElement*>(GetVisualParent());
}

void UIElement::InvalidateMeasure() {
    if (!m_isMeasureValid) return;
    m_isMeasureValid = false;
    m_isArrangeValid = false;

    UIElement* parent = GetParentUIElement();
    if (parent) {
        parent->InvalidateMeasure();
    }
}

void UIElement::InvalidateArrange() {
    if (!m_isArrangeValid) return;
    m_isArrangeValid = false;

    UIElement* parent = GetParentUIElement();
    if (parent) {
        parent->InvalidateArrange();
    }
}

void UIElement::InvalidateVisual() {
    m_isVisualValid = false;
}

void UIElement::Measure(Size availableSize) {
    if (m_visibility == layout::Visibility::Collapsed) {
        m_desiredSize = Size(0.0f, 0.0f);
        m_isMeasureValid = true;
        return;
    }

    if (m_isMeasureValid && availableSize == m_previousAvailableSize) {
        return; // 快取命中
    }

    // 扣除 Margin 保留空間
    float marginW = m_margin.left + m_margin.right;
    float marginH = m_margin.top + m_margin.bottom;
    float contentAvailableW = std::max(0.0f, availableSize.width - marginW);
    float contentAvailableH = std::max(0.0f, availableSize.height - marginH);

    // 套用明確 Width / Height 約束
    if (m_width >= 0.0f) {
        contentAvailableW = std::min(contentAvailableW, m_width);
    }
    if (m_height >= 0.0f) {
        contentAvailableH = std::min(contentAvailableH, m_height);
    }

    // 套用 Min/Max 約束
    contentAvailableW = compat::clamp(contentAvailableW, m_minWidth, m_maxWidth);
    contentAvailableH = compat::clamp(contentAvailableH, m_minHeight, m_maxHeight);

    // 呼叫內容測量自訂點
    Size unconstrainedContentSize = MeasureOverride(Size(contentAvailableW, contentAvailableH));

    // 結合明確尺寸與測量尺寸
    float finalContentW = (m_width >= 0.0f) ? m_width : unconstrainedContentSize.width;
    float finalContentH = (m_height >= 0.0f) ? m_height : unconstrainedContentSize.height;

    // 再次約束於 Min/Max
    finalContentW = compat::clamp(finalContentW, m_minWidth, m_maxWidth);
    finalContentH = compat::clamp(finalContentH, m_minHeight, m_maxHeight);

    // 快取 DesiredSize (包含 Margin)
    m_desiredSize = Size(finalContentW + marginW, finalContentH + marginH);
    m_previousAvailableSize = availableSize;
    m_isMeasureValid = true;
}

void UIElement::Arrange(Rect finalRect) {
    if (m_visibility == layout::Visibility::Collapsed) {
        m_renderBounds = Rect(0.0f, 0.0f, 0.0f, 0.0f);
        SetVisualOffset(Point(0.0f, 0.0f));
        SetVisualBounds(Rect(0.0f, 0.0f, 0.0f, 0.0f));
        m_isArrangeValid = true;
        return;
    }

    if (m_isArrangeValid && finalRect == m_previousFinalRect) {
        return; // 快取命中
    }

    // 扣除 Margin 幾何
    float marginW = m_margin.left + m_margin.right;
    float marginH = m_margin.top + m_margin.bottom;
    float availW = std::max(0.0f, finalRect.width - marginW);
    float availH = std::max(0.0f, finalRect.height - marginH);

    // 計算內容目標寬度
    float contentDesiredW = std::max(0.0f, m_desiredSize.width - marginW);
    float contentDesiredH = std::max(0.0f, m_desiredSize.height - marginH);

    float arrangeW = (m_width >= 0.0f) ? m_width : contentDesiredW;
    float arrangeH = (m_height >= 0.0f) ? m_height : contentDesiredH;

    if (m_horizontalAlignment == layout::HorizontalAlignment::Stretch && m_width < 0.0f) {
        arrangeW = availW;
    } else {
        arrangeW = std::min(availW, arrangeW);
    }

    if (m_verticalAlignment == layout::VerticalAlignment::Stretch && m_height < 0.0f) {
        arrangeH = availH;
    } else {
        arrangeH = std::min(availH, arrangeH);
    }

    arrangeW = compat::clamp(arrangeW, m_minWidth, m_maxWidth);
    arrangeH = compat::clamp(arrangeH, m_minHeight, m_maxHeight);

    // 根據 HorizontalAlignment 決定 X 偏移
    float offsetX = 0.0f;
    switch (m_horizontalAlignment) {
        case layout::HorizontalAlignment::Left:
            offsetX = m_margin.left;
            break;
        case layout::HorizontalAlignment::Center:
            offsetX = m_margin.left + (availW - arrangeW) * 0.5f;
            break;
        case layout::HorizontalAlignment::Right:
            offsetX = finalRect.width - m_margin.right - arrangeW;
            break;
        case layout::HorizontalAlignment::Stretch:
            offsetX = m_margin.left;
            break;
    }

    // 根據 VerticalAlignment 決定 Y 偏移
    float offsetY = 0.0f;
    switch (m_verticalAlignment) {
        case layout::VerticalAlignment::Top:
            offsetY = m_margin.top;
            break;
        case layout::VerticalAlignment::Center:
            offsetY = m_margin.top + (availH - arrangeH) * 0.5f;
            break;
        case layout::VerticalAlignment::Bottom:
            offsetY = finalRect.height - m_margin.bottom - arrangeH;
            break;
        case layout::VerticalAlignment::Stretch:
            offsetY = m_margin.top;
            break;
    }

    float finalX = finalRect.x + offsetX;
    float finalY = finalRect.y + offsetY;

    m_renderBounds = Rect(finalX, finalY, arrangeW, arrangeH);
    SetVisualOffset(Point(finalX, finalY));
    SetVisualBounds(Rect(0.0f, 0.0f, arrangeW, arrangeH));

    // 派發給子類別 ArrangeOverride
    ArrangeOverride(Size(arrangeW, arrangeH));

    m_previousFinalRect = finalRect;
    m_isArrangeValid = true;
}

Size UIElement::MeasureOverride(Size availableSize) {
    // 預設實作：測量所有子視覺節點 (若為 UIElement)
    float maxW = 0.0f;
    float maxH = 0.0f;
    for (auto& childVisual : m_visualChildren) {
        auto* childUI = dynamic_cast<UIElement*>(childVisual.get());
        if (childUI && childUI->IsVisible()) {
            childUI->Measure(availableSize);
            maxW = std::max(maxW, childUI->GetDesiredSize().width);
            maxH = std::max(maxH, childUI->GetDesiredSize().height);
        }
    }
    return Size(maxW, maxH);
}

void UIElement::ArrangeOverride(Size finalSize) {
    // 預設實作：將所有子節點直接排滿 finalSize
    Rect childRect(0.0f, 0.0f, finalSize.width, finalSize.height);
    for (auto& childVisual : m_visualChildren) {
        auto* childUI = dynamic_cast<UIElement*>(childVisual.get());
        if (childUI && childUI->IsVisible()) {
            childUI->Arrange(childRect);
        }
    }
}

} // namespace core
} // namespace cppui
