#include "ui/input/InputManager.hpp"
#include "ui/input/EventRoute.hpp"
#include "ui/core/UIElement.hpp"
#include <algorithm>

namespace cppui {
namespace input {

InputManager& InputManager::Get() noexcept {
    static InputManager s_instance;
    return s_instance;
}

void InputManager::UpdateHover(core::UIElement* newHover, core::Point windowPos, pal::KeyModifiers mods) {
    if (m_hoveredElement == newHover) return;

    core::UIElement* oldHover = m_hoveredElement;
    m_hoveredElement = newHover;

    if (oldHover) {
        PointerEventArgs exitArgs(StandardEvents::PointerExited, oldHover, windowPos, pal::RawMouseButton::None, mods);
        EventRoute route(oldHover);
        route.DispatchDirect(StandardEvents::PointerExited, exitArgs);
    }

    if (newHover) {
        PointerEventArgs enterArgs(StandardEvents::PointerEntered, newHover, windowPos, pal::RawMouseButton::None, mods);
        EventRoute route(newHover);
        route.DispatchDirect(StandardEvents::PointerEntered, enterArgs);
    }
}

bool InputManager::CapturePointer(core::UIElement* element) {
    if (!element) return false;
    m_capturedElement = element;
    return true;
}

void InputManager::ReleasePointerCapture(core::UIElement* element) {
    if (!element || m_capturedElement == element) {
        m_capturedElement = nullptr;
    }
}

void InputManager::PruneDeadPopups() {
    m_openPopups.erase(
        std::remove_if(m_openPopups.begin(), m_openPopups.end(), [](const OpenPopupEntry& entry) {
            return !entry.popup || (entry.token.HasHandle() && !entry.token.IsAlive());
        }),
        m_openPopups.end()
    );
}

void InputManager::ResetState() {
    m_capturedElement = nullptr;
    m_hoveredElement = nullptr;
    m_openPopups.clear();
    m_activePopupsCache.clear();
}

void InputManager::NotifyElementDestroyed(core::UIElement* element) noexcept {
    if (m_hoveredElement == element) {
        m_hoveredElement = nullptr;
    }
    if (m_capturedElement == element) {
        m_capturedElement = nullptr;
    }
    auto* dismissable = dynamic_cast<ILightDismissable*>(element);
    if (dismissable) {
        UnregisterOpenPopup(dismissable);
    }
    PruneDeadPopups();
}

void InputManager::RegisterOpenPopup(ILightDismissable* popup, core::LifetimeToken token) {
    if (!popup) return;
    PruneDeadPopups();

    if (!token.HasHandle()) {
        if (auto* elem = dynamic_cast<core::UIElement*>(popup)) {
            token = elem->GetLifetimeToken();
        }
    }

    auto it = std::find_if(m_openPopups.begin(), m_openPopups.end(), [popup](const OpenPopupEntry& e) {
        return e.popup == popup;
    });
    if (it == m_openPopups.end()) {
        m_openPopups.push_back({ popup, token });
    } else {
        it->token = token;
    }
}

void InputManager::UnregisterOpenPopup(ILightDismissable* popup) {
    if (!popup) return;
    auto it = std::find_if(m_openPopups.begin(), m_openPopups.end(), [popup](const OpenPopupEntry& e) {
        return e.popup == popup;
    });
    if (it != m_openPopups.end()) {
        m_openPopups.erase(it);
    }
    PruneDeadPopups();
}

const std::vector<ILightDismissable*>& InputManager::GetOpenPopups() const {
    const_cast<InputManager*>(this)->PruneDeadPopups();
    m_activePopupsCache.clear();
    m_activePopupsCache.reserve(m_openPopups.size());
    for (const auto& entry : m_openPopups) {
        if (entry.popup && (!entry.token.HasHandle() || entry.token.IsAlive())) {
            m_activePopupsCache.push_back(entry.popup);
        }
    }
    return m_activePopupsCache;
}

void InputManager::ProcessMouseMove(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods) {
    core::Visual* hitVisual = root.HitTest(mouseData.position);
    auto* hitUI = dynamic_cast<core::UIElement*>(hitVisual);

    if (m_inspectorHook && m_inspectorHook->IsPickerActive()) {
        m_inspectorHook->OnMouseMove(hitUI);
        root.InvalidateVisual();
    }

    UpdateHover(hitUI, mouseData.position, mods);

    core::UIElement* target = m_capturedElement ? m_capturedElement : hitUI;
    if (!target) target = &root;

    PointerEventArgs args(
        StandardEvents::PointerMoved,
        target,
        mouseData.position,
        mouseData.button,
        mods,
        mouseData.clickCount,
        mouseData.wheelDeltaX,
        mouseData.wheelDeltaY
    );
    EventRoute route(target);
    route.Dispatch(StandardEvents::PreviewPointerMoved, StandardEvents::PointerMoved, args);
}

void InputManager::ProcessMouseDown(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods) {
    core::Visual* hitVisual = root.HitTest(mouseData.position);
    auto* hitUI = dynamic_cast<core::UIElement*>(hitVisual);

    if (m_inspectorHook && m_inspectorHook->IsPickerActive()) {
        m_inspectorHook->OnMouseDown(hitUI ? hitUI : &root);
        root.InvalidateVisual();
    }

    PruneDeadPopups();

    // Light-Dismiss: 若存在開啟且未鎖定常開的浮層，且點擊落於該浮層及其目標之外，則自動收合
    if (!m_openPopups.empty()) {
        const auto snapshot = m_openPopups;
        std::vector<ILightDismissable*> toClose;
        for (const auto& entry : snapshot) {
            if (entry.token.HasHandle() && !entry.token.IsAlive()) continue;
            auto* popup = entry.popup;
            if (!popup || popup->StaysOpen()) continue;

            auto it = std::find_if(m_openPopups.begin(), m_openPopups.end(), [popup](const OpenPopupEntry& e) {
                return e.popup == popup;
            });
            if (it == m_openPopups.end()) continue;

            bool isInside = false;
            core::Visual* cur = hitUI;
            auto* popupElement = dynamic_cast<core::UIElement*>(popup);
            while (cur) {
                if ((popupElement && cur == popupElement) || (popup->PlacementTarget() && cur == popup->PlacementTarget())) {
                    isInside = true;
                    break;
                }
                cur = cur->GetVisualParent();
            }
            if (!isInside) {
                toClose.push_back(popup);
            }
        }
        for (auto* popup : toClose) {
            auto it = std::find_if(m_openPopups.begin(), m_openPopups.end(), [popup](const OpenPopupEntry& e) {
                return e.popup == popup;
            });
            if (it != m_openPopups.end() && (!it->token.HasHandle() || it->token.IsAlive())) {
                popup->SetIsOpen(false);
            }
        }
    }

    core::UIElement* target = m_capturedElement ? m_capturedElement : hitUI;
    if (!target) target = &root;

    // 點擊命中自動獲焦 (若目標元件支援 Focus)
    if (hitUI && hitUI->IsFocusable()) {
        FocusManager::Get().SetFocus(hitUI);
    }

    PointerEventArgs args(
        StandardEvents::PointerPressed,
        target,
        mouseData.position,
        mouseData.button,
        mods,
        mouseData.clickCount,
        mouseData.wheelDeltaX,
        mouseData.wheelDeltaY
    );
    EventRoute route(target);
    route.Dispatch(StandardEvents::PreviewPointerPressed, StandardEvents::PointerPressed, args);
}

void InputManager::ProcessMouseUp(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods) {
    core::Visual* hitVisual = root.HitTest(mouseData.position);
    auto* hitUI = dynamic_cast<core::UIElement*>(hitVisual);

    core::UIElement* target = m_capturedElement ? m_capturedElement : hitUI;
    if (!target) target = &root;

    PointerEventArgs args(
        StandardEvents::PointerReleased,
        target,
        mouseData.position,
        mouseData.button,
        mods,
        mouseData.clickCount,
        mouseData.wheelDeltaX,
        mouseData.wheelDeltaY
    );
    EventRoute route(target);
    route.Dispatch(StandardEvents::PreviewPointerReleased, StandardEvents::PointerReleased, args);
}

void InputManager::ProcessMouseWheel(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods) {
    core::Visual* hitVisual = root.HitTest(mouseData.position);
    auto* hitUI = dynamic_cast<core::UIElement*>(hitVisual);

    core::UIElement* target = m_capturedElement ? m_capturedElement : hitUI;
    if (!target) target = &root;

    PointerEventArgs args(
        StandardEvents::PointerWheel,
        target,
        mouseData.position,
        mouseData.button,
        mods,
        mouseData.clickCount,
        mouseData.wheelDeltaX,
        mouseData.wheelDeltaY
    );
    EventRoute route(target);
    route.DispatchBubble(StandardEvents::PointerWheel, args);
}

void InputManager::ProcessKeyDown(core::UIElement& root, const pal::RawKeyEventData& keyData, pal::KeyModifiers mods) {
    Key key = static_cast<Key>(keyData.virtualKey);

    // 檢查 Escape 鍵關閉頂層 Popup
    if (key == Key::Escape) {
        PruneDeadPopups();
        while (!m_openPopups.empty()) {
            auto entry = m_openPopups.back();
            m_openPopups.pop_back();
            if (entry.token.HasHandle() && !entry.token.IsAlive()) {
                continue;
            }
            if (entry.popup) {
                entry.popup->SetIsOpen(false);
                return; // 優先消費 ESC 鍵關閉浮層
            }
        }
    }

    // 檢查 F12 鍵切換 DevTools Visual Tree Inspector (支援標準 0x7B 與相容 0x77)
    if ((key == Key::F12 || keyData.virtualKey == 0x77) && m_inspectorHook) {
        m_inspectorHook->OnF12Pressed(root);
        return;
    }

    // 檢查 Tab / Shift+Tab 鍵盤焦點導航
    if (key == Key::Tab) {
        bool shiftPressed = pal::HasModifier(mods, pal::KeyModifiers::Shift);
        if (FocusManager::Get().MoveFocus(&root, !shiftPressed)) {
            return; // 成功導航焦點，終止該按鍵派發以防止轉移至新焦點
        }
    }

    core::UIElement* target = FocusManager::Get().GetFocusedElement();
    if (!target) target = &root;

    KeyEventArgs args(
        StandardEvents::KeyDown,
        target,
        keyData.virtualKey,
        keyData.scanCode,
        keyData.isRepeat,
        mods
    );
    EventRoute route(target);
    route.Dispatch(StandardEvents::PreviewKeyDown, StandardEvents::KeyDown, args);
}

void InputManager::ProcessKeyUp(core::UIElement& root, const pal::RawKeyEventData& keyData, pal::KeyModifiers mods) {
    core::UIElement* target = FocusManager::Get().GetFocusedElement();
    if (!target) target = &root;

    KeyEventArgs args(
        StandardEvents::KeyUp,
        target,
        keyData.virtualKey,
        keyData.scanCode,
        keyData.isRepeat,
        mods
    );
    EventRoute route(target);
    route.Dispatch(StandardEvents::PreviewKeyUp, StandardEvents::KeyUp, args);
}

void InputManager::ProcessTextInput(core::UIElement& root, const pal::RawTextInputData& textData) {
    uint32_t cp = textData.codepoint;
    // 過濾非列印 ASCII 控制字元 (如 \t, \r, \b, \x1b)，防止控制鍵作為字串插入單行文字方塊
    if (cp < 0x20) {
        return;
    }

    core::UIElement* target = FocusManager::Get().GetFocusedElement();
    if (!target) target = &root;

    // 將 UTF-32 碼點轉換為 UTF-8 字串
    std::string utf8Str;
    if (cp <= 0x7F) {
        utf8Str.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        utf8Str.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        utf8Str.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        utf8Str.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        utf8Str.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        utf8Str.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        utf8Str.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        utf8Str.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        utf8Str.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        utf8Str.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }

    TextInputEventArgs args(StandardEvents::TextInput, target, cp, compat::string_view(utf8Str.data(), utf8Str.size()));
    EventRoute route(target);
    route.DispatchBubble(StandardEvents::TextInput, args);
}

void InputManager::ProcessImeComposition(core::UIElement& root, const pal::RawImeData& imeData) {
    core::UIElement* target = FocusManager::Get().GetFocusedElement();
    if (!target) target = &root;

    ImeCompositionEventArgs args(
        StandardEvents::ImeComposition,
        target,
        compat::string_view(imeData.text),
        imeData.cursorPosition,
        imeData.isResult
    );
    EventRoute route(target);
    route.DispatchBubble(StandardEvents::ImeComposition, args);
}

void InputManager::ProcessImeEndComposition(core::UIElement& root) {
    core::UIElement* target = FocusManager::Get().GetFocusedElement();
    if (!target) target = &root;

    ImeCompositionEventArgs args(
        StandardEvents::ImeComposition,
        target,
        compat::string_view(),
        0,
        false
    );
    EventRoute route(target);
    route.DispatchBubble(StandardEvents::ImeComposition, args);
}

void InputManager::ProcessRawEvent(core::UIElement& root, const pal::RawInputEvent& rawEvent) {
    switch (rawEvent.type) {
    case pal::RawEventType::MouseMove:
        ProcessMouseMove(root, rawEvent.mouse, rawEvent.modifiers);
        break;
    case pal::RawEventType::MouseDown:
        ProcessMouseDown(root, rawEvent.mouse, rawEvent.modifiers);
        break;
    case pal::RawEventType::MouseUp:
        ProcessMouseUp(root, rawEvent.mouse, rawEvent.modifiers);
        break;
    case pal::RawEventType::MouseWheel:
        ProcessMouseWheel(root, rawEvent.mouse, rawEvent.modifiers);
        break;
    case pal::RawEventType::KeyDown:
        ProcessKeyDown(root, rawEvent.key, rawEvent.modifiers);
        break;
    case pal::RawEventType::KeyUp:
        ProcessKeyUp(root, rawEvent.key, rawEvent.modifiers);
        break;
    case pal::RawEventType::TextInput:
        ProcessTextInput(root, rawEvent.text);
        break;
    case pal::RawEventType::ImeComposition:
        ProcessImeComposition(root, rawEvent.ime);
        break;
    case pal::RawEventType::ImeEndComposition:
        ProcessImeEndComposition(root);
        break;
    case pal::RawEventType::WindowFocusLost:
        ResetState();
        break;
    default:
        break;
    }
}

} // namespace input
} // namespace cppui
