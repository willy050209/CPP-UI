/**
 * @file FocusEventArgs.hpp
 * @brief 鍵盤焦點切換 (GotFocus / LostFocus) 路由事件參數定義
 */

#pragma once

#include "compat/compat.hpp"
#include "ui/input/RoutedEventArgs.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 焦點變更事件參數 (FocusEventArgs)
/// </summary>
class FocusEventArgs : public RoutedEventArgs {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    FocusEventArgs() = default;

    /// <summary>
    /// 建構焦點變更事件參數。
    /// </summary>
    /// <param name="routedEvent">路由事件定義描述項常數引用。</param>
    /// <param name="source">引發事件之來源 UI 元件指標。</param>
    /// <param name="oldFocus">失去焦點之原 UI 元件指標。</param>
    /// <param name="newFocus">獲得焦點之新 UI 元件指標。</param>
    FocusEventArgs(
        const RoutedEvent& routedEvent,
        core::UIElement* source,
        core::UIElement* oldFocus,
        core::UIElement* newFocus
    ) noexcept
        : RoutedEventArgs(routedEvent, source)
        , m_oldFocus(oldFocus)
        , m_newFocus(newFocus) {}

    /// <summary>取得失去焦點之原元件指標。</summary>
    /// <returns>UIElement 指標，若原本無焦點則為 nullptr。</returns>
    COMPAT_NODISCARD core::UIElement* GetOldFocus() const noexcept { return m_oldFocus; }

    /// <summary>取得獲得焦點之新元件指標。</summary>
    /// <returns>UIElement 指標，若無新焦點則為 nullptr。</returns>
    COMPAT_NODISCARD core::UIElement* GetNewFocus() const noexcept { return m_newFocus; }

private:
    core::UIElement* m_oldFocus = nullptr;
    core::UIElement* m_newFocus = nullptr;
};

} // namespace input
} // namespace cppui
