#include "ui/input/InputManager.hpp"
#include "ui/input/EventRoute.hpp"
#include "ui/core/UIElement.hpp"

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

void InputManager::ResetState() {
    m_capturedElement = nullptr;
    m_hoveredElement = nullptr;
}

void InputManager::NotifyElementDestroyed(core::UIElement* element) noexcept {
    if (m_hoveredElement == element) {
        m_hoveredElement = nullptr;
    }
    if (m_capturedElement == element) {
        m_capturedElement = nullptr;
    }
}

void InputManager::ProcessMouseMove(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods) {
    core::Visual* hitVisual = root.HitTest(mouseData.position);
    auto* hitUI = dynamic_cast<core::UIElement*>(hitVisual);

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
    // 檢查 Tab / Shift+Tab 鍵盤焦點導航 (VK_TAB = 0x09)
    if (keyData.virtualKey == 0x09) {
        bool shiftPressed = pal::HasModifier(mods, pal::KeyModifiers::Shift);
        FocusManager::Get().MoveFocus(&root, !shiftPressed);
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
    core::UIElement* target = FocusManager::Get().GetFocusedElement();
    if (!target) target = &root;

    // 將 UTF-32 碼點轉換為 UTF-8 字串
    std::string utf8Str;
    uint32_t cp = textData.codepoint;
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
    case pal::RawEventType::WindowFocusLost:
        ResetState();
        break;
    default:
        break;
    }
}

} // namespace input
} // namespace cppui
