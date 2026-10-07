# UIElement 類別 (cppui::core::UIElement)

- **命名空間**: [`cppui::core`](../namespaces/cppui_core.md)
- **標頭檔**: `include/ui/core/UIElement.hpp`
- **繼承關係**: 繼承自 [`Visual`](core_visual.md)

---

## 概述

`UIElement` 是具備雙階段排版管線（`Measure` -> `Arrange`）、約束幾何運算（Margin, Alignment, Min/Max Constraint）、精確髒標記追蹤（Dirty Tracking）以及現代路由事件派發與焦點管理的核心 UI 抽象基底類別。

---

## 核心成員方法

### 幾何與佈局約束屬性
- `GetMargin() / SetMargin(const Thickness& margin)`: 外邊距幾何。
- `GetWidth() / SetWidth(float width)`: 顯式目標寬度（`< 0` 表示 Auto）。
- `GetHeight() / SetHeight(float height)`: 顯式目標高度（`< 0` 表示 Auto）。
- `GetMinWidth() / SetMinWidth(float minWidth)`: 最小寬度約束。
- `GetMaxWidth() / SetMaxWidth(float maxWidth)`: 最大寬度約束。
- `GetHorizontalAlignment() / SetHorizontalAlignment(layout::HorizontalAlignment alignment)`: 水平對齊（Left, Center, Right, Stretch）。
- `GetVerticalAlignment() / SetVerticalAlignment(layout::VerticalAlignment alignment)`: 垂直對齊（Top, Center, Bottom, Stretch）。
- `GetVisibility() / SetVisibility(layout::Visibility visibility)`: 可見度（Visible, Hidden, Collapsed）。

### 髒標記狀態與隔離性
- `void InvalidateMeasure()`: 將 Measure 與 Arrange 狀態標記為髒，並沿視覺樹向上冒泡至父容器。
- `void InvalidateArrange()`: 將 Arrange 標記為髒，保留 Measure 快取結果。
- `void InvalidateVisual()`: 僅標記重繪需求，**絕不**使 Measure 與 Arrange 失效。

### 雙階段排版管線
- `void Measure(Size availableSize)`: 執行第一階段測量，計算並快取 `DesiredSize`。
- `void Arrange(Rect finalRect)`: 執行第二階段排列，依對齊約束配置 `RenderBounds` 與 `VisualOffset`。
- `virtual Size MeasureOverride(Size availableSize)`: 子類別內容度量實作點。
- `virtual void ArrangeOverride(Size finalSize)`: 子類別子元件排列實作點。

### 路由事件註冊與派發
- `size_t AddHandler(const input::RoutedEvent& routedEvent, input::RoutedEventHandler handler, bool handledEventsToo = false)`: 註冊事件監聽常式，支援 `handledEventsToo` 參數。
- `void RemoveHandler(const input::RoutedEvent& routedEvent, size_t handlerId)`: 依據註冊 Token 解除事件監聽。
- `void RaiseEvent(input::RoutedEventArgs& args)`: 以當前節點為目標引發路由事件。

### 指標事件快捷回呼 (Pointer Shortcuts)
- `size_t OnPointerPressed(input::PointerEventHandler handler)`: 註冊滑鼠/觸控按壓事件 (`PointerPressed`)。
- `size_t OnPointerReleased(input::PointerEventHandler handler)`: 註冊滑鼠/觸控釋放事件 (`PointerReleased`)。
- `size_t OnPointerMoved(input::PointerEventHandler handler)`: 註冊游標移動事件 (`PointerMoved`)。
- `size_t OnPointerEntered(input::PointerEventHandler handler)`: 註冊游標進入邊界事件 (`PointerEntered`)。
- `size_t OnPointerExited(input::PointerEventHandler handler)`: 註冊游標離開邊界事件 (`PointerExited`)。

### 鍵盤與輸入快捷回呼 (Keyboard & Text Shortcuts)
- `size_t OnKeyDown(input::KeyEventHandler handler)`: 註冊實體按鍵按下事件 (`KeyDown`)。
- `size_t OnKeyUp(input::KeyEventHandler handler)`: 註冊實體按鍵放開事件 (`KeyUp`)。
- `size_t OnTextInput(input::TextInputEventHandler handler)`: 註冊文字合成輸入事件 (`TextInput`)。

### 焦點狀態與 Tab 導航
- `COMPAT_NODISCARD bool IsFocusable() const noexcept / void SetFocusable(bool focusable) noexcept`: 查詢或設定該元件是否具備獲焦能力。
- `COMPAT_NODISCARD bool IsFocused() const noexcept`: 查詢當前元件是否持有鍵盤焦點。
- `COMPAT_NODISCARD int32_t GetTabIndex() const noexcept / void SetTabIndex(int32_t index) noexcept`: 查詢或指定 Tab 鍵巡訪權重（預設為 0）。
- `COMPAT_NODISCARD bool IsTabStop() const noexcept / void SetTabStop(bool tabStop) noexcept`: 查詢或設定是否參與 Tab 鍵輪詢導航。
- `size_t OnGotFocus(input::FocusEventHandler handler)`: 註冊獲得焦點事件 (`GotFocus`)。
- `size_t OnLostFocus(input::FocusEventHandler handler)`: 註冊失去焦點事件 (`LostFocus`)。

---

## 請參閱 (See Also)
* [`Visual` 類別參考](core_visual.md)
* [`RoutedEvent` 類別參考](input_routedevent.md)
* [`PointerEventArgs` 類別參考](input_pointereventargs.md)
* [`FocusManager` 類別參考](input_focusmanager.md)
* [輸入管線與路由事件指南](../guides/input_and_routed_events.md)
