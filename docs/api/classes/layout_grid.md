# Grid 類別

- **命名空間**: `cppui::layout`
- **標頭檔**: [`include/ui/layout/Grid.hpp`](../../../include/ui/layout/Grid.hpp)
- **繼承關係**: 繼承自 [`layout::Panel`](layout_panel.md)

---

## 概述

`Grid` 是多維行列彈性網格排版容器。支援類似 WPF / Avalonia 的多儲存格排版機制，定義像素固定（`Pixel`）、自適應（`Auto`）與比例權重（`Star (*)`）的行列規則，並透過附加屬性（Attached Properties）為子元件設定跨列跨欄（`RowSpan`, `ColumnSpan`）。

---

## 相關輔助結構

### GridLength
- `GridUnitType::Pixel`: 絕對 DIP 像素長度（如 `GridLength::Pixel(100.0f)`）。
- `GridUnitType::Auto`: 依據該欄列所有子元件的最大內容自適應度量（`GridLength::Auto()`）。
- `GridUnitType::Star`: 按權重比率分配剩餘空間（如 `GridLength::Star(1.0f)`, `GridLength::Star(2.0f)`）。

### RowDefinition & ColumnDefinition
- `RowDefinition(GridLength height, float minH = 0.0f, float maxH = 1e6f)`: 定義橫向列的高規格與約束。
- `ColumnDefinition(GridLength width, float minW = 0.0f, float maxW = 1e6f)`: 定義直向欄的寬規格與約束。

---

## 核心成員方法

### 欄列定義註冊
- `void AddRow(RowDefinition row)` / `void AddRow(GridLength height)`: 加入一列定義。
- `void AddRowDefinition(RowDefinition row)`: WPF 相容別名方法。
- `void AddColumn(ColumnDefinition col)` / `void AddColumn(GridLength width)`: 加入一欄定義。
- `void AddColumnDefinition(ColumnDefinition col)`: WPF 相容別名方法。
- `const std::vector<RowDefinition>& GetRowDefinitions() const noexcept`: 取得所有列定義。
- `const std::vector<ColumnDefinition>& GetColumnDefinitions() const noexcept`: 取得所有欄定義。

### 附加屬性 (Attached Properties)
- `static void SetRow(core::UIElement& element, size_t row) / static size_t GetRow(const core::UIElement& element)`:
  設定或取得子元件所屬之網格列索引（預設為 0）。
- `static void SetColumn(core::UIElement& element, size_t col) / static size_t GetColumn(const core::UIElement& element)`:
  設定或取得子元件所屬之網格欄索引（預設為 0）。
- `static void SetRowSpan(core::UIElement& element, size_t span) / static size_t GetRowSpan(const core::UIElement& element)`:
  設定或取得子元件橫跨之列數（預設為 1）。
- `static void SetColumnSpan(core::UIElement& element, size_t span) / static size_t GetColumnSpan(const core::UIElement& element)`:
  設定或取得子元件橫跨之欄數（預設為 1）。

### 排版度量與排列
- `core::Size MeasureOverride(core::Size availableSize) override`: 執行多階段求解（第一階段評估 Pixel 與 Auto 儲存格，第二階段依據比例權重求解 Star 儲存格）。
- `void ArrangeOverride(core::Size finalSize) override`: 將所有子元件依其配置之 `(Row, Column, RowSpan, ColumnSpan)` 正確置放於計算所得之矩形邊界。
