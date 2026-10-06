# Border 類別

- **命名空間**: `cppui::layout`
- **標頭檔**: `include/ui/layout/Border.hpp`
- **繼承關係**: 繼承自 `core::UIElement`

---

## 概述

`Border` 是一個裝飾性單一子元件容器，提供邊框線條（`BorderBrush`, `BorderThickness`）、背景填滿（`Background`）、圓角弧度（`CornerRadius`）以及內部留白（`Padding`）。

---

## 核心成員方法

- `SetChild(std::shared_ptr<UIElement> child)`: 設定內容子元件。
- `SetBackground(Color color)`: 設定背景填滿色彩。
- `SetBorderBrush(Color brush)`: 設定邊框線條色彩。
- `SetBorderThickness(const Thickness& thickness)`: 設定四周邊框寬度。
- `SetPadding(const Thickness& padding)`: 設定內容區域內縮邊界。
- `SetCornerRadius(float cornerRadius)`: 設定四角圓角半徑。
- `void OnRender(rendering::DrawingContext& dc) override`: 產生帶有圓角幾何之背景與邊框繪製指令。
