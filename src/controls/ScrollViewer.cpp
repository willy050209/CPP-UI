/*
 * File: ScrollViewer.cpp
 * Purpose: Implements ScrollViewer control with viewport scrolling, pointer interaction,
 *          wheel scrolling, and theme-adaptive scrollbars.
 */

#include "ui/controls/ScrollViewer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include "ui/input/InputManager.hpp"
#include "ui/theme/ThemeManager.hpp"

namespace cppui {
namespace controls {

ScrollViewer::ScrollViewer() : ContentControl() {
    SetClipToBounds(true);
    InitializeHandlers();
}

ScrollViewer::ScrollViewer(std::shared_ptr<core::UIElement> content)
    : ContentControl(content) {
    SetClipToBounds(true);
    InitializeHandlers();
}

void ScrollViewer::InitializeHandlers() {
    OnPointerWheel([this](input::PointerEventArgs& e) {
        int32_t deltaY = e.GetWheelDeltaY();
        if (deltaY != 0 && m_extentHeight > m_viewportHeight) {
            float deltaSteps = static_cast<float>(deltaY) / 120.0f;
            SetVerticalOffset(m_verticalOffset - deltaSteps * m_scrollStep);
            e.SetHandled(true);
        }
    });

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (!m_enabled || !e.IsLeftButton()) return;
        if (m_extentHeight <= m_viewportHeight) return;

        core::Point pos = e.GetPosition(this);
        core::Rect thumbRect = GetThumbRect();
        core::Rect trackRect = GetTrackRect();

        if (thumbRect.Contains(pos)) {
            m_isDraggingThumb = true;
            m_dragStartMouseY = pos.y;
            m_dragStartOffset = m_verticalOffset;
            input::InputManager::Get().CapturePointer(this);
            InvalidateVisual();
            e.SetHandled(true);
        } else if (trackRect.Contains(pos)) {
            if (pos.y < thumbRect.y) {
                PageUp();
            } else {
                PageDown();
            }
            e.SetHandled(true);
        }
    });

    OnPointerMoved([this](input::PointerEventArgs& e) {
        if (!m_enabled) return;
        core::Point pos = e.GetPosition(this);
        if (m_isDraggingThumb) {
            float deltaY = pos.y - m_dragStartMouseY;
            float thumbH = GetThumbRect().height;
            float trackScrollable = m_viewportHeight - thumbH;
            float maxScroll = m_extentHeight - m_viewportHeight;
            if (trackScrollable > 0.0f && maxScroll > 0.0f) {
                float offsetDelta = (deltaY / trackScrollable) * maxScroll;
                SetVerticalOffset(m_dragStartOffset + offsetDelta);
            }
            e.SetHandled(true);
        } else {
            bool wasHovered = m_isThumbHovered;
            m_isThumbHovered = GetThumbRect().Contains(pos);
            if (wasHovered != m_isThumbHovered) {
                InvalidateVisual();
            }
        }
    });

    OnPointerReleased([this](input::PointerEventArgs& e) {
        if (m_isDraggingThumb) {
            m_isDraggingThumb = false;
            input::InputManager::Get().ReleasePointerCapture(this);
            InvalidateVisual();
            e.SetHandled(true);
        }
    });

    OnPointerExited([this](input::PointerEventArgs&) {
        if (m_isThumbHovered && !m_isDraggingThumb) {
            m_isThumbHovered = false;
            InvalidateVisual();
        }
    });
}

void ScrollViewer::SetVerticalOffset(float offset) {
    float maxOffset = (std::max)(0.0f, m_extentHeight - m_viewportHeight);
    float clamped = compat::clamp(offset, 0.0f, maxOffset);
    if (std::abs(m_verticalOffset - clamped) > 0.001f) {
        m_verticalOffset = clamped;
        InvalidateArrange();
        InvalidateVisual();
    }
}

void ScrollViewer::SetHorizontalOffset(float offset) {
    float maxOffset = (std::max)(0.0f, m_extentWidth - m_viewportWidth);
    float clamped = compat::clamp(offset, 0.0f, maxOffset);
    if (std::abs(m_horizontalOffset - clamped) > 0.001f) {
        m_horizontalOffset = clamped;
        InvalidateArrange();
        InvalidateVisual();
    }
}

void ScrollViewer::SetScrollStep(float step) noexcept {
    m_scrollStep = (std::max)(1.0f, step);
}

void ScrollViewer::SetScrollBarWidth(float width) noexcept {
    m_scrollBarWidth = (std::max)(4.0f, width);
    InvalidateArrange();
    InvalidateVisual();
}

void ScrollViewer::ScrollToHome() {
    SetVerticalOffset(0.0f);
}

void ScrollViewer::ScrollToEnd() {
    SetVerticalOffset(m_extentHeight);
}

void ScrollViewer::LineUp() {
    SetVerticalOffset(m_verticalOffset - m_scrollStep);
}

void ScrollViewer::LineDown() {
    SetVerticalOffset(m_verticalOffset + m_scrollStep);
}

