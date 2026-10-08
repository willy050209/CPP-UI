/**
 * @file RoutedEventArgs.hpp
 * @brief 路由事件參數基底類別，提供路由事件參考、來源元件指標與中斷標記 (Handled)
 */

#pragma once

#include "compat/compat.hpp"
#include "ui/input/RoutedEvent.hpp"

namespace cppui {
namespace core {
    class UIElement;
} // namespace core

namespace input {

/// <summary>
/// 路由事件參數基底類別 (RoutedEventArgs)
/// </summary>
class RoutedEventArgs {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    RoutedEventArgs() = default;

    /// <summary>
    /// 建構具備事件元資料與來源元件之事件參數。
    /// </summary>
    /// <param name="routedEvent">路由事件定義描述項常數引用。</param>
    /// <param name="source">引發此事件之來源 UI 元件指標。</param>
    explicit RoutedEventArgs(const RoutedEvent& routedEvent, core::UIElement* source = nullptr) noexcept
        : m_routedEvent(&routedEvent), m_source(source), m_handled(false) {}

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    virtual ~RoutedEventArgs() = default;

    /// <summary>
    /// 取得關聯之路由事件元資料指標。
    /// </summary>
    /// <returns>RoutedEvent 指標。</returns>
    COMPAT_NODISCARD const RoutedEvent* GetRoutedEvent() const noexcept { return m_routedEvent; }

    /// <summary>
    /// 設定關聯之路由事件元資料。
    /// </summary>
    /// <param name="routedEvent">RoutedEvent 常數引用。</param>
    void SetRoutedEvent(const RoutedEvent& routedEvent) noexcept { m_routedEvent = &routedEvent; }

    /// <summary>
    /// 取得引發此事件之來源 UI 元件指標。
    /// </summary>
    /// <returns>UIElement 來源指標。</returns>
    COMPAT_NODISCARD core::UIElement* GetSource() const noexcept { return m_source; }

    /// <summary>
    /// 設定引發此事件之來源 UI 元件指標。
    /// </summary>
    /// <param name="source">UIElement 來源指標。</param>
    void SetSource(core::UIElement* source) noexcept { m_source = source; }

    /// <summary>
    /// 檢查事件是否已被標記為已處理 (Handled)。
    /// </summary>
    /// <returns>若已處理傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsHandled() const noexcept { return m_handled; }

    /// <summary>
    /// 設定事件處理狀態標記。
    /// </summary>
    /// <param name="handled">是否已處理，預設為 true。</param>
    void SetHandled(bool handled = true) noexcept { m_handled = handled; }

protected:
    const RoutedEvent* m_routedEvent = nullptr;
    core::UIElement*   m_source = nullptr;
    bool               m_handled = false;
};

} // namespace input
} // namespace cppui
