/**
 * @file FocusManager.hpp
 * @brief 全域焦點管理中樞，維護焦點狀態、分發焦點事件與 Tab 導航
 */

#pragma once

#include <vector>
#include "compat/compat.hpp"
#include "ui/input/InputTypes.hpp"

namespace cppui {
namespace core {
    class UIElement;
} // namespace core

namespace input {

/// <summary>
/// 全域焦點管理中樞
/// 負責維護當前焦點元件、處理 GotFocus / LostFocus 事件、並執行 Tab / Shift+Tab 鍵盤導航尋路演算法。
/// </summary>
class FocusManager {
public:
    /// <summary>取得焦點管理器單例引用。</summary>
    /// <returns>FocusManager 實例引用。</returns>
    static FocusManager& Get() noexcept;

    /// <summary>預設建構子。</summary>
    FocusManager() = default;

    /// <summary>預設解構子。</summary>
    ~FocusManager() = default;

    // 禁止拷貝
    FocusManager(const FocusManager&) = delete;
    FocusManager& operator=(const FocusManager&) = delete;

    /// <summary>取得當前擁有焦點之 UI 元件指標。</summary>
    /// <returns>UIElement 指標，若無焦點則為 nullptr。</returns>
    COMPAT_NODISCARD core::UIElement* GetFocusedElement() const noexcept { return m_focusedElement; }

    /// <summary>
    /// 切換鍵盤焦點至指定元件；傳入 nullptr 則清空焦點。
    /// </summary>
    /// <param name="element">目標 UI 元件指標。</param>
    /// <returns>若焦點成功切換傳回 true，否則傳回 false。</returns>
    bool SetFocus(core::UIElement* element);

    /// <summary>
    /// 清除當前焦點。
    /// </summary>
    void ClearFocus();

    /// <summary>
    /// 執行 Tab 鍵盤導航尋路切換焦點。
    /// </summary>
    /// <param name="root">導航根節點。</param>
    /// <param name="forward">true 為 Tab 順向；false 為 Shift+Tab 逆向。</param>
    /// <returns>若成功切換至下一個焦點元件傳回 true。</returns>
    bool MoveFocus(core::UIElement* root, bool forward = true);

    /// <summary>
    /// 元件解構通知，防止懸空指標。
    /// </summary>
    /// <param name="element">即將解構之 UI 元件指標。</param>
    void NotifyElementDestroyed(core::UIElement* element) noexcept;

private:
    void CollectFocusableElements(core::UIElement* current, std::vector<core::UIElement*>& outList);

    core::UIElement* m_focusedElement = nullptr;
};

} // namespace input
} // namespace cppui
