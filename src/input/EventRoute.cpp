/*
 * File: EventRoute.cpp
 * Purpose: Implements event routing path traversal with LifetimeToken UAF protection.
 */

#include "ui/input/EventRoute.hpp"
#include "ui/core/UIElement.hpp"

namespace cppui {
namespace input {

EventRoute::EventRoute(core::UIElement* target) : m_target(target) {
    core::UIElement* cur = target;
    while (cur) {
        m_branch.push_back({ cur, cur->GetLifetimeToken() });
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
        if (it->token.IsAlive() && it->element) {
            it->element->RaiseEventInternal(tunnelEvent, args);
        }
    }

    // 2. 冒泡階段 (Bubbling: Target -> Root)
    args.SetRoutedEvent(bubbleEvent);
    for (auto it = m_branch.begin(); it != m_branch.end(); ++it) {
        if (it->token.IsAlive() && it->element) {
            it->element->RaiseEventInternal(bubbleEvent, args);
        }
    }
}

void EventRoute::DispatchDirect(const RoutedEvent& directEvent, RoutedEventArgs& args) {
    if (!m_target) return;
    if (m_branch.empty() || (m_branch.front().token.IsAlive() && m_branch.front().element == m_target)) {
        args.SetSource(m_target);
        args.SetRoutedEvent(directEvent);
        m_target->RaiseEventInternal(directEvent, args);
    }
}

void EventRoute::DispatchBubble(const RoutedEvent& bubbleEvent, RoutedEventArgs& args) {
    if (m_branch.empty()) return;
    args.SetSource(m_target);
    args.SetRoutedEvent(bubbleEvent);
    for (const auto& node : m_branch) {
        if (node.token.IsAlive() && node.element) {
            node.element->RaiseEventInternal(bubbleEvent, args);
        }
    }
}

void EventRoute::DispatchTunnel(const RoutedEvent& tunnelEvent, RoutedEventArgs& args) {
    if (m_branch.empty()) return;
    args.SetSource(m_target);
    args.SetRoutedEvent(tunnelEvent);
    for (auto it = m_branch.rbegin(); it != m_branch.rend(); ++it) {
        if (it->token.IsAlive() && it->element) {
            it->element->RaiseEventInternal(tunnelEvent, args);
        }
    }
}

} // namespace input
} // namespace cppui
