/**
 * @file Slider.cpp
 * @brief 數值滑桿控制項實作
 */

#include "ui/controls/Slider.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <algorithm>

namespace cppui {
namespace controls {

const input::RoutedEvent Slider::ValueChangedEvent(107, "ValueChanged", input::RoutingStrategy::Bubble);

Slider::Slider() {
    InitializeHandlers();
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    m_activeTrackColor = theme.Colors().accentPrimary;
    m_trackColor = theme.Colors().controlBorder;
    m_thumbColor = core::Color(240, 245, 255, 255);
}

Slider::Slider(float min, float max, float value) : Slider() {
    m_minimum = min;
    m_maximum = (max >= min) ? max : min;
    m_value = compat::clamp(value, m_minimum, m_maximum);
}

void Slider::InitializeHandlers() {
    SetFocusable(true);
    SetTabStop(true);

    OnPointerEntered([this](input::PointerEventArgs&) {
        if (!m_enabled) return;
        m_isHovered = true;
        InvalidateVisual();
    });

    OnPointerExited([this](input::PointerEventArgs&) {
        if (!m_enabled) return;
        m_isHovered = false;
        InvalidateVisual();
    });

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (!m_enabled || !e.IsLeftButton()) return;
        input::FocusManager::Get().SetFocus(this);
        input::InputManager::Get().CapturePointer(this);
        m_isDragging = true;
        UpdateValueFromPosition(e.GetPosition(this));
        InvalidateVisual();
        e.SetHandled(true);
    });

    OnPointerMoved([this](input::PointerEventArgs& e) {
        if (m_isDragging) {
            UpdateValueFromPosition(e.GetPosition(this));
            e.SetHandled(true);
        }
    });

    OnPointerReleased([this](input::PointerEventArgs& e) {
        if (m_isDragging && e.IsLeftButton()) {
            input::InputManager::Get().ReleasePointerCapture(this);
            m_isDragging = false;
            InvalidateVisual();
            e.SetHandled(true);
        }
    });

    OnKeyDown([this](input::KeyEventArgs& e) {
        if (!m_enabled) return;
        uint32_t vk = e.GetVirtualKey();
        if (vk == 0x25 || vk == 0x28) { // Left, Down
            SetValue(m_value - m_smallChange);
            e.SetHandled(true);
        } else if (vk == 0x27 || vk == 0x26) { // Right, Up
            SetValue(m_value + m_smallChange);
            e.SetHandled(true);
        } else if (vk == 0x21) { // PageUp
            SetValue(m_value + m_largeChange);
            e.SetHandled(true);
        } else if (vk == 0x22) { // PageDown
            SetValue(m_value - m_largeChange);
            e.SetHandled(true);
        } else if (vk == 0x24) { // Home
            SetValue(m_minimum);
            e.SetHandled(true);
        } else if (vk == 0x23) { // End
            SetValue(m_maximum);
            e.SetHandled(true);
        }
    });
}

void Slider::SetMinimum(float min) {
    m_minimum = min;
    if (m_maximum < m_minimum) {
        m_maximum = m_minimum;
    }
    SetValue(m_value);
}

void Slider::SetMaximum(float max) {
    m_maximum = max;
    if (m_minimum > m_maximum) {
        m_minimum = m_maximum;
    }
    SetValue(m_value);
}

void Slider::SetValue(float val) {
    float clamped = compat::clamp(val, m_minimum, m_maximum);
    if (m_value != clamped) {
        m_value = clamped;
        InvalidateVisual();

        input::RoutedEventArgs args(ValueChangedEvent, this);
        RaiseEvent(args);
    }
}

void Slider::SetOrientation(layout::Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        InvalidateMeasure();
        InvalidateVisual();
    }
}

void Slider::OnValueChanged(std::function<void(float)> handler) {
    AddHandler(ValueChangedEvent, [this, handler](input::RoutedEventArgs&) {
        handler(m_value);
    });
}

void Slider::UpdateValueFromPosition(core::Point pos) {
    core::Size sz = GetRenderBounds().GetSize();
    if (sz.width <= 0.0f || sz.height <= 0.0f) return;

    float range = m_maximum - m_minimum;
    if (range <= 0.0f) return;

    if (m_orientation == layout::Orientation::Horizontal) {
        float usableW = (std::max)(1.0f, sz.width - m_thumbRadius * 2.0f);
        float relX = compat::clamp(pos.x - m_thumbRadius, 0.0f, usableW);
        float frac = relX / usableW;
        SetValue(m_minimum + frac * range);
    } else {
        float usableH = (std::max)(1.0f, sz.height - m_thumbRadius * 2.0f);
        float relY = compat::clamp(pos.y - m_thumbRadius, 0.0f, usableH);
        float frac = 1.0f - (relY / usableH);
        SetValue(m_minimum + frac * range);
    }
}

