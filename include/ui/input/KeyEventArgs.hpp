#pragma once

#include <string>
#include "compat/compat.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 鍵盤按鍵事件參數
/// </summary>
class KeyEventArgs : public RoutedEventArgs {
public:
    KeyEventArgs() = default;

    KeyEventArgs(
        const RoutedEvent& routedEvent,
        core::UIElement* source,
        uint32_t virtualKey,
        uint32_t scanCode = 0,
        bool isRepeat = false,
        pal::KeyModifiers modifiers = pal::KeyModifiers::None
    ) noexcept
        : RoutedEventArgs(routedEvent, source)
        , m_virtualKey(virtualKey)
        , m_scanCode(scanCode)
        , m_isRepeat(isRepeat)
        , m_modifiers(modifiers) {}

    COMPAT_NODISCARD uint32_t GetVirtualKey() const noexcept { return m_virtualKey; }
    COMPAT_NODISCARD uint32_t GetScanCode() const noexcept { return m_scanCode; }
    COMPAT_NODISCARD bool IsRepeat() const noexcept { return m_isRepeat; }
    COMPAT_NODISCARD pal::KeyModifiers GetModifiers() const noexcept { return m_modifiers; }

    COMPAT_NODISCARD bool HasControl() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Control); }
    COMPAT_NODISCARD bool HasShift() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Shift); }
    COMPAT_NODISCARD bool HasAlt() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Alt); }

private:
    uint32_t          m_virtualKey = 0;
    uint32_t          m_scanCode = 0;
    bool              m_isRepeat = false;
    pal::KeyModifiers m_modifiers = pal::KeyModifiers::None;
};

/// <summary>
/// 鍵盤文字輸入事件參數 (IME / 字元輸入)
/// </summary>
class TextInputEventArgs : public RoutedEventArgs {
public:
    TextInputEventArgs() = default;

    TextInputEventArgs(
        const RoutedEvent& routedEvent,
        core::UIElement* source,
        uint32_t codepoint,
        compat::string_view text
    )
        : RoutedEventArgs(routedEvent, source)
        , m_codepoint(codepoint)
        , m_text(text.data(), text.size()) {}

    COMPAT_NODISCARD uint32_t GetCodepoint() const noexcept { return m_codepoint; }
    COMPAT_NODISCARD compat::string_view GetText() const noexcept { return compat::string_view(m_text.data(), m_text.size()); }

private:
    uint32_t    m_codepoint = 0;
    std::string m_text;
};

} // namespace input
} // namespace cppui
