/*
 * File: MenuItem.cpp
 * Purpose: Implements MenuItem control with submenu and command support.
 */

#include "ui/controls/MenuItem.hpp"
#include "ui/controls/Popup.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/theme/ThemeManager.hpp"
#include <stdexcept>
#include <algorithm>

namespace cppui {
namespace controls {

MenuItem::MenuItem() {
    SetPadding(core::Thickness(12.0f, 5.0f, 12.0f, 5.0f));
    SetForeground(core::Color(220, 225, 235, 255));
    SetHitTestVisible(true);

    OnPointerEntered([this](input::PointerEventArgs&) {
        SetHighlighted(true);
        if (HasSubmenu()) {
            OpenSubmenu();
        }
    });

    OnPointerExited([this](input::PointerEventArgs&) {
        SetHighlighted(false);
    });

    OnPointerPressed([this](input::PointerEventArgs& e) {
        if (HasSubmenu()) {
            OpenSubmenu();
        } else {
            TriggerClick();
        }
        e.SetHandled(true);
    });
}

MenuItem::MenuItem(compat::string_view header) : MenuItem() {
    SetHeader(header);
}

MenuItem::~MenuItem() {
    CloseSubmenu();
}

void MenuItem::SetHeader(compat::string_view header) {
    m_header = std::string(header.data(), header.size());
    InvalidateMeasure();
    InvalidateVisual();
}

void MenuItem::SetInputGestureText(compat::string_view text) {
    m_inputGestureText = std::string(text.data(), text.size());
    InvalidateMeasure();
    InvalidateVisual();
}

void MenuItem::SetCommand(std::shared_ptr<mvvm::ICommand> command) {
    m_command = std::move(command);
}

void MenuItem::AddMenuItem(std::shared_ptr<MenuItem> item) {
    if (!item) {
        throw std::invalid_argument("MenuItem::AddMenuItem: item cannot be nullptr.");
    }
    m_submenuItems.push_back(std::move(item));
}

void MenuItem::RemoveMenuItem(const std::shared_ptr<MenuItem>& item) {
    if (!item) return;
    auto it = std::find(m_submenuItems.begin(), m_submenuItems.end(), item);
    if (it != m_submenuItems.end()) {
        m_submenuItems.erase(it);
    }
}

void MenuItem::SetHighlighted(bool value) {
    if (m_isHighlighted != value) {
        m_isHighlighted = value;
        InvalidateVisual();
    }
}

void MenuItem::OpenSubmenu() {
    if (!HasSubmenu()) return;

    if (!m_submenuPopup) {
        auto panel = std::make_shared<layout::StackPanel>();
        panel->SetOrientation(layout::Orientation::Vertical);
        for (const auto& sub : m_submenuItems) {
            panel->AddChild(sub);
        }

        auto border = std::make_shared<layout::Border>();
        border->SetBorderThickness(core::Thickness(1.0f));
        border->SetCornerRadius(4.0f);
        border->SetBackground(core::Color(38, 42, 50, 255));
        border->SetBorderBrush(core::Color(65, 72, 84, 255));
        border->SetPadding(core::Thickness(2.0f));
        border->SetChild(panel);

        m_submenuPopup = std::make_shared<Popup>(border);
        m_submenuPopup->SetPlacementTarget(this);
        m_submenuPopup->SetPlacementMode(PlacementMode::Right);
        m_submenuPopup->SetStaysOpen(false);
    }

    m_submenuPopup->SetIsOpen(true);
}

void MenuItem::CloseSubmenu() {
    if (m_submenuPopup && m_submenuPopup->IsOpen()) {
        m_submenuPopup->Close();
    }
}

void MenuItem::AddClickHandler(ClickHandler handler) {
    if (handler) {
        m_clickHandlers.push_back(std::move(handler));
    }
}

void MenuItem::TriggerClick() {
    for (const auto& handler : m_clickHandlers) {
        if (handler) {
            handler(*this);
        }
    }

    if (m_command) {
        m_command->Execute();
    }

    // 點擊葉節點項目後關閉頂層所屬 Popup
    core::Visual* cur = this;
    while (cur) {
        if (auto* popup = dynamic_cast<Popup*>(cur)) {
            popup->Close();
            break;
        }
        cur = cur->GetVisualParent();
    }
}

core::Size MenuItem::MeasureOverride(core::Size availableSize) {
    (void)availableSize;
    float textWidth = static_cast<float>(m_header.size() * 9);
    float gestureWidth = static_cast<float>(m_inputGestureText.size() * 8);
    float totalWidth = textWidth + gestureWidth + m_padding.left + m_padding.right + (HasSubmenu() ? 24.0f : 16.0f);
    float minWidth = 140.0f;
    return core::Size((std::max)(minWidth, totalWidth), 28.0f);
}

void MenuItem::OnRender(rendering::DrawingContext& dc) {
    core::Rect rect(0.0f, 0.0f, m_visualBounds.width, m_visualBounds.height);

    if (m_isHighlighted) {
        // 反白背景繪製
        dc.DrawRectangle(rect, core::Color(65, 105, 175, 255));
    }

    text::FontDescriptor font = GetFont();

    // 標題文字
    if (!m_header.empty()) {
        dc.DrawText(m_header, core::Point(m_padding.left, m_padding.top + 2.0f), font, m_foreground);
    }

    // 快捷鍵文字
    if (!m_inputGestureText.empty()) {
        float gestureX = m_visualBounds.width - m_padding.right - static_cast<float>(m_inputGestureText.size() * 8) - (HasSubmenu() ? 16.0f : 0.0f);
        dc.DrawText(m_inputGestureText, core::Point(gestureX, m_padding.top + 2.0f), font, core::Color(150, 160, 175, 255));
    }

    // 次級選單箭頭標記
    if (HasSubmenu()) {
        float arrowX = m_visualBounds.width - m_padding.right - 8.0f;
        dc.DrawText(">", core::Point(arrowX, m_padding.top + 2.0f), font, m_foreground);
    }
}

} // namespace controls
} // namespace cppui
