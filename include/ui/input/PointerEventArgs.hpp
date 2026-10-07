#pragma once

#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/Visual.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 滑鼠與指標事件參數
/// </summary>
class PointerEventArgs : public RoutedEventArgs {
public:
    PointerEventArgs() = default;

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

    COMPAT_NODISCARD core::Point GetWindowPosition() const noexcept {
        return m_windowPosition;
    }

    /// <summary>
    /// 取得相對於指定視覺節點的局部座標；若傳入 nullptr 則回傳視窗根節點座標
    /// </summary>
    COMPAT_NODISCARD core::Point GetPosition(const core::Visual* relativeTo = nullptr) const {
        if (!relativeTo) {
            return m_windowPosition;
        }
        return relativeTo->PointToClient(m_windowPosition);
    }

    COMPAT_NODISCARD pal::RawMouseButton GetButton() const noexcept { return m_button; }
    COMPAT_NODISCARD pal::KeyModifiers GetModifiers() const noexcept { return m_modifiers; }
    COMPAT_NODISCARD int32_t GetClickCount() const noexcept { return m_clickCount; }
    COMPAT_NODISCARD int32_t GetWheelDeltaX() const noexcept { return m_wheelDeltaX; }
    COMPAT_NODISCARD int32_t GetWheelDeltaY() const noexcept { return m_wheelDeltaY; }

    COMPAT_NODISCARD bool IsLeftButton() const noexcept { return m_button == pal::RawMouseButton::Left; }
    COMPAT_NODISCARD bool IsRightButton() const noexcept { return m_button == pal::RawMouseButton::Right; }
    COMPAT_NODISCARD bool IsMiddleButton() const noexcept { return m_button == pal::RawMouseButton::Middle; }

    COMPAT_NODISCARD bool HasControl() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Control); }
    COMPAT_NODISCARD bool HasShift() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Shift); }
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