core::Rect Slider::GetThumbRect() const noexcept {
    core::Size sz = GetRenderBounds().GetSize();
    float diameter = m_thumbRadius * 2.0f;
    float range = m_maximum - m_minimum;
    float frac = (range > 0.0f) ? compat::clamp((m_value - m_minimum) / range, 0.0f, 1.0f) : 0.0f;

    if (m_orientation == layout::Orientation::Horizontal) {
        float usableW = (std::max)(0.0f, sz.width - diameter);
        float x = usableW * frac;
        float y = (sz.height - diameter) * 0.5f;
        return core::Rect(x, y, diameter, diameter);
    } else {
        float usableH = (std::max)(0.0f, sz.height - diameter);
        float x = (sz.width - diameter) * 0.5f;
        float y = usableH * (1.0f - frac);
        return core::Rect(x, y, diameter, diameter);
    }
}

core::Size Slider::MeasureOverride(core::Size availableSize) {
    (void)availableSize;
    if (m_orientation == layout::Orientation::Horizontal) {
        return InflateSize(core::Size(150.0f, 24.0f));
    } else {
        return InflateSize(core::Size(24.0f, 150.0f));
    }
}

void Slider::OnRender(rendering::DrawingContext& dc) {
    core::Rect bounds(core::Point(0, 0), GetRenderBounds().GetSize());
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    float trackThickness = 4.0f;
    core::Rect thumb = GetThumbRect();

    // 繪製軌道槽
    if (m_orientation == layout::Orientation::Horizontal) {
        float trackY = (bounds.height - trackThickness) * 0.5f;
        float trackX = m_thumbRadius;
        float trackW = (std::max)(0.0f, bounds.width - m_thumbRadius * 2.0f);

        core::Rect trackRect(trackX, trackY, trackW, trackThickness);
        dc.DrawRoundedRectangle(trackRect, trackThickness * 0.5f, m_trackColor);

        // 作用中亮色軌道
        float thumbCenterX = thumb.x + thumb.width * 0.5f;
        float activeW = (std::max)(0.0f, thumbCenterX - trackX);
        if (activeW > 0.0f) {
            core::Rect activeRect(trackX, trackY, activeW, trackThickness);
            dc.DrawRoundedRectangle(activeRect, trackThickness * 0.5f, m_activeTrackColor);
        }
    } else {
        float trackX = (bounds.width - trackThickness) * 0.5f;
        float trackY = m_thumbRadius;
        float trackH = (std::max)(0.0f, bounds.height - m_thumbRadius * 2.0f);

        core::Rect trackRect(trackX, trackY, trackThickness, trackH);
        dc.DrawRoundedRectangle(trackRect, trackThickness * 0.5f, m_trackColor);

        // 作用中亮色軌道 (從下至上)
        float thumbCenterY = thumb.y + thumb.height * 0.5f;
        float activeH = (std::max)(0.0f, (trackY + trackH) - thumbCenterY);
        if (activeH > 0.0f) {
            core::Rect activeRect(trackX, thumbCenterY, trackThickness, activeH);
            dc.DrawRoundedRectangle(activeRect, trackThickness * 0.5f, m_activeTrackColor);
        }
    }

    // 繪製旋鈕 (Thumb)
    core::Color thumbFill = m_thumbColor;
    core::Color thumbBorder = m_isDragging ? m_activeTrackColor : (m_isHovered ? core::Color(255, 255, 255, 255) : m_activeTrackColor);

    dc.FillEllipse(thumb, thumbFill);
    dc.DrawEllipse(thumb, thumbBorder, 2.0f);

    if (IsFocused()) {
        core::Rect ring(thumb.x - 2.5f, thumb.y - 2.5f, thumb.width + 5.0f, thumb.height + 5.0f);
        dc.DrawEllipse(ring, m_activeTrackColor, 1.5f);
    }
}

void Slider::ApplyTheme(const theme::Theme& theme) {
    Control::ApplyTheme(theme);
    m_activeTrackColor = theme.Colors().accentPrimary;
    m_trackColor = theme.Colors().controlBorder;
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
