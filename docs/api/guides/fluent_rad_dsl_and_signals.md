# 宣告式 Fluent RAD DSL 與響應式信號開發指南 (Fluent RAD DSL & Reactive Signals Guide)

本指南介紹如何使用 CPPUI Milestone 6 的宣告式 Fluent DSL 與 `Signal<T>` 響應式狀態管理系統，以不到 50 行的現代 C++ 程式碼快速構建結構清晰、具備動態雙向綁定與排版能力的桌面應用程式。

---

## 1. 核心設計架構

CPPUI M6 引入了三大宣告式開發支柱：

```
┌────────────────────────────────────────────────────────┐
│  Fluent RAD DSL (VStack, HStack, Grid, Border, Button) │
└───────────────────────────┬────────────────────────────┘
                            │ 鏈式屬性與階層建構
┌───────────────────────────▼────────────────────────────┐
│      UIBuilder<T> (零負擔樣板包裝，隱式轉型 shared_ptr)       │
└─────────────┬────────────────────────────┬─────────────┘
              │ 綁定數據流                  │ 掛載路由事件
┌─────────────▼─────────────┐ ┌────────────▼─────────────┐
│ Signal<T> 響應式狀態中樞     │ │  RoutedEvent 冒泡/隧道管線  │
└───────────────────────────┘ └──────────────────────────┘
```

1. **`UIBuilder<T>`**:
   零額外記憶體負擔的樣板包裝器，透過流暢鏈式語法（`.Width()`, `.Padding()`, `.Background()`）配置屬性，並可隱式無縫轉換為 `std::shared_ptr<core::UIElement>`。
2. **語意化工廠函式**:
   `VStack`, `HStack`, `Grid`, `Border`, `Label`, `Button`, `CheckBox`, `TextBox`，支援直覺的 C++ 初始化串列語法 `{ ... }`。
3. **`Signal<T>` 響應式資料流**:
   型別安全且自動進行值比對，透過 `.BindText()` 搭配弱引用保護（`std::weak_ptr`）實現資料到畫面的即時零洩漏更新。

---

## 2. 快速範例：響應式點擊計數器與表單

以下範例示範如何在極簡程式碼內組合容器、控制項與響應式資料：

```cpp
#include "ui/dsl/Fluent.hpp"

using namespace cppui;
using namespace cppui::core;
using namespace cppui::reactive;
using namespace cppui::dsl;

// 1. 定義響應式資料狀態
Signal<int> clickCount(0);
Signal<std::string> status("準備就緒");

// 2. 宣告式建構 UI 階層樹
auto myUi = Border(
    VStack({
        Label("CPPUI 現代宣告式介面")
            .FontSize(18.0f)
            .FontWeight(text::FontWeight::Bold)
            .Foreground(Color(255, 255, 255, 255)),

        // 自動綁定 Signal 的標籤
        Label("點擊次數: 0")
            .FontSize(16.0f)
            .Foreground(Color(80, 200, 120, 255))
            .BindText(clickCount, [](int count) {
                return "點擊次數: " + std::to_string(count);
            }),

        HStack({
            Button("+1 增加")
                .Width(80.0f)
                .Height(32.0f)
                .Background(Color(40, 110, 200, 255))
                .CornerRadius(4.0f)
                .OnClick([&]() {
                    clickCount = clickCount.Get() + 1;
                }),

            Button("重設")
                .Width(80.0f)
                .Height(32.0f)
                .Background(Color(180, 50, 50, 255))
                .CornerRadius(4.0f)
                .OnClick([&]() {
                    clickCount = 0;
                })
        }).Spacing(8.0f),

        TextBox("請輸入您的姓名...")
            .Height(36.0f)
            .OnTextChanged([&](compat::string_view txt) {
                status = "當前使用者: " + std::string(txt.data(), txt.size());
            }),

        CheckBox("記住我的偏好設定")
            .OnCheckedChanged([&](bool checked) {
                // 處理選項變更
            })
    }).Spacing(12.0f)
)
.Background(Color(24, 28, 36, 255))
.Padding(20.0f)
.CornerRadius(8.0f)
.Get();
```

---

## 3. Grid 彈性網格排版宣告

在 DSL 中使用 `Grid` 搭配附加屬性助手：

```cpp
auto myGrid = Grid({
    // Row 0, Col 0..1 (跨 2 欄的標題列)
    Label("應用程式標題").Row(0).Column(0).ColumnSpan(2),

    // Row 1, Col 0 (左側工具箱)
    VStack({
        Button("工具 1"),
        Button("工具 2")
    }).Row(1).Column(0),

    // Row 1, Col 1 (主要工作區)
    Border(Label("主要內容繪製區")).Row(1).Column(1)
}).Get();

// 設定行列度量規則 (Pixel, Star, Auto)
myGrid->AddColumn(layout::GridLength::Pixel(200.0f)); // 固定 200px 側邊欄
myGrid->AddColumn(layout::GridLength::Star(1.0f));    // 剩餘彈性寬度
myGrid->AddRow(layout::GridLength::Pixel(50.0f));      // 固定 50px 標題列
myGrid->AddRow(layout::GridLength::Star(1.0f));       // 剩餘彈性高度
```

---

## 4. 最佳實踐與編碼契約

1. **命名空間隔離原則**：
   在撰寫 Fluent DSL 程式碼時，建議 `using namespace cppui::dsl;`，若需要控制項與排版型別，使用 `controls::Button` 或 `layout::Grid` 前綴，避免 class 名稱與 DSL factory 函式同名衝突。
2. **生命週期保護**：
   `Signal::Bind()` 內部會將傳入的 `std::shared_ptr<ControlType>` 轉換為 `std::weak_ptr`，當 UI 控制項被銷毀時，監聽回呼將自動安全略過，防止懸空指標與循環引用記憶體洩漏。
3. **零拷貝字串視圖**：
   文字事件回呼（如 `OnTextChanged`）傳遞 `compat::string_view`，僅在需要長期持有時才進行 `std::string` 分配。
