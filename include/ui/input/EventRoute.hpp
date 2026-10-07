#pragma once

#include <vector>
#include "compat/compat.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"

namespace cppui {
namespace core {
    class UIElement;
} // namespace core

namespace input {

/// <summary>
/// 樹狀事件路由路徑與雙向分發器
/// </summary>
class EventRoute {
public:
    explicit EventRoute(core::UIElement* target);

    COMPAT_NODISCARD core::UIElement* GetTarget() const noexcept { return m_target; }
    COMPAT_NODISCARD const std::vector<core::UIElement*>& GetBranch() const noexcept { return m_branch; }

    /// <summary>
    /// 依序執行完整的 Tunnel -> Direct/Bubble 派發週期
    /// </summary>
    void Dispatch(const RoutedEvent& tunnelEvent,
                  const RoutedEvent& bubbleEvent,
                  RoutedEventArgs& args);

    /// <summary>
    /// 執行單一直接事件派發 (Direct)
    /// </summary>
    void DispatchDirect(const RoutedEvent& directEvent, RoutedEventArgs& args);

    /// <summary>
    /// 僅執行冒泡階段派發 (Bubble: Target -> Root)
    /// </summary>
    void DispatchBubble(const RoutedEvent& bubbleEvent, RoutedEventArgs& args);

    /// <summary>
    /// 僅執行隧道階段派發 (Tunnel: Root -> Target)
    /// </summary>
    void DispatchTunnel(const RoutedEvent& tunnelEvent, RoutedEventArgs& args);

private:
    core::UIElement*              m_target = nullptr;
    std::vector<core::UIElement*> m_branch; // [target, parent1, parent2, ..., root]
};

} // namespace input
} // namespace cppui
