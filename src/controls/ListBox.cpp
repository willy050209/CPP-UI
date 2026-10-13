/**
 * @file ListBox.cpp
 * @brief 清單方塊控制項實作
 */

#include "ui/controls/ListBox.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/theme/Theme.hpp"
#include <algorithm>
#include <stdexcept>

namespace cppui {
namespace controls {

const input::RoutedEvent ListBox::SelectedIndexChangedEvent(108, "SelectedIndexChanged", input::RoutingStrategy::Bubble);

ListBox::ListBox() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    SetBackground(theme.Colors().surfaceBackground);
    SetBorderBrush(theme.Colors().controlBorder);
    SetBorderThickness(core::Thickness(1.0f));
    SetPadding(core::Thickness(2.0f, 2.0f, 2.0f, 2.0f));
    SetCornerRadius(theme.Metrics().cornerRadiusSmall);
    SetForeground(theme.Colors().textPrimary);

    m_selectionColor = theme.Colors().accentPrimary;
    m_selectionTextColor = theme.Colors().textOnAccent;
    m_hasExplicitBackground = false;
    m_hasExplicitBorder = false;
    m_hasExplicitForeground = false;

    InitializeHandlers();
}

void ListBox::InitializeHandlers() {
    SetFocusable(true);
    SetTabStop(true);

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (!m_enabled || !e.IsLeftButton()) return;
        input::FocusManager::Get().SetFocus(this);

        core::Point pt = e.GetPosition(this);
        float topOffset = m_padding.top + m_borderThickness.top;
        float relY = pt.y - topOffset + m_scrollOffset;

        if (relY >= 0.0f) {
            int32_t idx = static_cast<int32_t>(relY / m_itemHeight);
            if (idx >= 0 && idx < static_cast<int32_t>(m_items.size())) {
                SetSelectedIndex(idx);
            }
        }
        e.SetHandled(true);
    });

    OnPointerWheel([this](input::PointerEventArgs& e) {
        if (!m_enabled) return;
        float delta = static_cast<float>(e.GetWheelDeltaY());
        if (delta != 0.0f) {
            SetScrollOffset(m_scrollOffset - delta * 0.5f);
            e.SetHandled(true);
        }
    });

    OnKeyDown([this](input::KeyEventArgs& e) {
        if (!m_enabled) return;
        input::Key key = e.GetKey();

        if (key == input::Key::Up) { // Up
            if (m_selectedIndex > 0) {
                SetSelectedIndex(m_selectedIndex - 1);
                ScrollIntoView(m_selectedIndex);
                e.SetHandled(true);
            } else if (m_selectedIndex == -1 && !m_items.empty()) {
                SetSelectedIndex(0);
                ScrollIntoView(0);
                e.SetHandled(true);
            }
        } else if (key == input::Key::Down) { // Down
            if (m_selectedIndex + 1 < static_cast<int32_t>(m_items.size())) {
                SetSelectedIndex(m_selectedIndex + 1);
                ScrollIntoView(m_selectedIndex);
                e.SetHandled(true);
            } else if (m_selectedIndex == -1 && !m_items.empty()) {
                SetSelectedIndex(0);
                ScrollIntoView(0);
                e.SetHandled(true);
            }
        } else if (key == input::Key::Home) { // Home
            if (!m_items.empty()) {
                SetSelectedIndex(0);
                ScrollIntoView(0);
                e.SetHandled(true);
            }
        } else if (key == input::Key::End) { // End
            if (!m_items.empty()) {
                SetSelectedIndex(static_cast<int32_t>(m_items.size() - 1));
                ScrollIntoView(m_selectedIndex);
                e.SetHandled(true);
            }
        }
    });
}

void ListBox::AddItem(compat::string_view item) {
    m_items.emplace_back(item.data(), item.size());
    InvalidateMeasure();
    InvalidateVisual();
}

void ListBox::InsertItem(size_t index, compat::string_view item) {
    if (index > m_items.size()) {
        throw std::out_of_range("ListBox::InsertItem: index out of range");
    }
    m_items.insert(m_items.begin() + index, std::string(item.data(), item.size()));
    if (m_selectedIndex >= static_cast<int32_t>(index)) {
        m_selectedIndex++;
    }
    InvalidateMeasure();
    InvalidateVisual();
}

