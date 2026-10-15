# FocusEventArgs 類別 (cppui::input::FocusEventArgs)

封裝焦點獲得（`GotFocus`）與焦點失去（`LostFocus`）的直接路由事件參數，攜帶先前獲焦元件與新獲焦元件的指標。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::input`](../namespaces/cppui_input.md)
* **標頭檔**：`#include <ui/input/FocusEventArgs.hpp>`
* **模組**：`CPPUI Input`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace input {

class FocusEventArgs : public RoutedEventArgs {
public:
    FocusEventArgs(const RoutedEvent& routedEvent,
                   core::UIElement*   source,
                   core::UIElement*   oldFocus,
                   core::UIElement*   newFocus) noexcept;

    COMPAT_NODISCARD core::UIElement* GetOldFocus() const noexcept;
    COMPAT_NODISCARD core::UIElement* GetNewFocus() const noexcept;

private:
    core::UIElement* m_oldFocus = nullptr;
    core::UIElement* m_newFocus = nullptr;
};

using FocusEventHandler = std::function<void(FocusEventArgs&)>;

} // namespace input
} // namespace cppui
```

---

## 簡介 (Description)

當使用者的操作（滑鼠點擊、Tab 鍵切換或呼叫 `FocusManager::SetFocus`）導致輸入焦點在不同元件之間轉移時，`FocusManager` 會以**步調一致 (Lockstep)** 的順序分發焦點事件：
1. 先向失去焦點的元件派發 `StandardEvents::LostFocus`（`GetOldFocus() == 失焦元件`, `GetNewFocus() == 獲焦元件`）。
2. 再向獲得焦點的元件派發 `StandardEvents::GotFocus`（`GetOldFocus() == 失焦元件`, `GetNewFocus() == 獲焦元件`）。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/input/FocusEventArgs.hpp"
#include "ui/core/UIElement.hpp"

void SetupFocusVisuals(cppui::core::UIElement& control) {
    control.OnGotFocus([](cppui::input::FocusEventArgs&) {
        compat::println("元件獲得焦點，繪製外圍霓虹光暈！");
    });

    control.OnLostFocus([](cppui::input::FocusEventArgs&) {
        compat::println("元件失去焦點，恢復預設邊框。");
    });
}
```

---

## 請參閱 (See Also)
* [`FocusManager` 類別參考](input_focusmanager.md)
* [`RoutedEventArgs` 基底參考](input_routedeventargs.md)
* [`UIElement` 類別參考](core_uielement.md)
