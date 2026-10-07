# cppui::dsl 命名空間

- **標頭檔**: [`include/ui/dsl/Fluent.hpp`](file:///D:/P/CPP/CPPUI/include/ui/dsl/Fluent.hpp)

---

## 概述

`cppui::dsl` 命名空間提供宣告式 Fluent RAD（Rapid Application Development）語法糖。透過 `UIBuilder<T>` 包裝樣板與現代 C++ 函式鏈式調用（Chaining），讓開發者能以極簡且直觀的階層樹語法快速構建完整的桌面使用者介面。

---

## 核心樣板

### `UIBuilder<T>`
包裝 `std::shared_ptr<T>` 的 RAII 流暢型別建構器。提供：
- 隱式型別轉換為 `std::shared_ptr<UIElement>`，支援直接嵌套於容器的 `{ ... }` 初始化串列中。
- 幾何與對齊鏈式配置：`.Width()`, `.Height()`, `.Margin()`, `.Padding()`, `.HorizontalAlignment()`, `.VerticalAlignment()`。
- 外觀樣式鏈式配置：`.Background()`, `.Foreground()`, `.BorderBrush()`, `.BorderThickness()`, `.CornerRadius()`。
- 附加排版屬性快捷：`.Row()`, `.Column()`, `.RowSpan()`, `.ColumnSpan()`。
- 事件鏈式掛載：`.OnClick()`, `.OnCheckedChanged()`, `.OnTextChanged()`。
- 響應式自動綁定：`.BindText(signal, formatter)`。

---

## 工廠函式清單

| 函式名稱 | 回傳型別 | 說明 |
| :--- | :--- | :--- |
| `VStack({ ... })` | `UIBuilder<layout::StackPanel>` | 建立垂直排列流式排版容器。 |
| `HStack({ ... })` | `UIBuilder<layout::StackPanel>` | 建立水平排列流式排版容器。 |
| `Border(child)` | `UIBuilder<layout::Border>` | 建立帶邊框/背景之裝飾容器。 |
| `Grid({ ... })` | `UIBuilder<layout::Grid>` | 建立多維行列彈性網格排版容器。 |
| `Label(text)` | `UIBuilder<controls::TextBlock>` | 建立文字標籤元件。 |
| `Button(text)` | `UIBuilder<controls::Button>` | 建立互動按鈕元件。 |
| `CheckBox(label)` | `UIBuilder<controls::CheckBox>` | 建立核取方塊元件。 |
| `TextBox(placeholder)` | `UIBuilder<controls::TextBox>` | 建立文字編輯輸入框元件。 |
