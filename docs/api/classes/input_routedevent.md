# RoutedEvent 與路由策略 (cppui::input::RoutedEvent)

定義路由事件（Routed Events）的元資料識別元、事件傳播策略（Routing Strategy）與系統標準預先定義事件集合。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::input`](../namespaces/cppui_input.md)
* **標頭檔**：`#include <ui/input/RoutedEvent.hpp>`
* **模組**：`CPPUI Input`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace input {

/// <summary>
/// 路由傳播策略
/// </summary>
enum class RoutingStrategy : uint8_t {
    Tunnel,  // 隧道階段：由視覺樹根節點 (Root) 向下傳播至事件目標 (Target)
    Bubble,  // 冒泡階段：由事件目標 (Target) 向上傳播至視覺樹根節點 (Root)
    Direct   // 直接模式：僅由事件目標自身處理，不進行樹狀傳播
};

/// <summary>
/// 路由事件識別元
/// </summary>
class RoutedEvent {
public:
    RoutedEvent(size_t id, compat::string_view name, RoutingStrategy strategy) noexcept;

    COMPAT_NODISCARD size_t GetId() const noexcept;
    COMPAT_NODISCARD compat::string_view GetName() const noexcept;
    COMPAT_NODISCARD RoutingStrategy GetRoutingStrategy() const noexcept;

    bool operator==(const RoutedEvent& other) const noexcept;
    bool operator!=(const RoutedEvent& other) const noexcept;

    // 註冊自訂路由事件
    static const RoutedEvent& Register(compat::string_view name, RoutingStrategy strategy);
};

/// <summary>
/// 系統預先定義標準路由事件
/// </summary>
struct StandardEvents {
    // 指標移動事件
    static const RoutedEvent PreviewPointerMoved;
    static const RoutedEvent PointerMoved;

    // 指標按壓事件
    static const RoutedEvent PreviewPointerPressed;
    static const RoutedEvent PointerPressed;

    // 指標釋放事件
    static const RoutedEvent PreviewPointerReleased;
    static const RoutedEvent PointerReleased;

    // 指標滾輪事件
    static const RoutedEvent PreviewPointerWheel;
    static const RoutedEvent PointerWheel;

    // 指標進出事件 (Direct 直接路由)
    static const RoutedEvent PointerEntered;
    static const RoutedEvent PointerExited;

    // 鍵盤與輸入事件
    static const RoutedEvent PreviewKeyDown;
    static const RoutedEvent KeyDown;
    static const RoutedEvent PreviewKeyUp;
    static const RoutedEvent KeyUp;
    static const RoutedEvent PreviewTextInput;
    static const RoutedEvent TextInput;

    // 焦點事件 (Direct 直接路由)
    static const RoutedEvent GotFocus;
    static const RoutedEvent LostFocus;
};

} // namespace input
} // namespace cppui
```

---

## 簡介 (Description)

在傳統桌面 UI 框架中，事件通常直接發送至單一控制項。CPPUI 採用類似 WPF 與 Avalonia 的**路由事件體系**：
一個事件在發生時可以在整個視覺樹（Visual Tree）中依照特定策略傳播，讓父節點能夠在事件到達子節點前進行預先攔截（Tunneling），或在子節點處理後接收匯總通知（Bubbling）。

### 傳播策略說明

1. **`Tunnel` (隧道階段 / 預先攔截)**：
   - 命名慣例通常冠以 `Preview*` 前綴（如 `PreviewPointerPressed`）。
   - 事件從視覺樹最頂層的根節點（如視窗主畫布）開始，由上而下依序傳遞至事件目標節點。
   - 用途：父容器預先檢查或阻斷使用者輸入。
2. **`Bubble` (冒泡階段 / 主邏輯傳播)**：
   - 命名為常規名稱（如 `PointerPressed`）。
   - 事件由命中之目標葉節點開始，由下而上依序向父節點傳遞，直到抵達視覺樹根節點。
   - 用途：自訂複合按鈕內部的圖示或文字被點擊時，外層按鈕本體可接收到點擊信號。
3. **`Direct` (直接路由)**：
   - 不進行上下傳播，事件僅直接送達目標節點。
   - 典型代表：`PointerEntered`、`PointerExited`、`GotFocus`、`LostFocus`。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/input/RoutedEvent.hpp"

void RegisterCustomEvent() {
    using namespace cppui::input;

    // 動態註冊自訂冒泡事件
    const RoutedEvent& customClick = RoutedEvent::Register("CustomClick", RoutingStrategy::Bubble);

    compat::println("已註冊事件: {} [ID: {}], 策略: 冒泡", 
        customClick.GetName(), customClick.GetId());
}
```

---

## 請參閱 (See Also)
* [`RoutedEventArgs` 類別參考](input_routedeventargs.md)
* [`EventRoute` 類別參考](input_eventroute.md)
* [`UIElement` 類別參考](core_uielement.md)
* [輸入管線與路由事件指南](../guides/input_and_routed_events.md)
