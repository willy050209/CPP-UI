# Control 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/Control.hpp`](../../../include/ui/controls/Control.hpp)
- **繼承關係**: 繼承自 [`core::UIElement`](core_uielement.md)

---

## 概述

`Control` 是所有具備樣式、外觀邊界與互動狀態之 UI 控制項的基礎抽象類別。提供標準的內部邊距（`Padding`）、背景填滿色彩（`Background`）、前景繪製色彩（`Foreground`）、邊框畫筆（`BorderBrush`）、邊框粗細（`BorderThickness`）與圓角曲率（`CornerRadius`），並實作 `DeflateSize` 與 `InflateSize` 邊界度量運算。

---

## 核心成員方法

### 邊界與幾何樣式
- `COMPAT_NODISCARD core::Point GetLocation() const noexcept / void SetLocation(core::Point location) / void SetLocation(float x, float y)`:
  取得或設定控制器左上角相對於容器左上角的座標（DIP 像素）。設定座標時會自動同步更新渲染邊界並標記 `InvalidateArrange()` 重新排版。
- `COMPAT_NODISCARD const core::Thickness& GetPadding() const noexcept / void SetPadding(const core::Thickness& padding)`:
  取得或設定控制項內部內縮邊距。當邊距改變時觸發 `InvalidateMeasure()`。
- `COMPAT_NODISCARD core::Color GetBackground() const noexcept / void SetBackground(core::Color bg)`:
  取得或設定背景色彩。變更時觸發 `InvalidateVisual()`。
- `COMPAT_NODISCARD core::Color GetForeground() const noexcept / void SetForeground(core::Color fg)`:
  取得或設定前景文字與內容繪製色彩。變更時觸發 `InvalidateVisual()`。
- `COMPAT_NODISCARD core::Color GetBorderBrush() const noexcept / void SetBorderBrush(core::Color brush)`:
  取得或設定邊框描邊色彩。變更時觸發 `InvalidateVisual()`。
- `COMPAT_NODISCARD const core::Thickness& GetBorderThickness() const noexcept / void SetBorderThickness(const core::Thickness& thickness)`:
  取得或設定四邊外框粗細。變更時觸發 `InvalidateMeasure()`。
- `COMPAT_NODISCARD float GetCornerRadius() const noexcept / void SetCornerRadius(float radius)`:
  取得或設定圓角半徑（像素 DIP）。變更時觸發 `InvalidateVisual()`。

### 度量輔助運算
- `COMPAT_NODISCARD core::Size DeflateSize(core::Size size) const noexcept`:
  自指定空間中扣除四周 `Padding` 與 `BorderThickness` 後計算內部可用區域尺寸。
- `COMPAT_NODISCARD core::Size InflateSize(core::Size size) const noexcept`:
  將指定內部內容尺寸累加四周 `Padding` 與 `BorderThickness` 膨脹回控制項所需尺寸。

### 繪製輔助
- `void RenderBackgroundAndBorder(rendering::DrawingContext& dc, const core::Rect& bounds)`:
  依據目前背景色、邊框筆刷、邊框粗細與圓角半徑，將背景矩形/圓角矩形與描邊指令輸出至繪圖上下文。