void ListBox::RemoveItem(size_t index) {
    if (index >= m_items.size()) {
        throw std::out_of_range("ListBox::RemoveItem: index out of range");
    }
    m_items.erase(m_items.begin() + index);

    if (m_selectedIndex == static_cast<int32_t>(index)) {
        if (m_items.empty()) {
            m_selectedIndex = -1;
            RaiseSelectedIndexChanged();
        } else if (m_selectedIndex >= static_cast<int32_t>(m_items.size())) {
            m_selectedIndex = static_cast<int32_t>(m_items.size()) - 1;
            RaiseSelectedIndexChanged();
        } else {
            RaiseSelectedIndexChanged();
        }
    } else if (m_selectedIndex > static_cast<int32_t>(index)) {
        m_selectedIndex--;
    }

    InvalidateMeasure();
    InvalidateVisual();
}

void ListBox::ClearItems() {
    m_items.clear();
    if (m_selectedIndex != -1) {
        m_selectedIndex = -1;
        RaiseSelectedIndexChanged();
    }
    InvalidateMeasure();
    InvalidateVisual();
}

compat::string_view ListBox::GetItem(size_t index) const {
    if (index >= m_items.size()) {
        throw std::out_of_range("ListBox::GetItem: index out of range");
    }
    return m_items[index];
}

void ListBox::SetSelectedIndex(int32_t index) {
    if (index < -1 || index >= static_cast<int32_t>(m_items.size())) {
        throw std::out_of_range("ListBox::SetSelectedIndex: index out of range");
    }
    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        InvalidateVisual();
        RaiseSelectedIndexChanged();
    }
}

compat::string_view ListBox::GetSelectedItem() const noexcept {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int32_t>(m_items.size())) {
        return m_items[static_cast<size_t>(m_selectedIndex)];
    }
    return compat::string_view{};
}

void ListBox::SetSelectedItem(compat::string_view item) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i] == item) {
            SetSelectedIndex(static_cast<int32_t>(i));
            return;
        }
    }
    SetSelectedIndex(-1);
}

void ListBox::SetItemHeight(float height) {
    if (height <= 0.0f) {
        throw std::invalid_argument("ListBox::SetItemHeight: height must be positive");
    }
    if (m_itemHeight != height) {
        m_itemHeight = height;
        InvalidateMeasure();
        InvalidateVisual();
    }
}

void ListBox::SetVirtualizationEnabled(bool enable) {
    if (m_virtualizationEnabled != enable) {
        m_virtualizationEnabled = enable;
        InvalidateVisual();
    }
}

void ListBox::SetScrollOffset(float offset) {
    float contentH = (std::max)(0.0f, GetRenderBounds().height - (m_padding.Vertical() + m_borderThickness.Vertical()));
    float maxScroll = (std::max)(0.0f, static_cast<float>(m_items.size()) * m_itemHeight - contentH);
    float clamped = compat::clamp(offset, 0.0f, maxScroll);
    if (m_scrollOffset != clamped) {
        m_scrollOffset = clamped;
        InvalidateVisual();
    }
}

void ListBox::ScrollIntoView(int32_t index) {
    if (index < 0 || index >= static_cast<int32_t>(m_items.size())) return;
    float itemTop = static_cast<float>(index) * m_itemHeight;
    float itemBottom = itemTop + m_itemHeight;
    float viewH = GetRenderBounds().height - (m_padding.Vertical() + m_borderThickness.Vertical());
    if (viewH <= 0.0f) viewH = 200.0f;

    if (itemTop < m_scrollOffset) {
        SetScrollOffset(itemTop);
    } else if (itemBottom > m_scrollOffset + viewH) {
        SetScrollOffset(itemBottom - viewH);
    }
}

void ListBox::OnSelectedIndexChanged(std::function<void(int32_t)> handler) {
    AddHandler(SelectedIndexChangedEvent, [handler, this](input::RoutedEventArgs&) {
        if (handler) {
            handler(m_selectedIndex);
        }
    });
}

