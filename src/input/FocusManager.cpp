#include "ui/input/FocusManager.hpp"
#include "ui/input/FocusEventArgs.hpp"
#include "ui/input/EventRoute.hpp"
#include "ui/core/UIElement.hpp"
#include <algorithm>

namespace cppui {
namespace input {

FocusManager& FocusManager::Get() noexcept {
    static FocusManager s_instance;
    return s_instance;
}

bool FocusManager::SetFocus(core::UIElement* element) {
    if (element == m_focusedElement) {
        return true;
    }

    if (element && (!element->IsFocusable() || !element->IsVisible())) {
        return false;
    }

    core::UIElement* oldFocus = m_focusedElement;
    m_focusedElement = element;

    // 1. 通知舊焦點元件失焦 (LostFocus)
    if (oldFocus) {
        oldFocus->SetFocusedInternal(false);
        FocusEventArgs lostArgs(StandardEvents::LostFocus, oldFocus, oldFocus, element);
        EventRoute route(oldFocus);
        route.DispatchDirect(StandardEvents::LostFocus, lostArgs);
    }

    // 2. 通知新焦點元件獲焦 (GotFocus)
    if (element) {
        element->SetFocusedInternal(true);
        FocusEventArgs gotArgs(StandardEvents::GotFocus, element, oldFocus, element);
        EventRoute route(element);
        route.DispatchDirect(StandardEvents::GotFocus, gotArgs);
    }

    return true;
}

void FocusManager::ClearFocus() {
    SetFocus(nullptr);
}

void FocusManager::NotifyElementDestroyed(core::UIElement* element) noexcept {
    if (m_focusedElement == element) {
        m_focusedElement = nullptr;
    }
}

void FocusManager::CollectFocusableElements(core::UIElement* current, std::vector<core::UIElement*>& outList) {
    if (!current || !current->IsVisible()) return;

    if (current->IsFocusable() && current->IsTabStop()) {
        outList.push_back(current);
    }

    size_t childCount = current->GetVisualChildrenCount();
    for (size_t i = 0; i < childCount; ++i) {
        core::Visual* childVisual = current->GetVisualChild(i);
        auto* childUI = dynamic_cast<core::UIElement*>(childVisual);
        if (childUI) {
            CollectFocusableElements(childUI, outList);
        }
    }
}

bool FocusManager::MoveFocus(core::UIElement* root, bool forward) {
    if (!root) return false;

    std::vector<core::UIElement*> focusables;
    CollectFocusableElements(root, focusables);
    if (focusables.empty()) return false;

    // 依 TabIndex 升序穩定排序
    std::stable_sort(focusables.begin(), focusables.end(), [](const core::UIElement* a, const core::UIElement* b) {
        return a->GetTabIndex() < b->GetTabIndex();
    });

    size_t targetIndex = 0;
    auto it = std::find(focusables.begin(), focusables.end(), m_focusedElement);
    if (it == focusables.end()) {
        targetIndex = forward ? 0 : (focusables.size() - 1);
    } else {
        size_t currentIdx = static_cast<size_t>(std::distance(focusables.begin(), it));
        if (forward) {
            targetIndex = (currentIdx + 1) % focusables.size();
        } else {
            targetIndex = (currentIdx == 0) ? (focusables.size() - 1) : (currentIdx - 1);
        }
    }

    return SetFocus(focusables[targetIndex]);
}

} // namespace input
} // namespace cppui
