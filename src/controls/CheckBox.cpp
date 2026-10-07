#include "ui/controls/CheckBox.hpp"
#include <algorithm>

namespace cppui {
namespace controls {

const input::RoutedEvent CheckBox::CheckedEvent(101, "Checked", input::RoutingStrategy::Bubble);
const input::RoutedEvent CheckBox::UncheckedEvent(102, "Unchecked", input::RoutingStrategy::Bubble);

CheckBox::CheckBox() {
    SetPadding(core::Thickness(4.0f, 4.0f, 4.0f, 4.0f));
    SetBorderThickness(core::Thickness(0.0f));
    SetBackground(core::Color(0, 0, 0, 0));
    SetCornerRadius(3.0f);

    // 點擊時切換打勾狀態
    OnClick([this]() {
        Toggle();
    });
}

CheckBox::CheckBox(compat::string_view label) : CheckBox() {
    SetContent(label);
}

void CheckBox::SetChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        InvalidateVisual();

        input::RoutedEventArgs args(m_isChecked ? CheckedEvent : UncheckedEvent, this);
        RaiseEvent(args);
    }
}

void CheckBox::Toggle() {
    SetChecked(!m_isChecked);
}

void CheckBox::OnCheckedChanged(std::function<void(bool)> handler) {
    AddHandler(CheckedEvent, [handler](input::RoutedEventArgs&) {
        handler(true);
    });
    AddHandler(UncheckedEvent, [handler](input::RoutedEventArgs&) {
        handler(false);
    });
}

core::Size CheckBox::MeasureOverride(core::Size availableSize) {
    core::Size deflated = DeflateSize(availableSize);
    float boxTotalWidth = m_boxSize;

    core::Size contentDesired(0.0f, 0.0f);
    if (m_content) {
        float availW = (std::max)(0.0f, deflated.width - (m_boxSize + m_boxSpacing));
        m_content->Measure(core::Size(availW, deflated.height));
        contentDesired = m_content->GetDesiredSize();
        boxTotalWidth += m_boxSpacing + contentDesired.width;
    }

    float totalHeight = (std::max)(m_boxSize, contentDesired.height);
    return InflateSize(core::Size(boxTotalWidth, totalHeight));
}

void CheckBox::ArrangeOverride(core::Size finalSize) {
    if (!m_content) return;

    float left = m_padding.left + m_borderThickness.left;
    float top = m_padding.top + m_borderThickness.top;
    float contentX = left + m_boxSize + m_boxSpacing;
    float contentW = (std::max)(0.0f, finalSize.width - (m_padding.Horizontal() + m_borderThickness.Horizontal() + m_boxSize + m_boxSpacing));
    float contentH = (std::max)(0.0f, finalSize.height - (m_padding.Vertical() + m_borderThickness.Vertical()));

    // 垂直置中配置標籤
    float contentY = top + (contentH - m_content->GetDesiredSize().height) * 0.5f;
    m_content->Arrange(core::Rect(contentX, contentY, contentW, m_content->GetDesiredSize().height));
}

void CheckBox::OnRender(rendering::DrawingContext& dc) {
    float left = m_padding.left + m_borderThickness.left;
    float top = m_padding.top + m_borderThickness.top;
    float h = GetRenderBounds().height - (m_padding.Vertical() + m_borderThickness.Vertical());
    float boxY = top + (h - m_boxSize) * 0.5f;

    core::Rect boxRect(left, boxY, m_boxSize, m_boxSize);

    // 繪製方框
    if (m_isChecked) {
        // 勾選態：藍色填色
        dc.DrawRoundedRectangle(boxRect, m_cornerRadius, m_checkColor);
        // 白色向量勾號
        dc.DrawLine(core::Point(left + 4.0f, boxY + 9.5f), core::Point(left + 7.5f, boxY + 13.5f), core::Color(255, 255, 255, 255), 2.0f);
        dc.DrawLine(core::Point(left + 7.5f, boxY + 13.5f), core::Point(left + 14.0f, boxY + 5.0f), core::Color(255, 255, 255, 255), 2.0f);
    } else {
        // 未勾選態：深色底 + 灰色框
        core::Color bg = m_isHovered ? core::Color(45, 52, 65, 255) : core::Color(30, 35, 45, 255);
        dc.DrawRoundedRectangle(boxRect, m_cornerRadius, bg);
        dc.DrawLine(core::Point(boxRect.x, boxRect.y), core::Point(boxRect.Right(), boxRect.y), m_normalBorder, 1.5f);
        dc.DrawLine(core::Point(boxRect.x, boxRect.Bottom()), core::Point(boxRect.Right(), boxRect.Bottom()), m_normalBorder, 1.5f);
        dc.DrawLine(core::Point(boxRect.x, boxRect.y), core::Point(boxRect.x, boxRect.Bottom()), m_normalBorder, 1.5f);
        dc.DrawLine(core::Point(boxRect.Right(), boxRect.y), core::Point(boxRect.Right(), boxRect.Bottom()), m_normalBorder, 1.5f);
    }

    // 獲焦外框光暈
    if (IsFocused()) {
        core::Rect ring(boxRect.x - 2.0f, boxRect.y - 2.0f, boxRect.width + 4.0f, boxRect.height + 4.0f);
        dc.DrawRoundedRectangle(ring, m_cornerRadius + 2.0f, m_focusBorder);
        if (m_isChecked) {
            dc.DrawRoundedRectangle(boxRect, m_cornerRadius, m_checkColor);
            dc.DrawLine(core::Point(left + 4.0f, boxY + 9.5f), core::Point(left + 7.5f, boxY + 13.5f), core::Color(255, 255, 255, 255), 2.0f);
            dc.DrawLine(core::Point(left + 7.5f, boxY + 13.5f), core::Point(left + 14.0f, boxY + 5.0f), core::Color(255, 255, 255, 255), 2.0f);
        } else {
            core::Color bg = m_isHovered ? core::Color(45, 52, 65, 255) : core::Color(30, 35, 45, 255);
            dc.DrawRoundedRectangle(boxRect, m_cornerRadius, bg);
        }
    }
}

} // namespace controls
} // namespace cppui
