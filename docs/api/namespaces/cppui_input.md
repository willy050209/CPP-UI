# cppui::input 命名空間 (Namespace)

`cppui::input` 命名空間封裝了 CPPUI 框架的事件路由系統、指標與鍵盤輸入管線、焦點管理與指標捕獲機制。

---

## 簡介 (Description)

`cppui::input` 模組是 CPPUI 達到高互動性與現代化桌面體驗的核心。它提供了一套完整的路由事件（Routed Events）基礎架構，支援微軟 WPF 與 Avalonia 風格的雙向（隧道 Tunneling + 冒泡 Bubbling）事件派發、全域焦點轉移與鍵盤 Tab 導航。

### 核心設計原則
1. **樹狀雙向傳播**：任何輸入操作均先自視覺樹根部向下進行預先攔截檢查（`Preview*`），再由目標葉節點向上冒泡。
2. **事件短路阻斷**：支援 `e.SetHandled(true)` 隨時終止後續處理器調用。
3. **零懸空指標保證**：全域焦點中樞 `FocusManager` 與輸入中樞 `InputManager` 深度整合元件解構通知，嚴防 Use-after-free 漏洞。
4. **高分屏精確對齊**：與 `CoordinateTransform` 緊密配合，分發給命中測試的坐標始終與客戶區排版空間 1:1 精確對齊。

---

## 類別與結構目錄 (Types Directory)

| 結構 / 類別 (Type) | 簡介與職責 |
| :--- | :--- |
| [`RoutedEvent`](../classes/input_routedevent.md) | 路由事件識別元與元資料，定義傳播策略（Tunnel, Bubble, Direct）。 |
| [`RoutedEventArgs`](../classes/input_routedeventargs.md) | 路由事件參數抽象基底，提供 `Handled` 狀態標記與來源節點資訊。 |
| [`EventRoute`](../classes/input_eventroute.md) | 路由路徑建構器與雙向（隧道 $\rightarrow$ 冒泡）派發管線執行器。 |
| [`PointerEventArgs`](../classes/input_pointereventargs.md) | 封裝滑鼠移動、按鍵按下/釋放、滾輪滾動與雙擊計數之事件參數。 |
| [`KeyEventArgs`](../classes/input_keyeventargs.md) | 封裝鍵盤實體按鍵（KeyDown / KeyUp）、虛擬鍵碼與修飾鍵狀態。 |
| [`TextInputEventArgs`](../classes/input_keyeventargs.md) | 封裝經 IME 輸入法或鍵盤合成之 Unicode UTF-32 / UTF-8 文字輸入。 |
| [`FocusEventArgs`](../classes/input_focuseventargs.md) | 封裝焦點獲得（`GotFocus`）與失去（`LostFocus`）之直接路由事件。 |
| [`FocusManager`](../classes/input_focusmanager.md) | 全域焦點管理單例，提供焦點設定、清除與 Tab / Shift+Tab 導航演算法。 |
| [`InputManager`](../classes/input_inputmanager.md) | 輸入中樞單例，負責原生事件轉譯、懸停追蹤（Hover）與指標捕獲（Capture）。 |

---

## 標準路由事件速查表 (`StandardEvents`)

| 事件名稱 | 傳播策略 | 觸發時機與用途 |
| :--- | :--- | :--- |
| `PreviewPointerPressed` / `PointerPressed` | Tunnel / Bubble | 滑鼠或觸控指標按下，常用於點擊判定與即時獲焦。 |
| `PreviewPointerReleased` / `PointerReleased` | Tunnel / Bubble | 滑鼠或觸控指標釋放，常用於完成按鈕點擊或結束拖曳。 |
| `PreviewPointerMoved` / `PointerMoved` | Tunnel / Bubble | 游標於視窗內移動，更新局部坐標或拖曳物體。 |
| `PreviewPointerWheel` / `PointerWheel` | Tunnel / Bubble | 滑鼠滾輪滾動，驅動捲動面板或數值增減。 |
| `PointerEntered` / `PointerExited` | Direct | 游標進入或離開元件邊界，驅動 Hover 視覺變光反饋。 |
| `PreviewKeyDown` / `KeyDown` | Tunnel / Bubble | 實體鍵盤按下，驅動快捷鍵或導航。 |
| `PreviewKeyUp` / `KeyUp` | Tunnel / Bubble | 實體鍵盤放開。 |
| `PreviewTextInput` / `TextInput` | Tunnel / Bubble | 文字合成輸入，驅動文字輸入框輸入。 |
| `GotFocus` / `LostFocus` | Direct | 元件獲得或失去鍵盤輸入焦點，更新焦點框視覺。 |

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/core/UIElement.hpp"

void SetupInteractiveButton(cppui::core::UIElement& button) {
    using namespace cppui::input;

    // 1. 懸停變光
    button.OnPointerEntered([&](PointerEventArgs&) {
        compat::println("游標進入按鈕！");
    });
    button.OnPointerExited([&](PointerEventArgs&) {
        compat::println("游標離開按鈕！");
    });

    // 2. 點擊處理
    button.OnPointerPressed([&](PointerEventArgs& e) {
        if (e.IsLeftButton()) {
            compat::println("按鈕被按下！");
            e.SetHandled(true); // 標記已處理，防止父容器重疊觸發
        }
    });
}
```

---

## 請參閱 (See Also)
* [CPPUI API 首頁](../README.md)
* [`UIElement` 類別參考](../classes/core_uielement.md)
* [輸入管線與路由事件指南](../guides/input_and_routed_events.md)
