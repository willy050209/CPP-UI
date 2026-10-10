/**
 * @file RadioButton.cpp
 * @brief 單選按鈕控制項實作，支援群組互斥選取與向量繪製
 */

#include "ui/controls/RadioButton.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <algorithm>

namespace cppui {
namespace controls {

const input::RoutedEvent RadioButton::CheckedEvent(104, "Checked", input::RoutingStrategy::Bubble);
const input::RoutedEvent RadioButton::UncheckedEvent(105, "Unchecked", input::RoutingStrategy::Bubble);

namespace {

void CollectRadioButtons(core::Visual* root, std::vector<RadioButton*>& outList) {
    if (!root) return;
    if (auto* rb = dynamic_cast<RadioButton*>(root)) {
        outList.push_back(rb);
    }
    size_t count = root->GetVisualChildrenCount();
    for (size_t i = 0; i < count; ++i) {
        CollectRadioButtons(root->GetVisualChild(i), outList);
    }
}

} // namespace

RadioButton::RadioButton() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    SetPadding(core::Thickness(4.0f, 4.0f, 4.0f, 4.0f));
    SetBorderThickness(core::Thickness(0.0f));
    SetBackground(core::Color(0, 0, 0, 0));
    m_checkColor = theme.Colors().accentPrimary;

    OnClick([this]() {
        if (!m_isChecked) {
            SetChecked(true);
        }
    });
}

RadioButton::RadioButton(compat::string_view label) : RadioButton() {
    SetContent(label);
}

void RadioButton::SetChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        InvalidateVisual();

        if (m_isChecked) {
            ClearOtherRadioButtonsInGroup();
            input::RoutedEventArgs args(CheckedEvent, this);
            RaiseEvent(args);
        } else {
            input::RoutedEventArgs args(UncheckedEvent, this);
            RaiseEvent(args);
        }
    }
}

void RadioButton::SetGroupName(compat::string_view name) {
    m_groupName = std::string(name.data(), name.size());
}

void RadioButton::ClearOtherRadioButtonsInGroup() {
    core::Visual* scope = nullptr;
    if (!m_groupName.empty()) {
        const core::Visual* cur = this;
        while (cur->GetVisualParent()) {
            cur = cur->GetVisualParent();
        }
        scope = const_cast<core::Visual*>(cur);
    } else {
        scope = const_cast<core::Visual*>(GetVisualParent());
    }

    if (!scope) return;

    std::vector<RadioButton*> list;
    CollectRadioButtons(scope, list);

    for (auto* rb : list) {
        if (rb != this) {
            if (m_groupName.empty()) {
                if (rb->GetGroupName().empty() && rb->IsChecked()) {
                    rb->SetChecked(false);
                }
            } else {
                if (rb->GetGroupName() == m_groupName && rb->IsChecked()) {
                    rb->SetChecked(false);
                }
            }
        }
    }
}

void RadioButton::OnCheckedChanged(std::function<void(bool)> handler) {
    AddHandler(CheckedEvent, [handler](input::RoutedEventArgs&) {
        handler(true);
    });
    AddHandler(UncheckedEvent, [handler](input::RoutedEventArgs&) {
        handler(false);
    });
}

core::Size RadioButton::MeasureOverride(core::Size availableSize) {
    core::Size deflated = DeflateSize(availableSize);
    float circleDiameter = m_circleRadius * 2.0f;
    float totalWidth = circleDiameter;

    core::Size contentDesired(0.0f, 0.0f);
    if (m_content) {
        float availW = (std::max)(0.0f, deflated.width - (circleDiameter + m_boxSpacing));
        m_content->Measure(core::Size(availW, deflated.height));
        contentDesired = m_content->GetDesiredSize();
        totalWidth += m_boxSpacing + contentDesired.width;
    }

    float totalHeight = (std::max)(circleDiameter, contentDesired.height);
    return InflateSize(core::Size(totalWidth, totalHeight));
}

void RadioButton::ArrangeOverride(core::Size finalSize) {
    if (!m_content) return;

    float left = m_padding.left + m_borderThickness.left;
    float top = m_padding.top + m_borderThickness.top;
    float circleDiameter = m_circleRadius * 2.0f;
    float contentX = left + circleDiameter + m_boxSpacing;
    float contentW = (std::max)(0.0f, finalSize.width - (m_padding.Horizontal() + m_borderThickness.Horizontal() + circleDiameter + m_boxSpacing));
    float contentH = (std::max)(0.0f, finalSize.height - (m_padding.Vertical() + m_borderThickness.Vertical()));

    float contentY = top + (contentH - m_content->GetDesiredSize().height) * 0.5f;
    m_content->Arrange(core::Rect(contentX, contentY, contentW, m_content->GetDesiredSize().height));
}

void RadioButton::OnRender(rendering::DrawingContext& dc) {
    float left = m_padding.left + m_borderThickness.left;
    float top = m_padding.top + m_borderThickness.top;
    float h = GetRenderBounds().height - (m_padding.Vertical() + m_borderThickness.Vertical());
    float circleDiameter = m_circleRadius * 2.0f;
    float circleY = top + (h - circleDiameter) * 0.5f;

    core::Rect circleRect(left, circleY, circleDiameter, circleDiameter);

    if (m_isChecked) {
        // 選取態：外圈填深色，邊框使用 accentPrimary，中心實心圓
        dc.FillEllipse(circleRect, core::Color(25, 30, 42, 255));
        dc.DrawEllipse(circleRect, m_checkColor, 1.5f);

        float dotRadius = m_circleRadius * 0.5f;
        core::Rect dotRect(left + m_circleRadius - dotRadius, circleY + m_circleRadius - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
        dc.FillEllipse(dotRect, m_checkColor);
    } else {
        // 未選取態
        core::Color bg = m_isHovered ? core::Color(45, 52, 65, 255) : core::Color(30, 35, 45, 255);
        dc.FillEllipse(circleRect, bg);
        dc.DrawEllipse(circleRect, m_normalBorder, 1.5f);
    }

    if (IsFocused()) {
        core::Rect ring(circleRect.x - 2.0f, circleRect.y - 2.0f, circleRect.width + 4.0f, circleRect.height + 4.0f);
        dc.DrawEllipse(ring, m_focusBorder, 1.5f);
    }
}

void RadioButton::ApplyTheme(const theme::Theme& theme) {
    Button::ApplyTheme(theme);
    m_checkColor = theme.Colors().accentPrimary;
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
