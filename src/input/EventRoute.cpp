#include "ui/input/EventRoute.hpp"
#include "ui/core/UIElement.hpp"

namespace cppui {
namespace input {

EventRoute::EventRoute(core::UIElement* target) : m_target(target) {
    core::UIElement* cur = target;
    while (cur) {
        m_branch.push_back(cur);
        cur = cur->GetParentUIElement();
    }
}

void EventRoute::Dispatch(const RoutedEvent& tunnelEvent,
                          const RoutedEvent& bubbleEvent,
                          RoutedEventArgs& args) {
    if (m_branch.empty()) return;

    args.SetSource(m_target);

    // 1. 隧道階段 (Tunneling: Root -> Target)
    args.SetRoutedEvent(tunnelEvent);
    for (auto it = m_branch.rbegin(); it != m_branch.rend(); ++it) {
        (*it)->RaiseEventInternal(tunnelEvent, args);
    }

    // 2. 冒泡階段 (Bubbling: Target -> Root)
    args.SetRoutedEvent(bubbleEvent);
    for (auto it = m_branch.begin(); it != m_branch.end(); ++it) {
        (*it)->RaiseEventInternal(bubbleEvent, args);
    }
}

void EventRoute::DispatchDirect(const RoutedEvent& directEvent, RoutedEventArgs& args) {
    if (!m_target) return;
    args.SetSource(m_target);
    args.SetRoutedEvent(directEvent);
    m_target->RaiseEventInternal(directEvent, args);
}

void EventRoute::DispatchBubble(const RoutedEvent& bubbleEvent, RoutedEventArgs& args) {
    if (m_branch.empty()) return;
    args.SetSource(m_target);
    args.SetRoutedEvent(bubbleEvent);
    for (auto* element : m_branch) {
        element->RaiseEventInternal(bubbleEvent, args);
    }
}

void EventRoute::DispatchTunnel(const RoutedEvent& tunnelEvent, RoutedEventArgs& args) {
    if (m_branch.empty()) return;
    args.SetSource(m_target);
    args.SetRoutedEvent(tunnelEvent);
    for (auto it = m_branch.rbegin(); it != m_branch.rend(); ++it) {
        (*it)->RaiseEventInternal(tunnelEvent, args);
    }
}

} // namespace input
} // namespace cppui
