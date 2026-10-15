/**
 * @file InputTypes.hpp
 * @brief 輸入系統列舉型別定義 (RoutingStrategy, FocusNavigationDirection)
 */

#pragma once

#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/pal/RawInputEvent.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 路由事件傳播策略
/// </summary>
enum class RoutingStrategy : uint8_t {
    Tunnel, // 隧道策略：自根節點 (Root) 向下傳遞至目標節點 (Target)，通常為 Preview* 事件
    Bubble, // 冒泡策略：自目標節點 (Target) 向上傳遞至根節點 (Root)，通常為常規互動事件
    Direct  // 直接策略：不進行樹狀傳播，直接發送給目標節點 (例如 PointerEntered, GotFocus)
};

/// <summary>
/// 焦點導航方向
/// </summary>
enum class FocusNavigationDirection : uint8_t {
    Next,    // Tab 鍵：順向導航至下一個焦點元件
    Previous // Shift + Tab 鍵：逆向導航至上一個焦點元件
};

} // namespace input
} // namespace cppui
