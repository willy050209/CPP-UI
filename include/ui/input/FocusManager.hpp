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
    static FocusManager& Get() noexcept;

    FocusManager() = default;
    ~FocusManager() = default;

    FocusManager(const FocusManager&) = delete;
    FocusManager& operator=(const FocusManager&) = delete;

    COMPAT_NODISCARD core::UIElement* GetFocusedElement() const noexcept { return m_focusedElement; }

    /// <summary>
    /// 切換鍵盤焦點至指定元件；傳入 nullptr 則清空焦點
    /// </summary>
    bool SetFocus(core::UIElement* element);

    /// <summary>
    /// 清除當前焦點
    /// </summary>
    void ClearFocus();

    /// <summary>
    /// Tab 鍵導航尋路 (forward=true 為 Tab 順向；forward=false 為 Shift+Tab 逆向)
    /// </summary>
    bool MoveFocus(core::UIElement* root, bool forward = true);

    /// <summary>
    /// 元件解構通知，防止懸空指標
    /// </summary>
    void NotifyElementDestroyed(core::UIElement* element) noexcept;

private:
    void CollectFocusableElements(core::UIElement* current, std::vector<core::UIElement*>& outList);

    core::UIElement* m_focusedElement = nullptr;
};

} // namespace input
} // namespace cppui
