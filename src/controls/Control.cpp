#include "ui/controls/Control.hpp"
#include <algorithm>

namespace cppui {
namespace controls {

Control::Control() {
    SetFocusable(false);
}

void Control::SetPadding(const core::Thickness& padding) {
    if (m_padding != padding) {
        m_padding = padding;
        InvalidateMeasure();
    }
}

void Control::SetBackground(core::Color bg) {
    if (m_background != bg) {
        m_background = bg;
        InvalidateVisual();
    }
}

void Control::SetForeground(core::Color fg) {
    if (m_foreground != fg) {
        m_foreground = fg;
        InvalidateVisual();
    }
}

void Control::SetBorderBrush(core::Color brush) {
    if (m_borderBrush != brush) {
        m_borderBrush = brush;
        InvalidateVisual();
    }
}

void Control::SetBorderThickness(const core::Thickness& thickness) {
    if (m_borderThickness != thickness) {
        m_borderThickness = thickness;
        InvalidateMeasure();
    }
}

void Control::SetCornerRadius(float radius) {
    if (m_cornerRadius != radius) {
        m_cornerRadius = (std::max)(0.0f, radius);
        InvalidateVisual();
    }
}

core::Size Control::DeflateSize(core::Size size) const noexcept {
    float hInset = m_padding.Horizontal() + m_borderThickness.Horizontal();
    float vInset = m_padding.Vertical() + m_borderThickness.Vertical();
    return core::Size(
        (std::max)(0.0f, size.width - hInset),
        (std::max)(0.0f, size.height - vInset)
    );
}

core::Size Control::InflateSize(core::Size size) const noexcept {
    float hInset = m_padding.Horizontal() + m_borderThickness.Horizontal();
    float vInset = m_padding.Vertical() + m_borderThickness.Vertical();
    return core::Size(size.width + hInset, size.height + vInset);
}

void Control::RenderBackgroundAndBorder(rendering::DrawingContext& dc, const core::Rect& bounds) {
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    // 1. 繪製背景
    if (m_background.a > 0) {
        if (m_cornerRadius > 0.5f) {
            dc.DrawRoundedRectangle(bounds, m_cornerRadius, m_background);
        } else {
            dc.DrawRectangle(bounds, m_background);
        }
    }

    // 2. 繪製邊框
    if (m_borderBrush.a > 0) {
        if (m_borderThickness.top > 0.0f) {
            dc.DrawLine(core::Point(bounds.x, bounds.y),
                        core::Point(bounds.Right(), bounds.y),
                        m_borderBrush, m_borderThickness.top);
        }
        if (m_borderThickness.bottom > 0.0f) {
            dc.DrawLine(core::Point(bounds.x, bounds.Bottom()),
                        core::Point(bounds.Right(), bounds.Bottom()),
                        m_borderBrush, m_borderThickness.bottom);
        }
        if (m_borderThickness.left > 0.0f) {
            dc.DrawLine(core::Point(bounds.x, bounds.y),
                        core::Point(bounds.x, bounds.Bottom()),
                        m_borderBrush, m_borderThickness.left);
        }
        if (m_borderThickness.right > 0.0f) {
            dc.DrawLine(core::Point(bounds.Right(), bounds.y),
                        core::Point(bounds.Right(), bounds.Bottom()),
                        m_borderBrush, m_borderThickness.right);
        }
    }
}

core::Point Control::GetLocation() const noexcept {
    if (m_isArrangeValid) {
        return core::Point(m_renderBounds.x, m_renderBounds.y);
    }
    return m_location;
}

void Control::SetLocation(core::Point location) {
    m_location = location;
    m_renderBounds.x = location.x;
    m_renderBounds.y = location.y;
    SetVisualOffset(location);
    InvalidateArrange();
}

void Control::SetLocation(float x, float y) {
    SetLocation(core::Point(x, y));
}

} // namespace controls
} // namespace cppui
