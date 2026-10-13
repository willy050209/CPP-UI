/*
 * File: ILightDismissable.hpp
 * Purpose: Declares ILightDismissable interface for light-dismiss popup controls.
 */

#pragma once

#include "compat/compat.hpp"

namespace cppui {
namespace core {
class UIElement;
} // namespace core

namespace input {

/// <summary>
/// 輕量關閉協定介面 (ILightDismissable)
/// 供浮層控制項實作，供 InputManager 進行全域外部點擊收合與 ESC 關閉管理。
/// </summary>
class ILightDismissable {
public:
    /// <summary>虛擬解構子。</summary>
    virtual ~ILightDismissable() = default;

    /// <summary>查詢是否鎖定常開。</summary>
    /// <returns>若常開回傳 true。</returns>
    COMPAT_NODISCARD virtual bool StaysOpen() const noexcept = 0;

    /// <summary>取得錨定目標 UIElement 指標。</summary>
    /// <returns>目標元件指標，若無則傳回 nullptr。</returns>
    COMPAT_NODISCARD virtual core::UIElement* PlacementTarget() const noexcept = 0;

    /// <summary>設定開啟或關閉狀態。</summary>
    /// <param name="open">true 開啟，false 關閉。</param>
    virtual void SetIsOpen(bool open) = 0;
};

} // namespace input
} // namespace cppui
