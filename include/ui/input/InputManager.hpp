/**
 * @file InputManager.hpp
 * @brief 輸入管理中樞，負責底層原始輸入轉譯、路由事件分發、懸停追蹤與指標捕獲
 */

#pragma once

#include "compat/compat.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/input/InputTypes.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/FocusEventArgs.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/ILightDismissable.hpp"

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
    /// <summary>取得輸入管理器單例引用。</summary>
    /// <returns>InputManager 實例引用。</returns>
    static InputManager& Get() noexcept;

    /// <summary>預設建構子。</summary>
    InputManager() = default;

    /// <summary>預設解構子。</summary>
    ~InputManager() = default;

    // 禁止拷貝
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    /// <summary>
    /// 核心分發入口：接收底層視窗原始事件並轉發至視覺樹。
    /// </summary>
    /// <param name="root">視覺樹根節點引用。</param>
    /// <param name="rawEvent">底層平台原始事件結構。</param>
    void ProcessRawEvent(core::UIElement& root, const pal::RawInputEvent& rawEvent);

    // --- 游標懸停追蹤 ---

    /// <summary>取得當前懸停之 UI 元件指標。</summary>
    /// <returns>UIElement 指標，若無懸停則為 nullptr。</returns>
    COMPAT_NODISCARD core::UIElement* GetHoveredElement() const noexcept { return m_hoveredElement; }

    // --- 指標捕獲管理 ---

    /// <summary>
    /// 設定指定元件捕獲所有指標事件。
    /// </summary>
    /// <param name="element">欲捕獲指標之 UI 元件指標。</param>
    /// <returns>若捕獲成功傳回 true。</returns>
    bool CapturePointer(core::UIElement* element);

    /// <summary>
    /// 釋放指標捕獲。
    /// </summary>
    /// <param name="element">預期釋放之元件指標，傳入 nullptr 表示無條件釋放。</param>
    void ReleasePointerCapture(core::UIElement* element = nullptr);

    /// <summary>取得目前捕獲指標之元件指標。</summary>
    /// <returns>UIElement 指標，若無捕獲則為 nullptr。</returns>
    COMPAT_NODISCARD core::UIElement* GetCapturedElement() const noexcept { return m_capturedElement; }

    // --- 狀態重設 (如視窗失焦或關閉時) ---

    /// <summary>重設輸入管理器狀態 (清除懸停與捕獲)。</summary>
    void ResetState();

    /// <summary>
    /// 元件解構通知，防止懸空游標或捕獲指標。
    /// </summary>
    /// <param name="element">即將解構之 UI 元件指標。</param>
    void NotifyElementDestroyed(core::UIElement* element) noexcept;

    // --- 浮層彈出控制項 (Popup Light-Dismiss) ---

    /// <summary>註冊當前已開啟之浮層 (用於 Light-Dismiss 監聽)。</summary>
    /// <param name="popup">開啟之 ILightDismissable 指標。</param>
    void RegisterOpenPopup(ILightDismissable* popup);

    /// <summary>註銷已關閉之浮層。</summary>
    /// <param name="popup">關閉之 ILightDismissable 指標。</param>
    void UnregisterOpenPopup(ILightDismissable* popup);

    /// <summary>取得當前所有已開啟之浮層清單。</summary>
    /// <returns>ILightDismissable 指標陣列常數引用。</returns>
    COMPAT_NODISCARD const std::vector<ILightDismissable*>& GetOpenPopups() const noexcept { return m_openPopups; }

    // 分流事件處理輔助方法

    /// <summary>處理滑鼠移動原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <param name="mouseData">滑鼠事件原始資料。</param>
    /// <param name="mods">修飾鍵狀態。</param>
    void ProcessMouseMove(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);

    /// <summary>處理滑鼠按鍵下壓原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <param name="mouseData">滑鼠事件原始資料。</param>
    /// <param name="mods">修飾鍵狀態。</param>
    void ProcessMouseDown(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);

    /// <summary>處理滑鼠按鍵釋放原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <param name="mouseData">滑鼠事件原始資料。</param>
    /// <param name="mods">修飾鍵狀態。</param>
    void ProcessMouseUp(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);

    /// <summary>處理滑鼠滾輪滾動原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <param name="mouseData">滑鼠事件原始資料。</param>
    /// <param name="mods">修飾鍵狀態。</param>
    void ProcessMouseWheel(core::UIElement& root, const pal::RawMouseEventData& mouseData, pal::KeyModifiers mods);

    /// <summary>處理鍵盤按鍵下壓原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <param name="keyData">按鍵原始資料。</param>
    /// <param name="mods">修飾鍵狀態。</param>
    void ProcessKeyDown(core::UIElement& root, const pal::RawKeyEventData& keyData, pal::KeyModifiers mods);

    /// <summary>處理鍵盤按鍵釋放原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <param name="keyData">按鍵原始資料。</param>
    /// <param name="mods">修飾鍵狀態。</param>
    void ProcessKeyUp(core::UIElement& root, const pal::RawKeyEventData& keyData, pal::KeyModifiers mods);

    /// <summary>處理鍵盤文字輸入原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <param name="textData">文字輸入原始資料。</param>
    void ProcessTextInput(core::UIElement& root, const pal::RawTextInputData& textData);

    /// <summary>處理 IME 輸入法組字/定稿原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    /// <param name="imeData">IME 事件原始資料。</param>
    void ProcessImeComposition(core::UIElement& root, const pal::RawImeData& imeData);

    /// <summary>處理 IME 結束組字原始事件。</summary>
    /// <param name="root">視覺樹根節點。</param>
    void ProcessImeEndComposition(core::UIElement& root);

private:
    void UpdateHover(core::UIElement* newHover, core::Point windowPos, pal::KeyModifiers mods);

    core::UIElement* m_hoveredElement = nullptr;
    core::UIElement* m_capturedElement = nullptr;
    std::vector<ILightDismissable*> m_openPopups;
};

} // namespace input
} // namespace cppui
