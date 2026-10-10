# Separator 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/Separator.hpp`](../../../include/ui/controls/Separator.hpp)
- **繼承關係**: 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`Separator` 是分隔線控制項，用於在選單、工具列、清單或面版中劃分不同群組的控制項。支援水平與垂直兩種方向，並可自訂線條厚度與繪製色彩。

---

## 建構子

- `Separator()`: 建立預設水平分隔線（厚度 1.0f）。
- `explicit Separator(layout::Orientation orientation)`: 建立指定方向之分隔線控制項。

---

## 核心成員方法

### 方向與厚度控制
- `COMPAT_NODISCARD layout::Orientation GetOrientation() const noexcept / void SetOrientation(layout::Orientation orientation)`: 取得或設定分隔線方向（`Orientation::Horizontal` 或 `Orientation::Vertical`）。
- `COMPAT_NODISCARD float GetThickness() const noexcept / void SetThickness(float thickness)`: 取得或設定線條厚度（像素）。若傳入之 `thickness < 0.0f` 則擲出 `std::invalid_argument`。

### 色彩自訂
- `COMPAT_NODISCARD core::Color GetLineColor() const noexcept / void SetLineColor(core::Color color)`: 取得或自訂分隔線繪製色彩。若未特別設定，預設將採用主題外框或分隔線色彩代幣。

### 排版與繪製管線
- `core::Size MeasureOverride(core::Size availableSize) override`: 測量階段。若為水平方向，回傳期望高度為 `thickness`；若為垂直方向，回傳期望寬度為 `thickness`。
- `void OnRender(rendering::DrawingContext& dc) override`: 繪製水平或垂直純量線條。

---

## Fluent DSL 支援

可透過 `#include "ui/dsl/Fluent.hpp"` 鏈式建立與綁定：

```cpp
using namespace cppui::dsl;

auto sep = Separator(layout::Orientation::Horizontal)
    .Thickness(1.0f);
```
