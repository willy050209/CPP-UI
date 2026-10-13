/*
 * File: ToolTip.cpp
 * Purpose: Implements ToolTip and ToolTipService.
 */

#include "ui/controls/ToolTip.hpp"
#include "ui/controls/ToolTipService.hpp"
#include "ui/controls/Popup.hpp"
#include "ui/theme/ThemeManager.hpp"
#include <unordered_map>
#include <stdexcept>

namespace cppui {
namespace controls {

ToolTip::ToolTip() {
    SetPadding(core::Thickness(8.0f, 4.0f, 8.0f, 4.0f));
    SetBorderThickness(core::Thickness(1.0f));
    SetCornerRadius(4.0f);
    SetHitTestVisible(false); // 提示標籤不阻擋游標交互

    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    ApplyTheme(theme);
}

ToolTip::ToolTip(compat::string_view text) : ToolTip() {
    SetContent(text);
}

void ToolTip::ApplyTheme(const theme::Theme& theme) {
    ContentControl::ApplyTheme(theme);
    SetBackground(core::Color(35, 39, 46, 240));
    SetBorderBrush(core::Color(70, 76, 88, 255));
    SetForeground(core::Color(240, 243, 246, 255));
}

namespace {
struct ToolTipEntry {
    std::shared_ptr<ToolTip> toolTip;
    std::shared_ptr<Popup> popup;
};

static std::unordered_map<core::UIElement*, ToolTipEntry>& GetRegistry() {
    static std::unordered_map<core::UIElement*, ToolTipEntry> s_registry;
    return s_registry;
}
} // anonymous namespace

void ToolTipService::SetToolTip(core::UIElement* element, compat::string_view text) {
    if (!element) {
        throw std::invalid_argument("ToolTipService::SetToolTip: element cannot be nullptr.");
    }
    SetToolTip(element, std::make_shared<ToolTip>(text));
}

void ToolTipService::SetToolTip(core::UIElement* element, std::shared_ptr<ToolTip> toolTip) {
    if (!element) {
        throw std::invalid_argument("ToolTipService::SetToolTip: element cannot be nullptr.");
    }
    if (!toolTip) {
        HideToolTip(element);
        GetRegistry().erase(element);
        return;
    }

    auto popup = std::make_shared<Popup>(toolTip);
    popup->SetPlacementTarget(element);
    popup->SetPlacementMode(PlacementMode::Bottom);
    popup->SetOffset(core::Point(0.0f, 4.0f));
    popup->SetStaysOpen(false);

    GetRegistry()[element] = ToolTipEntry{ toolTip, popup };

    element->OnPointerEntered([element](input::PointerEventArgs&) {
        ShowToolTip(element);
    });
    element->OnPointerExited([element](input::PointerEventArgs&) {
        HideToolTip(element);
    });
    element->OnPointerPressed([element](input::PointerEventArgs&) {
        HideToolTip(element);
    });
}

std::shared_ptr<ToolTip> ToolTipService::GetToolTip(core::UIElement* element) {
    if (!element) return nullptr;
    auto& reg = GetRegistry();
    auto it = reg.find(element);
    if (it != reg.end()) {
        return it->second.toolTip;
    }
    return nullptr;
}

void ToolTipService::ShowToolTip(core::UIElement* element) {
    if (!element) return;
    auto& reg = GetRegistry();
    auto it = reg.find(element);
    if (it != reg.end() && it->second.popup) {
        it->second.popup->SetIsOpen(true);
    }
}

void ToolTipService::HideToolTip(core::UIElement* element) {
    if (!element) return;
    auto& reg = GetRegistry();
    auto it = reg.find(element);
    if (it != reg.end() && it->second.popup) {
        it->second.popup->SetIsOpen(false);
    }
}

} // namespace controls
} // namespace cppui
