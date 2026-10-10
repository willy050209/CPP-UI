/**
 * @file ProgressBar.cpp
 * @brief 進度指示條控制項實作
 */

#include "ui/controls/ProgressBar.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <algorithm>

namespace cppui {
namespace controls {

const input::RoutedEvent ProgressBar::ValueChangedEvent(106, "ValueChanged", input::RoutingStrategy::Bubble);

ProgressBar::ProgressBar() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    SetBackground(core::Color(30, 35, 45, 255));
    SetBorderBrush(theme.Colors().controlBorder);
    SetBorderThickness(core::Thickness(1.0f));
    SetCornerRadius(theme.Metrics().cornerRadiusSmall);
    m_progressColor = theme.Colors().accentPrimary;
    m_hasExplicitBackground = false;
    m_hasExplicitBorder = false;
}

void ProgressBar::SetMinimum(float min) {
    m_minimum = min;
    if (m_maximum < m_minimum) {
        m_maximum = m_minimum;
    }
    SetValue(m_value);
}

void ProgressBar::SetMaximum(float max) {
    m_maximum = max;
    if (m_minimum > m_maximum) {
        m_minimum = m_maximum;
    }
    SetValue(m_value);
}

void ProgressBar::SetValue(float val) {
    float clamped = compat::clamp(val, m_minimum, m_maximum);
    if (m_value != clamped) {
        m_value = clamped;
        InvalidateVisual();

        input::RoutedEventArgs args(ValueChangedEvent, this);
        RaiseEvent(args);
    }
}

void ProgressBar::PerformStep() {
    SetValue(m_value + m_step);
}

void ProgressBar::Increment(float val) {
    SetValue(m_value + val);
}

void ProgressBar::SetOrientation(layout::Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        InvalidateMeasure();
        InvalidateVisual();
    }
}

void ProgressBar::SetIndeterminate(bool indeterminate) {
    if (m_isIndeterminate != indeterminate) {
        m_isIndeterminate = indeterminate;
        InvalidateVisual();
    }
}

void ProgressBar::OnValueChanged(std::function<void(float)> handler) {
    AddHandler(ValueChangedEvent, [this, handler](input::RoutedEventArgs&) {
        handler(m_value);
    });
}

core::Size ProgressBar::MeasureOverride(core::Size availableSize) {
    (void)availableSize;
    if (m_orientation == layout::Orientation::Horizontal) {
        return InflateSize(core::Size(150.0f, 16.0f));
    } else {
        return InflateSize(core::Size(16.0f, 150.0f));
    }
}

void ProgressBar::OnRender(rendering::DrawingContext& dc) {
    core::Rect bounds(core::Point(0, 0), GetRenderBounds().GetSize());
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    // 繪製軌道底色
    if (m_cornerRadius > 0.0f) {
        dc.DrawRoundedRectangle(bounds, m_cornerRadius, m_background);
    } else {
        dc.DrawRectangle(bounds, m_background);
    }

    // 計算進度長度比例
    float range = m_maximum - m_minimum;
    float fraction = (range > 0.0f) ? compat::clamp((m_value - m_minimum) / range, 0.0f, 1.0f) : 0.0f;

    core::Rect fillRect;
    if (m_isIndeterminate) {
        // 未定跑馬燈狀態：在中間繪製 30% 長度的浮動進度區塊
        if (m_orientation == layout::Orientation::Horizontal) {
            float barW = bounds.width * 0.3f;
            float barX = (bounds.width - barW) * 0.5f;
            fillRect = core::Rect(barX, bounds.y, barW, bounds.height);
        } else {
            float barH = bounds.height * 0.3f;
            float barY = (bounds.height - barH) * 0.5f;
            fillRect = core::Rect(bounds.x, barY, bounds.width, barH);
        }
    } else {
        if (m_orientation == layout::Orientation::Horizontal) {
            float fillW = bounds.width * fraction;
            fillRect = core::Rect(bounds.x, bounds.y, fillW, bounds.height);
        } else {
            float fillH = bounds.height * fraction;
            float fillY = bounds.Bottom() - fillH;
            fillRect = core::Rect(bounds.x, fillY, bounds.width, fillH);
        }
    }

    if (fillRect.width > 0.0f && fillRect.height > 0.0f) {
        if (m_cornerRadius > 0.0f) {
            dc.DrawRoundedRectangle(fillRect, m_cornerRadius, m_progressColor);
        } else {
            dc.DrawRectangle(fillRect, m_progressColor);
        }
    }

    // 繪製外框
    if (m_borderBrush.a > 0 && m_borderThickness.top > 0.0f) {
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.Right(), bounds.y), m_borderBrush, m_borderThickness.top);
        dc.DrawLine(core::Point(bounds.x, bounds.Bottom()), core::Point(bounds.Right(), bounds.Bottom()), m_borderBrush, m_borderThickness.bottom);
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.x, bounds.Bottom()), m_borderBrush, m_borderThickness.left);
        dc.DrawLine(core::Point(bounds.Right(), bounds.y), core::Point(bounds.Right(), bounds.Bottom()), m_borderBrush, m_borderThickness.right);
    }
}

void ProgressBar::ApplyTheme(const theme::Theme& theme) {
    Control::ApplyTheme(theme);
    if (!m_hasExplicitBackground) {
        m_background = core::Color(30, 35, 45, 255);
    }
    if (!m_hasExplicitBorder) {
        m_borderBrush = theme.Colors().controlBorder;
    }
    m_progressColor = theme.Colors().accentPrimary;
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
