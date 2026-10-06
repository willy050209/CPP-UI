# UIElement 類別

- **命名空間**: `cppui::core`
- **標頭檔**: `include/ui/core/UIElement.hpp`
- **繼承關係**: 繼承自 `Visual`

---

## 概述

`UIElement` 是具備雙階段排版管線（`Measure` -> `Arrange`）、約束幾何運算（Margin, Alignment, Min/Max Constraint）與精確髒標記追蹤（Dirty Tracking）的核心 UI 抽象基底類別。

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
