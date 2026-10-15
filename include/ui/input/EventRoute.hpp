/**
 * @file EventRoute.hpp
 * @brief 樹狀事件路由路徑與雙向派發器 (Tunnel / Direct / Bubble)
 */

#pragma once

#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/LifetimeToken.hpp"
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
    /// <summary>
    /// 路由節點結構，結合 UIElement 指標與其 LifetimeToken，防範派發回呼中刪除物件引發 Use-After-Free。
    /// </summary>
    struct RouteNode {
        core::UIElement*    element = nullptr;
        core::LifetimeToken token;
    };

    /// <summary>
    /// 建構事件路由路徑，自目標節點向上回溯構建路由鏈分支。
    /// </summary>
    /// <param name="target">命中之目標 UI 元件指標。</param>
    explicit EventRoute(core::UIElement* target);

    /// <summary>取得事件目標元件指標。</summary>
    /// <returns>UIElement 指標。</returns>
    COMPAT_NODISCARD core::UIElement* GetTarget() const noexcept { return m_target; }

    /// <summary>取得向上回溯至根節點之視覺樹分支節點清單。</summary>
    /// <returns>RouteNode 清單引用。</returns>
    COMPAT_NODISCARD const std::vector<RouteNode>& GetBranch() const noexcept { return m_branch; }

    /// <summary>
    /// 依序執行完整的 Tunnel -> Direct/Bubble 派發週期。
    /// </summary>
    /// <param name="tunnelEvent">隧道事件描述項。</param>
    /// <param name="bubbleEvent">冒泡事件描述項。</param>
    /// <param name="args">事件參數引用。</param>
    void Dispatch(const RoutedEvent& tunnelEvent,
                  const RoutedEvent& bubbleEvent,
                  RoutedEventArgs& args);

    /// <summary>
    /// 執行單一直接事件派發 (Direct: 僅派發至目標節點)。
    /// </summary>
    /// <param name="directEvent">直接事件描述項。</param>
    /// <param name="args">事件參數引用。</param>
    void DispatchDirect(const RoutedEvent& directEvent, RoutedEventArgs& args);

    /// <summary>
    /// 僅執行冒泡階段派發 (Bubble: Target -> Root)。
    /// </summary>
    /// <param name="bubbleEvent">冒泡事件描述項。</param>
    /// <param name="args">事件參數引用。</param>
    void DispatchBubble(const RoutedEvent& bubbleEvent, RoutedEventArgs& args);

    /// <summary>
    /// 僅執行隧道階段派發 (Tunnel: Root -> Target)。
    /// </summary>
    /// <param name="tunnelEvent">隧道事件描述項。</param>
    /// <param name="args">事件參數引用。</param>
    void DispatchTunnel(const RoutedEvent& tunnelEvent, RoutedEventArgs& args);

private:
    core::UIElement*       m_target = nullptr;
    std::vector<RouteNode> m_branch; // [target, parent1, parent2, ..., root]
};

} // namespace input
} // namespace cppui
