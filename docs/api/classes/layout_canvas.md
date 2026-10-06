# Canvas 類別

- **命名空間**: `cppui::layout`
- **標頭檔**: `include/ui/layout/Canvas.hpp`
- **繼承關係**: 繼承自 `Panel`

---

## 概述

`Canvas` 是支援絕對坐標定位的版面容器。透過附加屬性靜態方法（`Canvas::SetLeft`, `Canvas::SetTop`）允許子元件自由放置於畫布上的任意像素坐標。

---

## 核心成員方法

- `static void SetLeft(core::UIElement& element, float left)`: 設定元件相對於 Canvas 的 X 坐標。
- `static float GetLeft(const core::UIElement& element)`: 取得元件相對於 Canvas 的 X 坐標。
- `static void SetTop(core::UIElement& element, float top)`: 設定元件相對於 Canvas 的 Y 坐標。
- `static float GetTop(const core::UIElement& element)`: 取得元件相對於 Canvas 的 Y 坐標。
- `Size MeasureOverride(Size availableSize) override`: 以無限寬高測量所有子元件。
- `void ArrangeOverride(Size finalSize) override`: 依據 Left 與 Top 配置子元件目標 Rect。
