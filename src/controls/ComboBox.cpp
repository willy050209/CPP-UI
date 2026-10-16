/*
 * File: ComboBox.cpp
 * Purpose: Implements ComboBox dropdown selection control.
 */

#include "ui/controls/ComboBox.hpp"
#include "ui/controls/MenuItem.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include <algorithm>
#include <stdexcept>

namespace cppui {
namespace controls {

ComboBox::ComboBox() {
    SetHitTestVisible(true);
    SetPadding(core::Thickness(10.0f, 4.0f, 10.0f, 4.0f));
    SetBorderThickness(core::Thickness(1.0f));
    SetCornerRadius(4.0f);
    SetBackground(core::Color(42, 46, 54, 255));
    SetBorderBrush(core::Color(72, 80, 94, 255));
    SetForeground(core::Color(230, 235, 245, 255));

    m_dropdownPanel = std::make_shared<layout::StackPanel>();
    m_dropdownPanel->SetOrientation(layout::Orientation::Vertical);

    auto border = std::make_shared<layout::Border>();
    border->SetBorderThickness(core::Thickness(1.0f));
    border->SetCornerRadius(4.0f);
    border->SetBackground(core::Color(36, 40, 48, 255));
    border->SetBorderBrush(core::Color(75, 85, 100, 255));
    border->SetPadding(core::Thickness(2.0f));
    border->SetChild(m_dropdownPanel);

    m_dropdownPopup = std::make_shared<Popup>(border);
    m_dropdownPopup->SetPlacementTarget(this);
    m_dropdownPopup->SetPlacementMode(PlacementMode::Bottom);
    m_dropdownPopup->SetStaysOpen(false);

    OnPointerEntered([this](input::PointerEventArgs&) {
        m_isHovered = true;
        InvalidateVisual();
    });

    OnPointerExited([this](input::PointerEventArgs&) {
        m_isHovered = false;
        InvalidateVisual();
    });

    OnPointerPressed([this](input::PointerEventArgs& e) {
        SetIsDropDownOpen(!IsDropDownOpen());
        e.SetHandled(true);
    });
}

ComboBox::~ComboBox() {
    if (m_dropdownPopup && m_dropdownPopup->IsOpen()) {
        m_dropdownPopup->Close();
    }
}

void ComboBox::AddItem(compat::string_view text) {
    m_items.emplace_back(text.data(), text.size());
    if (m_selectedIndex == -1 && !m_items.empty()) {
        m_selectedIndex = 0;
    }
    RebuildDropdownItems();
    InvalidateMeasure();
    InvalidateVisual();
}

void ComboBox::InsertItem(size_t index, compat::string_view text) {
    if (index > m_items.size()) {
        index = m_items.size();
    }
    m_items.emplace(m_items.begin() + static_cast<ptrdiff_t>(index), text.data(), text.size());
    if (m_selectedIndex == -1 && !m_items.empty()) {
        m_selectedIndex = 0;
    }
    RebuildDropdownItems();
    InvalidateMeasure();
    InvalidateVisual();
}

void ComboBox::RemoveItem(size_t index) {
    if (index >= m_items.size()) return;
    m_items.erase(m_items.begin() + static_cast<ptrdiff_t>(index));
    if (m_selectedIndex >= static_cast<int32_t>(m_items.size())) {
        m_selectedIndex = static_cast<int32_t>(m_items.size()) - 1;
    }
    RebuildDropdownItems();
    InvalidateMeasure();
    InvalidateVisual();
}

void ComboBox::ClearItems() {
    m_items.clear();
    m_selectedIndex = -1;
    RebuildDropdownItems();
    InvalidateMeasure();
    InvalidateVisual();
}

compat::string_view ComboBox::GetItem(size_t index) const {
    if (index >= m_items.size()) {
        return {};
    }
    return m_items[index];
}

void ComboBox::SetSelectedIndex(int32_t index) {
    if (index < -1 || (index >= 0 && static_cast<size_t>(index) >= m_items.size())) {
        return;
    }
    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        InvalidateVisual();
        for (const auto& handler : m_selectionHandlers) {
            if (handler) {
                handler(*this, m_selectedIndex);
            }
        }
    }
}

compat::string_view ComboBox::GetSelectedItem() const {
    if (m_selectedIndex >= 0 && static_cast<size_t>(m_selectedIndex) < m_items.size()) {
        return m_items[static_cast<size_t>(m_selectedIndex)];
    }
    return {};
}

bool ComboBox::IsDropDownOpen() const noexcept {
    return m_dropdownPopup && m_dropdownPopup->IsOpen();
}

void ComboBox::SetIsDropDownOpen(bool open) {
    if (!m_dropdownPopup) return;
    if (m_dropdownPopup->IsOpen() != open) {
        if (open) {
            RebuildDropdownItems();
            float minWidth = (std::max)(m_visualBounds.width, 140.0f);
            m_dropdownPopup->SetWidth(minWidth);
        }
        m_dropdownPopup->SetIsOpen(open);
        InvalidateVisual();
    }
}

void ComboBox::AddSelectionChangedHandler(SelectionChangedHandler handler) {
    if (handler) {
        m_selectionHandlers.push_back(std::move(handler));
    }
}

void ComboBox::RebuildDropdownItems() {
    if (!m_dropdownPanel) return;
    m_dropdownPanel->ClearChildren();

    auto token = GetLifetimeToken();
    for (size_t i = 0; i < m_items.size(); ++i) {
        auto item = std::make_shared<MenuItem>(m_items[i]);
        int32_t idx = static_cast<int32_t>(i);
        item->AddClickHandler([this, idx, token](MenuItem&) {
            if (!token.IsAlive()) return;
            SetSelectedIndex(idx);
            SetIsDropDownOpen(false);
        });
        m_dropdownPanel->AddChild(std::move(item));
    }
}

core::Size ComboBox::MeasureOverride(core::Size availableSize) {
    (void)availableSize;
    float w = GetWidth() > 0.0f ? GetWidth() : 140.0f;
    float h = GetHeight() > 0.0f ? GetHeight() : 30.0f;
    return core::Size(w, h);
}

void ComboBox::OnRender(rendering::DrawingContext& dc) {
    core::Rect rect(0.0f, 0.0f, m_visualBounds.width, m_visualBounds.height);

    // 框體底色與邊框
    dc.DrawRectangle(rect, m_background);
    core::Color borderColor = m_isHovered ? core::Color(100, 140, 210, 255) : m_borderBrush;
    if (borderColor.a > 0 && m_borderThickness.top > 0.0f) {
        dc.DrawLine(core::Point(rect.x, rect.y), core::Point(rect.Right(), rect.y), borderColor, m_borderThickness.top);
        dc.DrawLine(core::Point(rect.x, rect.Bottom()), core::Point(rect.Right(), rect.Bottom()), borderColor, m_borderThickness.bottom);
        dc.DrawLine(core::Point(rect.x, rect.y), core::Point(rect.x, rect.Bottom()), borderColor, m_borderThickness.left);
        dc.DrawLine(core::Point(rect.Right(), rect.y), core::Point(rect.Right(), rect.Bottom()), borderColor, m_borderThickness.right);
    }

    text::FontDescriptor font = GetFont();

    // 當前選中文字
    compat::string_view selected = GetSelectedItem();
    if (!selected.empty()) {
        dc.DrawText(selected, core::Point(m_padding.left, m_padding.top + 2.0f), font, m_foreground);
    } else {
        dc.DrawText("(Select...)", core::Point(m_padding.left, m_padding.top + 2.0f), font, core::Color(140, 150, 165, 255));
    }

    // 下拉倒三角箭頭標記
    float arrowX = m_visualBounds.width - m_padding.right - 10.0f;
    dc.DrawText("v", core::Point(arrowX, m_padding.top + 2.0f), font, m_foreground);
}

} // namespace controls
} // namespace cppui
