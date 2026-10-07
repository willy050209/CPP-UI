#pragma once

#include "compat/compat.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/input/InputTypes.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/FocusEventArgs.hpp"
#include "ui/input/FocusManager.hpp"

namespace cppui {
namespace core {
    class UIElement;
} // namespace core

namespace input {

/// <summary>
/// 輸入管理中樞
/// 負責將來自 PAL (Platform Abstraction Layer) 的 RawInputEvent 轉譯並派發為樹狀路由事件，
/// 同時管理指標懸停 (Hover Tracking) 與指標捕獲 (Pointer Capture)。
/// </summary>
class InputManager {
public:
    static InputManager& Get() noexcept;

    InputManager() = default;
    ~InputManager() = default;

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    /// <summary>
    /// 核心分發入口：接收底層視窗原始事件並轉發至視覺樹
    /// </summary>
    void ProcessRawEvent(core::UIElement& root, const pal::RawInputEvent& rawEvent);

    // --- 游標懸停追蹤 ---
    COMPAT_NODISCARD core::UIElement* GetHoveredElement() const noexcept { return m_hoveredElement; }

    // --- 指標捕獲管理 ---
    bool CapturePointer(core::UIElement* element);
    void ReleasePointerCapture(core::UIElement* element = nullptr);
    COMPAT_NODISCARD core::UIElement* GetCapturedElement() const noexcept { return m_capturedElement; }

    // --- 狀態重設 (如視窗失焦或關閉時) ---
    void ResetState();

    /// <summary>
    /// 元件解構通知，防止懸空游標或捕獲指標
    /// </summary>
    void NotifyElementDestroyed(core::UIElement* element) noexcept;

    // 分流事件處理輔助方法
    void ProcessMouseMove(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);
    void ProcessMouseDown(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);
    void ProcessMouseUp(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);
    void ProcessMouseWheel(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);
    void ProcessKeyDown(core::UIElement& root, const pal::RawKeyEventData& keyData, pal::KeyModifiers mods);
    void ProcessKeyUp(core::UIElement& root, const pal::RawKeyEventData& keyData, pal::KeyModifiers mods);
    void ProcessTextInput(core::UIElement& root, const pal::RawTextInputData& textData);

private:
    void UpdateHover(core::UIElement* newHover, core::Point windowPos, pal::KeyModifiers mods);

    core::UIElement* m_hoveredElement = nullptr;
    core::UIElement* m_capturedElement = nullptr;
};

} // namespace input
} // namespace cppui
