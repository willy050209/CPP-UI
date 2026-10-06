#include "ui/layout/Border.hpp"
#include <algorithm>

namespace cppui {
namespace layout {

void Border::SetChild(std::shared_ptr<core::UIElement> child) {
    if (m_child == child) return;

    if (m_child) {
        RemoveVisualChild(m_child);
    }

    m_child = std::move(child);

    if (m_child) {
        AddVisualChild(m_child);
    }

    InvalidateMeasure();
}

void Border::SetBackground(core::Color background) {
    if (m_background.r != background.r || m_background.g != background.g ||
        m_background.b != background.b || m_background.a != background.a) {
        m_background = background;
        InvalidateVisual();
    }
}

void Border::SetBorderBrush(core::Color borderBrush) {
    if (m_borderBrush.r != borderBrush.r || m_borderBrush.g != borderBrush.g ||
        m_borderBrush.b != borderBrush.b || m_borderBrush.a != borderBrush.a) {
        m_borderBrush = borderBrush;
        InvalidateVisual();
    }
}

void Border::SetBorderThickness(const core::Thickness& thickness) {
    if (m_borderThickness.left != thickness.left || m_borderThickness.top != thickness.top ||
        m_borderThickness.right != thickness.right || m_borderThickness.bottom != thickness.bottom) {
        m_borderThickness = thickness;
        InvalidateMeasure();
    }
}

void Border::SetPadding(const core::Thickness& padding) {
    if (m_padding.left != padding.left || m_padding.top != padding.top ||
        m_padding.right != padding.right || m_padding.bottom != padding.bottom) {
        m_padding = padding;
        InvalidateMeasure();
    }
}

void Border::SetCornerRadius(float cornerRadius) {
    if (m_cornerRadius != cornerRadius) {
        m_cornerRadius = cornerRadius;
        InvalidateVisual();
    }
}

core::Size Border::MeasureOverride(core::Size availableSize) {
    float borderAndPaddingW = m_borderThickness.left + m_borderThickness.right + m_padding.left + m_padding.right;
    float borderAndPaddingH = m_borderThickness.top + m_borderThickness.bottom + m_padding.top + m_padding.bottom;

    if (!m_child || m_child->GetVisibility() == Visibility::Collapsed) {
        return core::Size(borderAndPaddingW, borderAndPaddingH);
    }

    core::Size innerAvailable(
        std::max(0.0f, availableSize.width - borderAndPaddingW),
        std::max(0.0f, availableSize.height - borderAndPaddingH)
    );

    m_child->Measure(innerAvailable);
    core::Size childDesired = m_child->GetDesiredSize();

    return core::Size(childDesired.width + borderAndPaddingW, childDesired.height + borderAndPaddingH);
}

void Border::ArrangeOverride(core::Size finalSize) {
    if (!m_child || m_child->GetVisibility() == Visibility::Collapsed) {
        return;
    }

    float innerX = m_borderThickness.left + m_padding.left;
    float innerY = m_borderThickness.top + m_padding.top;
    float innerW = std::max(0.0f, finalSize.width - (m_borderThickness.left + m_borderThickness.right + m_padding.left + m_padding.right));
    float innerH = std::max(0.0f, finalSize.height - (m_borderThickness.top + m_borderThickness.bottom + m_padding.top + m_padding.bottom));

    m_child->Arrange(core::Rect(innerX, innerY, innerW, innerH));
}

void Border::OnRender(rendering::DrawingContext& dc) {
    core::Rect localRect(0.0f, 0.0f, m_renderBounds.width, m_renderBounds.height);
    if (localRect.IsEmpty()) return;

    // 1. 繪製背景
    if (m_background.a > 0) {
        if (m_cornerRadius > 0.5f) {
            dc.DrawRoundedRectangle(localRect, m_cornerRadius, m_background);
        } else {
            dc.DrawRectangle(localRect, m_background);
        }
    }

    // 2. 繪製外邊框 (若有設定筆刷與寬度)
    if (m_borderBrush.a > 0) {
        if (m_borderThickness.top > 0.0f) {
            dc.DrawLine(core::Point(localRect.x, localRect.y),
                        core::Point(localRect.Right(), localRect.y),
                        m_borderBrush, m_borderThickness.top);
        }
        if (m_borderThickness.bottom > 0.0f) {
            dc.DrawLine(core::Point(localRect.x, localRect.Bottom()),
                        core::Point(localRect.Right(), localRect.Bottom()),
                        m_borderBrush, m_borderThickness.bottom);
        }
        if (m_borderThickness.left > 0.0f) {
            dc.DrawLine(core::Point(localRect.x, localRect.y),
                        core::Point(localRect.x, localRect.Bottom()),
                        m_borderBrush, m_borderThickness.left);
        }
        if (m_borderThickness.right > 0.0f) {
            dc.DrawLine(core::Point(localRect.Right(), localRect.y),
                        core::Point(localRect.Right(), localRect.Bottom()),
                        m_borderBrush, m_borderThickness.right);
        }
    }
}

} // namespace layout
} // namespace cppui
