#pragma once

#include "compat/compat.hpp"
#include "ui/input/RoutedEventArgs.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 焦點變更事件參數
/// </summary>
class FocusEventArgs : public RoutedEventArgs {
public:
    FocusEventArgs() = default;

    FocusEventArgs(
        const RoutedEvent& routedEvent,
        core::UIElement* source,
        core::UIElement* oldFocus,
        core::UIElement* newFocus
    ) noexcept
        : RoutedEventArgs(routedEvent, source)
        , m_oldFocus(oldFocus)
        , m_newFocus(newFocus) {}

    COMPAT_NODISCARD core::UIElement* GetOldFocus() const noexcept { return m_oldFocus; }
    COMPAT_NODISCARD core::UIElement* GetNewFocus() const noexcept { return m_newFocus; }

private:
    core::UIElement* m_oldFocus = nullptr;
    core::UIElement* m_newFocus = nullptr;
};

} // namespace input
} // namespace cppui
