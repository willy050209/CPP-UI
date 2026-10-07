#pragma once

#include <cstdint>
#include "compat/compat.hpp"
#include "ui/input/InputTypes.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 路由事件元資料描述符
/// </summary>
class RoutedEvent {
public:
    constexpr RoutedEvent(uint32_t id, const char* name, RoutingStrategy strategy) noexcept
        : m_id(id), m_name(name), m_strategy(strategy) {}

    COMPAT_NODISCARD constexpr uint32_t GetId() const noexcept { return m_id; }
    COMPAT_NODISCARD constexpr const char* GetName() const noexcept { return m_name; }
    COMPAT_NODISCARD constexpr RoutingStrategy GetStrategy() const noexcept { return m_strategy; }

    COMPAT_NODISCARD constexpr bool operator==(const RoutedEvent& other) const noexcept {
        return m_id == other.m_id;
    }

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
} // namespace StandardEvents

} // namespace input
} // namespace cppui
