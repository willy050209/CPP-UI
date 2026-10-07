#pragma once

#include "compat/compat.hpp"
#include "ui/input/RoutedEvent.hpp"

namespace cppui {
namespace core {
    class UIElement;
} // namespace core

namespace input {

/// <summary>
/// 路由事件參數基底類別
/// </summary>
class RoutedEventArgs {
public:
    RoutedEventArgs() = default;

    explicit RoutedEventArgs(const RoutedEvent& routedEvent, core::UIElement* source = nullptr) noexcept
        : m_routedEvent(&routedEvent), m_source(source), m_handled(false) {}

    virtual ~RoutedEventArgs() = default;

    COMPAT_NODISCARD const RoutedEvent* GetRoutedEvent() const noexcept { return m_routedEvent; }
    void SetRoutedEvent(const RoutedEvent& routedEvent) noexcept { m_routedEvent = &routedEvent; }

    COMPAT_NODISCARD core::UIElement* GetSource() const noexcept { return m_source; }
    void SetSource(core::UIElement* source) noexcept { m_source = source; }

    COMPAT_NODISCARD bool IsHandled() const noexcept { return m_handled; }
    void SetHandled(bool handled = true) noexcept { m_handled = handled; }

protected:
    const RoutedEvent* m_routedEvent = nullptr;
    core::UIElement*   m_source = nullptr;
    bool               m_handled = false;
};

} // namespace input
} // namespace cppui
