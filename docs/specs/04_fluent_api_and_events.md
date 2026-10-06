# CPPUI 設計規範 04：Fluent RAD API 與路由事件標準 (Fluent API & Event Standards)

## 1. 現代 C++ Fluent DSL 設計理念

傳統 C++ UI 框架（如 Qt 或 wxWidgets）通常充斥著繁複的巨集（`SLOT`/`SIGNAL`）、過度的動態型別指標傳遞、或冗長的 `new` 呼叫；而現代框架（如 WPF/Avalonia）則深度依賴 XAML 標記語言與執行階段資料綁定引擎。

CPPUI 追求 **「Avalonia 架構核心 + Windows Forms 直覺心智模型 + 現代 C++ 型別安全」**：
1. **宣告式純代碼構建（Code-Only UI）**：無需外部 XML 編譯器，支援編譯時期語法檢查與 IDE 自動補全。
2. **極速事件掛載**：支援直接在宣告式階層樹中以 Lambda 表達式連結邏輯。
3. **高效率值語意與移動語意**：透過現代 C++ 移動語意構建元件樹，杜絕不必要的堆積指標分配。

---

## 2. Fluent Builder 語法規範

### 典型使用者視角程式碼範例：
```cpp
auto myWindow = Window::Create()
    .Title("CPPUI Quick Start")
    .Size(800, 600)
    .Content(
        VStack({
            Label("Welcome to CPPUI")
                .FontSize(24)
                .Foreground(Colors::Black),

            Button("Click Me")
                .Background(Colors::SteelBlue)
                .Padding(12, 8)
                .OnClick([this](auto& e) {
                    HandleSubmit();
                }),

            HStack({
                TextBox().Placeholder("Enter your name...").Width(200),
                Button("Search").OnClick([](auto&) { /* ... */ })
            }).Spacing(8)
        })
        .Spacing(16)
        .Padding(24)
    );
```

### Builder 方法設計規則：
1. **屬性鏈式配置**：每個屬性設定器皆傳回該元件的右值參照 `Type&&` 或參考 `Type&`，允許流暢鏈式調用：
   ```cpp
   Button&& Background(Color color) && {
       m_background = color;
       return std::move(*this);
   }
   ```
2. **容器語法**：容器（如 `VStack`, `HStack`, `Grid`）接受 `std::initializer_list` 或可變參數範本，以大括號 `{ ... }` 簡潔宣告子元素清單。

---

## 3. 路由事件體系 (Routed Events Pipeline)

事件系統依循 WPF / Avalonia 的路由架構，確保複合控制項（如包含圖標與文字的按鈕）能夠自然、預期地轉發事件。

```mermaid
graph TD
    Root["Root Window"]
    Container["VStack Container"]
    Btn["Button Control"]
    Leaf["Label (Inside Button)"]

    Root -->|1. Tunneling (PreviewPointerDown)| Container
    Container -->|2. Tunneling| Btn
    Btn -->|3. Tunneling| Leaf

    Leaf -->|4. Bubbling (PointerDown / Click)| Btn
    Btn -->|5. Bubbling (Click Handled!)| Container
    Container -->|6. Bubbling (Handled)| Root
```

### 路由策略三巨頭：
1. **Tunneling (隧道/預檢階段)**：
   - 命名慣例：以 `Preview` 為前綴（例如 `PreviewPointerDown`, `PreviewKeyDown`）。
   - 方向：從根節點（Window）沿著視覺樹路徑向下傳播至目標命中節點（Target）。
   - 用途：供父容器預先攔截或過濾特殊手勢。
2. **Bubbling (冒泡階段)**：
   - 命名慣例：無 Preview 前綴（例如 `PointerDown`, `PointerUp`, `Click`）。
   - 方向：從目標節點（Target）向上沿著父節點鏈一路傳播至根節點。
   - 用途：一般的業務事件處理（如按鈕觸發、表單送出）。
3. **Direct (直接階段)**：
   - 不進行樹狀傳播，直接發送給目標節點（例如 `MouseEnter`, `MouseLeave`）。

### 事件參數與短路機制：
```cpp
namespace cppui::input {

struct RoutedEventArgs {
    UIElement* source = nullptr;   // 原始引發事件的節點
    bool       handled = false;    // 短路標記

    void SetHandled() noexcept { handled = true; }
};

} // namespace cppui::input
```
* 當任何節點設定 `e.handled = true` 時，後續的冒泡階段將自動停止傳播（除非監聽器明確註冊 `handledEventsToo = true`）。

---

## 4. 點擊測試與焦點管理 (Hit Testing & Focus)

1. **反向 Z-Order 幾何命中測試 (Hit-Testing)**：
   - 輸入分發器沿視覺樹向下查詢，優先檢測 Z-Order 最高的最頂層子節點。
   - 支援 `IsHitTestVisible` 旗標控制穿透效果。
2. **焦點管理 (FocusManager)**：
   - 全域維護 `FocusedElement`。
   - 支援 Tab 鍵焦點鏈循序導航（`TabIndex` / `IsTabStop`）。
   - 當焦點切換時，精確觸發 `LostFocus` 與 `GotFocus` 路由事件。
