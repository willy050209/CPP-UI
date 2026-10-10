/**
 * @file Separator.cpp
 * @brief 分隔線控制項實作
 */

#include "ui/controls/Separator.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <stdexcept>

namespace cppui {
namespace controls {

Separator::Separator() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    m_lineColor = theme.Colors().controlBorder;
    SetFocusable(false);
    SetTabStop(false);
    SetMargin(core::Thickness(0.0f, 4.0f, 0.0f, 4.0f));
}

Separator::Separator(layout::Orientation orientation) : Separator() {
    m_orientation = orientation;
    if (orientation == layout::Orientation::Vertical) {
        SetMargin(core::Thickness(4.0f, 0.0f, 4.0f, 0.0f));
    }
}

void Separator::SetOrientation(layout::Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        InvalidateMeasure();
        InvalidateVisual();
    }
}

void Separator::SetThickness(float thickness) {
    if (thickness < 0.0f) {
        throw std::invalid_argument("Separator::SetThickness: thickness cannot be negative.");
    }
    if (m_thickness != thickness) {
        m_thickness = thickness;
        InvalidateMeasure();
        InvalidateVisual();
    }
}

void Separator::SetLineColor(core::Color color) {
    m_lineColor = color;
    m_hasExplicitLineColor = true;
    InvalidateVisual();
}

core::Size Separator::MeasureOverride(core::Size availableSize) {
    (void)availableSize;
    if (m_orientation == layout::Orientation::Horizontal) {
        return core::Size(0.0f, m_thickness);
    } else {
        return core::Size(m_thickness, 0.0f);
    }
}

void Separator::OnRender(rendering::DrawingContext& dc) {
    core::Rect bounds(core::Point(0, 0), GetRenderBounds().GetSize());
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) {
        return;
    }

    core::Color drawColor = m_hasExplicitLineColor ? m_lineColor : theme::ThemeManager::Get().GetCurrentTheme().Colors().controlBorder;

    if (m_orientation == layout::Orientation::Horizontal) {
        float y = bounds.y + (bounds.height - m_thickness) * 0.5f;
        dc.DrawRectangle(core::Rect(bounds.x, y, bounds.width, m_thickness), drawColor);
    } else {
        float x = bounds.x + (bounds.width - m_thickness) * 0.5f;
        dc.DrawRectangle(core::Rect(x, bounds.y, m_thickness, bounds.height), drawColor);
    }
}

} // namespace controls
} // namespace cppui
