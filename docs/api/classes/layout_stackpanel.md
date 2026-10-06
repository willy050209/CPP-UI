# StackPanel 類別

- **命名空間**: `cppui::layout`
- **標頭檔**: `include/ui/layout/StackPanel.hpp`
- **繼承關係**: 繼承自 `Panel`

---

## 概述

`StackPanel` 依照單一路徑方向（垂直 `Vertical` 或水平 `Horizontal`）連續排列子元件。支援元素間隔（`Spacing`），並自動跳過可見度為 `Collapsed` 的子節點。

---

## 核心成員方法

- `Orientation GetOrientation() const noexcept / void SetOrientation(Orientation orientation)`: 取得或設定排列走向（預設為 `Vertical`）。
- `float GetSpacing() const noexcept / void SetSpacing(float spacing)`: 取得或設定相鄰子元件之間的像素間距。
- `Size MeasureOverride(Size availableSize) override`: 依方向累加各子節點尺寸並包含 spacing。
- `void ArrangeOverride(Size finalSize) override`: 沿主軸依序排列子元件，交叉軸自動套用對齊或拉伸。
