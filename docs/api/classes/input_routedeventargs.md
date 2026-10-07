# RoutedEventArgs 類別 (cppui::input::RoutedEventArgs)

所有路由事件參數的抽象基底類別，攜帶事件識別元、事件來源物件以及處理狀態旗標（`Handled`）。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::input`](../namespaces/cppui_input.md)
* **標頭檔**：`#include <ui/input/RoutedEventArgs.hpp>`
* **模組**：`CPPUI Input`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace input {

class RoutedEventArgs {
public:
    explicit RoutedEventArgs(const RoutedEvent& routedEvent) noexcept;
    RoutedEventArgs(const RoutedEvent& routedEvent, core::UIElement* source) noexcept;
    virtual ~RoutedEventArgs() = default;

    COMPAT_NODISCARD const RoutedEvent& GetRoutedEvent() const noexcept;

    COMPAT_NODISCARD bool IsHandled() const noexcept;
    void SetHandled(bool handled = true) noexcept;

    COMPAT_NODISCARD core::UIElement* GetSource() const noexcept;
    void SetSource(core::UIElement* source) noexcept;

    COMPAT_NODISCARD core::UIElement* GetOriginalSource() const noexcept;
    void SetOriginalSource(core::UIElement* originalSource) noexcept;

private:
    const RoutedEvent& m_routedEvent;
    core::UIElement*   m_source = nullptr;
    core::UIElement*   m_originalSource = nullptr;
    bool               m_handled = false;
};

using RoutedEventHandler = std::function<void(RoutedEventArgs&)>;

} // namespace input
} // namespace cppui
```

---

## 簡介 (Description)

`RoutedEventArgs` 是事件處理器（Event Handler）所接收的主要資料載體。它提供了關鍵的狀態控制機制：

### 事件阻斷機制 (`SetHandled(true)`)
- 當任何節點的事件處理器調用 `args.SetHandled(true)` 後，該事件被標記為「已處理」。
- 預設情況下，後續節點的常規事件處理器**將不再被觸發**，事件傳播即刻短路終止。
- 若特定高級元件（如日誌記錄器、無障礙輔助元件）希望即使在事件被標記為已處理時依然能收到通知，可於註冊時指定 `handledEventsToo = true`。

### 來源節點 (`Source` vs `OriginalSource`)
- **`OriginalSource`**：在視覺樹中最深層最先命中點擊的原生節點（如按鈕內部的某一特定文字區塊）。
- **`Source`**：引發或重新包裝該事件的高級邏輯元件（通常為按鈕本體）。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/input/RoutedEventArgs.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/core/UIElement.hpp"

void SetupInterception(cppui::core::UIElement& card, cppui::core::UIElement& button) {
    using namespace cppui::input;

    // 父卡片攔截 PreviewPointerPressed 事件
    card.AddHandler(StandardEvents::PreviewPointerPressed, [](RoutedEventArgs& e) {
        compat::println("卡片攔截了 PreviewPointerPressed 並標記 Handled=true！");
        e.SetHandled(true); // 短路傳播，子元件 button 將收不到 PointerPressed
    });

    // 子按鈕監聽常規 PointerPressed 事件
    button.AddHandler(StandardEvents::PointerPressed, [](RoutedEventArgs&) {
        compat::println("按鈕收到 PointerPressed！"); // 不會被執行
    });

    // 另一個監聽器：即使已被處理依然強制接收
    card.AddHandler(StandardEvents::PointerPressed, [](RoutedEventArgs&) {
        compat::println("日誌器：強制接收到已被處理的事件！");
    }, /*handledEventsToo=*/true);
}
```

---

## 請參閱 (See Also)
* [`RoutedEvent` 類別參考](input_routedevent.md)
* [`PointerEventArgs` 類別參考](input_pointereventargs.md)
* [`UIElement` 類別參考](core_uielement.md)
