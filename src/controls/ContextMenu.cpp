/*
 * File: ContextMenu.cpp
 * Purpose: Implements ContextMenu and ContextMenuService.
 */

#include "ui/controls/ContextMenu.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include <unordered_map>
#include <stdexcept>
#include <algorithm>

namespace cppui {
namespace controls {

ContextMenu::ContextMenu() {
    m_panel = std::make_shared<layout::StackPanel>();
    m_panel->SetOrientation(layout::Orientation::Vertical);

    auto border = std::make_shared<layout::Border>();
    border->SetBorderThickness(core::Thickness(1.0f));
    border->SetCornerRadius(4.0f);
    border->SetBackground(core::Color(38, 42, 50, 255));
    border->SetBorderBrush(core::Color(65, 72, 84, 255));
    border->SetPadding(core::Thickness(2.0f));
    border->SetChild(m_panel);

    SetContent(border);
    SetStaysOpen(false);
}

void ContextMenu::AddMenuItem(std::shared_ptr<MenuItem> item) {
    if (!item) {
        throw std::invalid_argument("ContextMenu::AddMenuItem: item cannot be nullptr.");
    }
    m_items.push_back(item);
    m_panel->AddChild(std::move(item));
}

void ContextMenu::RemoveMenuItem(const std::shared_ptr<MenuItem>& item) {
    if (!item) return;
    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        m_items.erase(it);
        m_panel->RemoveChild(item);
    }
}

void ContextMenu::ClearMenuItems() {
    m_items.clear();
    m_panel->ClearChildren();
}

size_t ContextMenu::GetMenuItemCount() const noexcept {
    return m_items.size();
}

std::shared_ptr<MenuItem> ContextMenu::GetMenuItem(size_t index) const {
    if (index >= m_items.size()) {
        return nullptr;
    }
    return m_items[index];
}

void ContextMenu::Show(core::UIElement* target, core::Point relativePos) {
    SetPlacementTarget(target);
    SetPlacementMode(PlacementMode::Custom);
    SetOffset(relativePos);
    SetIsOpen(true);
}

void ContextMenu::Close() {
    SetIsOpen(false);
}

namespace {
static std::unordered_map<core::UIElement*, std::shared_ptr<ContextMenu>>& GetContextMenuRegistry() {
    static std::unordered_map<core::UIElement*, std::shared_ptr<ContextMenu>> s_registry;
    return s_registry;
}
} // anonymous namespace

void ContextMenuService::SetContextMenu(core::UIElement* element, std::shared_ptr<ContextMenu> menu) {
    if (!element) {
        throw std::invalid_argument("ContextMenuService::SetContextMenu: element cannot be nullptr.");
    }
    if (!menu) {
        GetContextMenuRegistry().erase(element);
        return;
    }

    GetContextMenuRegistry()[element] = menu;
    auto token = element->GetLifetimeToken();
    element->OnPointerReleased([element, menu, token](input::PointerEventArgs& e) {
        if (!token.IsAlive()) return;
        if (e.GetButton() == pal::RawMouseButton::Right) {
            menu->Show(element, e.GetPosition(element));
            e.SetHandled(true);
        }
    });
}

std::shared_ptr<ContextMenu> ContextMenuService::GetContextMenu(core::UIElement* element) {
    if (!element) return nullptr;
    auto& reg = GetContextMenuRegistry();
    auto it = reg.find(element);
    if (it != reg.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace controls
} // namespace cppui