void ScrollViewer::PageUp() {
    SetVerticalOffset(m_verticalOffset - m_viewportHeight);
}

void ScrollViewer::PageDown() {
    SetVerticalOffset(m_verticalOffset + m_viewportHeight);
}

core::Size ScrollViewer::MeasureOverride(core::Size availableSize) {
    if (m_content) {
        core::Size unconstrainedSize(availableSize.width, 10000000.0f);
        m_content->Measure(unconstrainedSize);
        core::Size contentDesired = m_content->GetDesiredSize();
        m_extentWidth = contentDesired.width;
        m_extentHeight = contentDesired.height;
    } else {
        m_extentWidth = 0.0f;
        m_extentHeight = 0.0f;
    }

    float w = (m_width >= 0.0f) ? m_width : (std::min)(availableSize.width, m_extentWidth);
    float h = (m_height >= 0.0f) ? m_height : (std::min)(availableSize.height, m_extentHeight);
    return core::Size(w, h);
}

void ScrollViewer::ArrangeOverride(core::Size finalSize) {
    m_viewportWidth = finalSize.width;
    m_viewportHeight = finalSize.height;

    float maxOffsetY = (std::max)(0.0f, m_extentHeight - m_viewportHeight);
    m_verticalOffset = compat::clamp(m_verticalOffset, 0.0f, maxOffsetY);

    float maxOffsetX = (std::max)(0.0f, m_extentWidth - m_viewportWidth);
    m_horizontalOffset = compat::clamp(m_horizontalOffset, 0.0f, maxOffsetX);

    if (m_content) {
        bool needsScrollbar = (m_extentHeight > m_viewportHeight);
        float baseW = needsScrollbar
            ? (std::max)(0.0f, finalSize.width - m_scrollBarWidth)
            : finalSize.width;
        float contentW = (std::max)(m_extentWidth, baseW);
        float contentH = (std::max)(m_extentHeight, finalSize.height);

        m_content->Arrange(core::Rect(-m_horizontalOffset, -m_verticalOffset, contentW, contentH));
    }
}

core::Rect ScrollViewer::GetTrackRect() const noexcept {
    return core::Rect(m_viewportWidth - m_scrollBarWidth, 0.0f, m_scrollBarWidth, m_viewportHeight);
}

core::Rect ScrollViewer::GetThumbRect() const noexcept {
    float maxOffset = (std::max)(0.0f, m_extentHeight - m_viewportHeight);
    if (maxOffset <= 0.0f || m_viewportHeight <= 0.0f) {
        return core::Rect(0.0f, 0.0f, 0.0f, 0.0f);
    }

    float thumbH = (std::max)(24.0f, m_viewportHeight * (m_viewportHeight / m_extentHeight));
    float trackScrollable = m_viewportHeight - thumbH;
    float thumbY = (trackScrollable > 0.0f && maxOffset > 0.0f)
        ? (m_verticalOffset / maxOffset) * trackScrollable
        : 0.0f;

    return core::Rect(
        m_viewportWidth - m_scrollBarWidth + 1.0f,
        thumbY + 1.0f,
        (std::max)(2.0f, m_scrollBarWidth - 2.0f),
        (std::max)(4.0f, thumbH - 2.0f)
    );
}

void ScrollViewer::OnRender(rendering::DrawingContext& dc) {
    ContentControl::OnRender(dc);
}

void ScrollViewer::OnPostRender(rendering::DrawingContext& dc) {
    if (m_extentHeight > m_viewportHeight && m_viewportHeight > 0.0f) {
        // 繪製卷軸軌道
        dc.DrawRectangle(GetTrackRect(), m_trackColor);

        // 繪製卷軸滑塊
        core::Color thumbClr = m_isDraggingThumb
            ? m_thumbPressedColor
            : (m_isThumbHovered ? m_thumbHoverColor : m_thumbColor);

        core::Rect thumbRect = GetThumbRect();
        float radius = (std::min)(thumbRect.width, thumbRect.height) * 0.5f;
        dc.DrawRoundedRectangle(thumbRect, radius, thumbClr);
    }
}

void ScrollViewer::ApplyTheme(const theme::Theme& theme) {
    ContentControl::ApplyTheme(theme);
    const auto& colors = theme.Colors();

    // 依據主題明暗模式微調卷軸色彩
    if (theme.GetMode() == theme::ThemeMode::Dark) {
        m_trackColor        = core::Color(25, 28, 38, 140);
        m_thumbColor        = core::Color(90, 105, 130, 180);
        m_thumbHoverColor   = core::Color(120, 140, 170, 220);
        m_thumbPressedColor = colors.accentPrimary;
    } else {
        m_trackColor        = core::Color(225, 230, 238, 160);
        m_thumbColor        = core::Color(160, 170, 185, 200);
        m_thumbHoverColor   = core::Color(130, 140, 160, 240);
        m_thumbPressedColor = colors.accentPrimary;
    }
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
