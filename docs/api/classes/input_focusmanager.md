# FocusManager 類別 (cppui::input::FocusManager)

全域焦點中樞管理單例（Singleton），負責當前獲焦元件追蹤、焦點轉移、Tab / Shift+Tab 巡訪演算法以及焦點生命週期維護。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::input`](../namespaces/cppui_input.md)
* **標頭檔**：`#include <ui/input/FocusManager.hpp>`
* **模組**：`CPPUI Input`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace input {

class FocusManager {
public:
    static FocusManager& Get() noexcept;

    // --- 焦點設定與查詢 ---
    bool SetFocus(core::UIElement* element);
    void ClearFocus();
    COMPAT_NODISCARD core::UIElement* GetFocusedElement() const noexcept;

    // --- Tab 鍵鍵盤焦點尋路演算法 ---
    bool MoveFocus(core::UIElement* root, bool forward = true);

    // --- 物件解構通知 (防止懸空指標) ---
    void NotifyElementDestroyed(core::UIElement* element) noexcept;

private:
    FocusManager() = default;
    ~FocusManager() = default;

    core::UIElement* m_focusedElement = nullptr;
};

} // namespace input
} // namespace cppui
```

---

## 簡介 (Description)

鍵盤事件與文字輸入只會被分發給目前處於「獲焦狀態」的單一元件。`FocusManager` 統一管理全域焦點生命週期：

### 焦點轉移規則 (`SetFocus`)
1. 若目標元件未設置 `IsFocusable() == true` 或目標為不可見/已停用元件，轉移將被拒絕並回傳 `false`。
2. 若目標與當前獲焦元件相同，直接回傳 `true`，不重複觸發事件。
3. 轉移成功時：
   - 舊元件解除焦點 (`SetFocusedInternal(false)`)，觸發 `LostFocus`。
   - 新元件獲得焦點 (`SetFocusedInternal(true)`)，觸發 `GotFocus`。

### Tab 鍵尋路演算法 (`MoveFocus`)
當使用者按下 `Tab`（`forward = true`）或 `Shift + Tab`（`forward = false`）時：
1. `MoveFocus` 遍歷 `root` 下所有滿足 `IsFocusable() && IsTabStop()` 的元件。
2. 依據 `GetTabIndex()` 數值進行升冪排序（若數值相同則依照視覺樹走訪順序排序）。
3. 自動計算下一個焦點目標，若到達末端則自動循環（Wrap-around）回起點。

### 懸空指標保護 (`NotifyElementDestroyed`)
當獲焦元件在視覺樹重構或解構時，自動將 `m_focusedElement` 設為 `nullptr`，徹底防止 Use-after-free 崩潰。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/core/UIElement.hpp"

void HandleTabKey(cppui::core::UIElement* root, bool isShiftPressed) {
    using namespace cppui::input;

    // 依據 Shift 鍵決定前進或後退導航
    bool moved = FocusManager::Get().MoveFocus(root, !isShiftPressed);
    if (moved) {
        auto* focused = FocusManager::Get().GetFocusedElement();
        compat::println("焦點已轉移至元件 [TabIndex: {}]", focused->GetTabIndex());
    }
}
```

---

## 請參閱 (See Also)
* [`FocusEventArgs` 類別參考](input_focuseventargs.md)
* [`UIElement` 類別參考](core_uielement.md)
* [`InputManager` 類別參考](input_inputmanager.md)
