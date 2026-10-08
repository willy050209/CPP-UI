/**
 * @file PointerEventArgs.hpp
 * @brief 滑鼠與指標路由事件參數定義，包含視窗座標、按鍵狀態、修飾鍵與局部座標投影
 */

#pragma once

#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/Visual.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 滑鼠與指標事件參數 (PointerEventArgs)
/// </summary>
class PointerEventArgs : public RoutedEventArgs {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    PointerEventArgs() = default;

    /// <summary>
    /// 建構完整的指標事件參數。
    /// </summary>
    /// <param name="routedEvent">路由事件定義描述項常數引用。</param>
    /// <param name="source">引發事件之來源 UI 元件指標。</param>
    /// <param name="windowPosition">相對於視窗客戶區之像素座標。</param>
    /// <param name="button">觸發之滑鼠按鈕。</param>
    /// <param name="modifiers">當前按下的鍵盤修飾鍵組合。</param>
    /// <param name="clickCount">連擊次數。</param>
    /// <param name="wheelDeltaX">水平滾輪滾動量。</param>
    /// <param name="wheelDeltaY">垂直滾輪滾動量。</param>
    PointerEventArgs(
        const RoutedEvent& routedEvent,
        core::UIElement* source,
        core::Point windowPosition,
        pal::RawMouseButton button = pal::RawMouseButton::None,
        pal::KeyModifiers modifiers = pal::KeyModifiers::None,
        int32_t clickCount = 1,
        int32_t wheelDeltaX = 0,
        int32_t wheelDeltaY = 0
    ) noexcept
        : RoutedEventArgs(routedEvent, source)
        , m_windowPosition(windowPosition)
        , m_button(button)
        , m_modifiers(modifiers)
        , m_clickCount(clickCount)
        , m_wheelDeltaX(wheelDeltaX)
        , m_wheelDeltaY(wheelDeltaY) {}

    /// <summary>
    /// 取得相對於視窗客戶區的原生座標點。
    /// </summary>
    /// <returns>視窗像素座標點。</returns>
    COMPAT_NODISCARD core::Point GetWindowPosition() const noexcept {
        return m_windowPosition;
    }

    /// <summary>
    /// 取得相對於指定視覺節點的局部座標；若傳入 nullptr 則回傳視窗根節點座標。
    /// </summary>
    /// <param name="relativeTo">目標視覺節點指標，預設為 nullptr。</param>
    /// <returns>局部座標點。</returns>
    COMPAT_NODISCARD core::Point GetPosition(const core::Visual* relativeTo = nullptr) const {
        if (!relativeTo) {
            return m_windowPosition;
        }
        return relativeTo->PointToClient(m_windowPosition);
    }

    /// <summary>取得觸發之滑鼠按鍵。</summary>
    /// <returns>RawMouseButton 列舉值。</returns>
    COMPAT_NODISCARD pal::RawMouseButton GetButton() const noexcept { return m_button; }

    /// <summary>取得鍵盤修飾鍵狀態。</summary>
    /// <returns>KeyModifiers 旗標組合。</returns>
    COMPAT_NODISCARD pal::KeyModifiers GetModifiers() const noexcept { return m_modifiers; }

    /// <summary>取得連擊次數。</summary>
    /// <returns>連擊次數整數值。</returns>
    COMPAT_NODISCARD int32_t GetClickCount() const noexcept { return m_clickCount; }

    /// <summary>取得水平滾輪滾動量。</summary>
    /// <returns>水平滾動差值。</returns>
    COMPAT_NODISCARD int32_t GetWheelDeltaX() const noexcept { return m_wheelDeltaX; }

    /// <summary>取得垂直滾輪滾動量。</summary>
    /// <returns>垂直滾動差值。</returns>
    COMPAT_NODISCARD int32_t GetWheelDeltaY() const noexcept { return m_wheelDeltaY; }

    /// <summary>檢查是否為滑鼠左鍵。</summary>
    /// <returns>若為左鍵傳回 true。</returns>
    COMPAT_NODISCARD bool IsLeftButton() const noexcept { return m_button == pal::RawMouseButton::Left; }

    /// <summary>檢查是否為滑鼠右鍵。</summary>
    /// <returns>若為右鍵傳回 true。</returns>
    COMPAT_NODISCARD bool IsRightButton() const noexcept { return m_button == pal::RawMouseButton::Right; }

    /// <summary>檢查是否為滑鼠中鍵。</summary>
    /// <returns>若為中鍵傳回 true。</returns>
    COMPAT_NODISCARD bool IsMiddleButton() const noexcept { return m_button == pal::RawMouseButton::Middle; }

    /// <summary>檢查是否按下 Control 鍵。</summary>
    /// <returns>若按下傳回 true。</returns>
    COMPAT_NODISCARD bool HasControl() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Control); }

    /// <summary>檢查是否按下 Shift 鍵。</summary>
    /// <returns>若按下傳回 true。</returns>
    COMPAT_NODISCARD bool HasShift() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Shift); }

    /// <summary>檢查是否按下 Alt 鍵。</summary>
    /// <returns>若按下傳回 true。</returns>
    COMPAT_NODISCARD bool HasAlt() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Alt); }

private:
    core::Point          m_windowPosition{ 0.0f, 0.0f };
    pal::RawMouseButton  m_button = pal::RawMouseButton::None;
    pal::KeyModifiers    m_modifiers = pal::KeyModifiers::None;
    int32_t              m_clickCount = 1;
    int32_t              m_wheelDeltaX = 0;
    int32_t              m_wheelDeltaY = 0;
};

} // namespace input
} // namespace cppui