void ListBox::RaiseSelectedIndexChanged() {
    input::RoutedEventArgs args(SelectedIndexChangedEvent, this);
    RaiseEvent(args);
}

core::Size ListBox::MeasureOverride(core::Size availableSize) {
    (void)availableSize;
    float width = 120.0f;
    float height = m_padding.Vertical() + m_borderThickness.Vertical() + m_itemHeight * static_cast<float>(m_items.size());
    return core::Size(width, (std::max)(24.0f, height));
}

void ListBox::OnRender(rendering::DrawingContext& dc) {
    core::Rect bounds(core::Point(0, 0), GetRenderBounds().GetSize());
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) return;

    if (m_cornerRadius > 0.0f) {
        dc.DrawRoundedRectangle(bounds, m_cornerRadius, m_background);
    } else {
        dc.DrawRectangle(bounds, m_background);
    }

    if (m_borderBrush.a > 0 && m_borderThickness.top > 0.0f) {
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.Right(), bounds.y), m_borderBrush, m_borderThickness.top);
        dc.DrawLine(core::Point(bounds.x, bounds.Bottom()), core::Point(bounds.Right(), bounds.Bottom()), m_borderBrush, m_borderThickness.bottom);
        dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.x, bounds.Bottom()), m_borderBrush, m_borderThickness.left);
        dc.DrawLine(core::Point(bounds.Right(), bounds.y), core::Point(bounds.Right(), bounds.Bottom()), m_borderBrush, m_borderThickness.right);
    }

    float contentX = bounds.x + m_padding.left + m_borderThickness.left;
    float contentY = bounds.y + m_padding.top + m_borderThickness.top;
    float contentW = (std::max)(0.0f, bounds.width - (m_padding.Horizontal() + m_borderThickness.Horizontal()));
    float contentH = (std::max)(0.0f, bounds.height - (m_padding.Vertical() + m_borderThickness.Vertical()));

    if (contentW <= 0.0f || contentH <= 0.0f) return;

    dc.PushClip(core::Rect(contentX, contentY, contentW, contentH));

    float maxScroll = (std::max)(0.0f, static_cast<float>(m_items.size()) * m_itemHeight - contentH);
    m_scrollOffset = compat::clamp(m_scrollOffset, 0.0f, maxScroll);

    size_t first = 0;
    size_t last = m_items.size();
    if (m_virtualizationEnabled && m_itemHeight > 0.0f) {
        first = static_cast<size_t>(m_scrollOffset / m_itemHeight);
        size_t visibleCount = static_cast<size_t>(std::ceil(contentH / m_itemHeight)) + 2;
        last = (std::min)(m_items.size(), first + visibleCount);
    }

    for (size_t i = first; i < last; ++i) {
        float itemY = contentY + static_cast<float>(i) * m_itemHeight - m_scrollOffset;
        if (itemY + m_itemHeight < contentY || itemY > contentY + contentH) {
            continue;
        }

        core::Rect itemRect(contentX, itemY, contentW, m_itemHeight);
        if (static_cast<int32_t>(i) == m_selectedIndex) {
            dc.DrawRectangle(itemRect, m_selectionColor);
        }

        core::Color textColor = (static_cast<int32_t>(i) == m_selectedIndex) ? m_selectionTextColor : m_foreground;
        float textY = itemY + (m_itemHeight - GetFont().GetSize() * 1.25f) * 0.5f;
        dc.DrawText(m_items[i], core::Point(contentX + 4.0f, textY), GetFont(), textColor);
    }

    dc.PopClip();
}

void ListBox::ApplyTheme(const theme::Theme& theme) {
    Control::ApplyTheme(theme);
    if (!m_hasExplicitBackground) {
        m_background = theme.Colors().surfaceBackground;
    }
    if (!m_hasExplicitBorder) {
        m_borderBrush = theme.Colors().controlBorder;
    }
    m_selectionColor = theme.Colors().accentPrimary;
    m_selectionTextColor = theme.Colors().textOnAccent;
    InvalidateVisual();
}

} // namespace controls
} // namespace cppui
