/**
 * @file RoutedEvent.hpp
 * @brief 路由事件元資料描述項與全域標準事件常數定義
 */

#pragma once

#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/input/InputTypes.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 路由事件元資料描述符
/// </summary>
class RoutedEvent {
public:
    /// <summary>
    /// 建構路由事件描述項。
    /// </summary>
    /// <param name="id">唯一整數識別碼。</param>
    /// <param name="name">事件名稱字串指標。</param>
    /// <param name="strategy">路由傳播策略 (Direct/Tunnel/Bubble)。</param>
    constexpr RoutedEvent(uint32_t id, const char* name, RoutingStrategy strategy) noexcept
        : m_id(id), m_name(name), m_strategy(strategy) {}

    /// <summary>
    /// 取得事件識別碼。
    /// </summary>
    /// <returns>識別碼整數值。</returns>
    COMPAT_NODISCARD constexpr uint32_t GetId() const noexcept { return m_id; }

    /// <summary>
    /// 取得事件名稱字串指標。
    /// </summary>
    /// <returns>字串指標。</returns>
    COMPAT_NODISCARD constexpr const char* GetName() const noexcept { return m_name; }

    /// <summary>
    /// 取得路由傳播策略。
    /// </summary>
    /// <returns>RoutingStrategy 列舉值。</returns>
    COMPAT_NODISCARD constexpr RoutingStrategy GetStrategy() const noexcept { return m_strategy; }

    /// <summary>
    /// 相等比較運算子。
    /// </summary>
    /// <param name="other">另一個路由事件。</param>
    /// <returns>若識別碼相同傳回 true。</returns>
    COMPAT_NODISCARD constexpr bool operator==(const RoutedEvent& other) const noexcept {
        return m_id == other.m_id;
    }

    /// <summary>
    /// 不相等比較運算子。
    /// </summary>
    /// <param name="other">另一個路由事件。</param>
    /// <returns>若識別碼不同傳回 true。</returns>
    COMPAT_NODISCARD constexpr bool operator!=(const RoutedEvent& other) const noexcept {
        return m_id != other.m_id;
    }

private:
    uint32_t        m_id;
    const char*     m_name;
    RoutingStrategy m_strategy;
};

/// <summary>
/// 全域預定義標準路由事件常數
/// </summary>
namespace StandardEvents {
    COMPAT_INLINE_VAR constexpr RoutedEvent PreviewPointerPressed(1, "PreviewPointerPressed", RoutingStrategy::Tunnel);
    COMPAT_INLINE_VAR constexpr RoutedEvent PointerPressed(2, "PointerPressed", RoutingStrategy::Bubble);
    COMPAT_INLINE_VAR constexpr RoutedEvent PreviewPointerReleased(3, "PreviewPointerReleased", RoutingStrategy::Tunnel);
    COMPAT_INLINE_VAR constexpr RoutedEvent PointerReleased(4, "PointerReleased", RoutingStrategy::Bubble);
    COMPAT_INLINE_VAR constexpr RoutedEvent PreviewPointerMoved(5, "PreviewPointerMoved", RoutingStrategy::Tunnel);
    COMPAT_INLINE_VAR constexpr RoutedEvent PointerMoved(6, "PointerMoved", RoutingStrategy::Bubble);
    COMPAT_INLINE_VAR constexpr RoutedEvent PointerWheel(7, "PointerWheel", RoutingStrategy::Bubble);
    COMPAT_INLINE_VAR constexpr RoutedEvent PointerEntered(8, "PointerEntered", RoutingStrategy::Direct);
    COMPAT_INLINE_VAR constexpr RoutedEvent PointerExited(9, "PointerExited", RoutingStrategy::Direct);

    COMPAT_INLINE_VAR constexpr RoutedEvent PreviewKeyDown(10, "PreviewKeyDown", RoutingStrategy::Tunnel);
    COMPAT_INLINE_VAR constexpr RoutedEvent KeyDown(11, "KeyDown", RoutingStrategy::Bubble);
    COMPAT_INLINE_VAR constexpr RoutedEvent PreviewKeyUp(12, "PreviewKeyUp", RoutingStrategy::Tunnel);
    COMPAT_INLINE_VAR constexpr RoutedEvent KeyUp(13, "KeyUp", RoutingStrategy::Bubble);
    COMPAT_INLINE_VAR constexpr RoutedEvent TextInput(14, "TextInput", RoutingStrategy::Bubble);

    COMPAT_INLINE_VAR constexpr RoutedEvent GotFocus(15, "GotFocus", RoutingStrategy::Direct);
    COMPAT_INLINE_VAR constexpr RoutedEvent LostFocus(16, "LostFocus", RoutingStrategy::Direct);
    COMPAT_INLINE_VAR constexpr RoutedEvent ImeComposition(17, "ImeComposition", RoutingStrategy::Bubble);
} // namespace StandardEvents

} // namespace input
} // namespace cppui
