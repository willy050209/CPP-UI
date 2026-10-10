# ProgressBar 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/ProgressBar.hpp`](../../../include/ui/controls/ProgressBar.hpp)
- **繼承關係**: 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`ProgressBar` 是數值進度指示條控制項。支援水平與垂直兩種方向配置、數值範圍夾取（Minimum / Maximum / Value）、步進量推進（`PerformStep` / `Increment`，對標 WinForms）、平滑數值填色，以及未定態跑馬燈模式（Indeterminate）。

---

## 靜態路由事件

- `static const input::RoutedEvent ProgressBar::ValueChangedEvent`: 當進度數值發生變更時引發（`RoutingStrategy::Bubble`）。

---

## 建構子

- `ProgressBar()`: 建立預設進度條控制項（範圍 0.0 ~ 100.0，初始值 0.0，水平方向）。

---

## 核心成員方法

### 數值與範圍控制
- `COMPAT_NODISCARD float GetMinimum() const noexcept / void SetMinimum(float min)`: 取得或設定進度下限數值（預設 0.0）。若當前數值小於新最小值，將自動夾取調整。
- `COMPAT_NODISCARD float GetMaximum() const noexcept / void SetMaximum(float max)`: 取得或設定進度上限數值（預設 100.0）。若當前數值大於新最大值，將自動夾取調整。
- `COMPAT_NODISCARD float GetValue() const noexcept / void SetValue(float val)`: 取得或設定目前進度數值。數值自動被限制於 `[Minimum, Maximum]` 範圍內，若數值改變則觸發 `ValueChangedEvent` 與重繪。

### 步進操作 (WinForms 相容)
- `COMPAT_NODISCARD float GetStep() const noexcept / void SetStep(float step) noexcept`: 取得或設定單次步進量數值（預設 10.0）。
- `void PerformStep()`: 依目前設定之 `Step` 步進量增加進度數值。
- `void Increment(float val)`: 增加指定數值之進度增量。

### 方向與未定態
- `COMPAT_NODISCARD layout::Orientation GetOrientation() const noexcept / void SetOrientation(layout::Orientation orientation)`: 取得或設定進度條排列方向（`Orientation::Horizontal` 或 `Orientation::Vertical`）。
- `COMPAT_NODISCARD bool IsIndeterminate() const noexcept / void SetIndeterminate(bool indeterminate)`: 取得或設定是否啟用未定跑馬燈狀態（Indeterminate）。

### 事件監聽捷徑
- `void OnValueChanged(std::function<void(float)> handler)`: 註冊進度數值變更回呼函式，參數為最新的進度浮點數值。

### 排版與繪製管線
- `core::Size MeasureOverride(core::Size availableSize) override`: 測量階段，水平方向預設高度 16px、垂直方向預設寬度 16px。
- `void OnRender(rendering::DrawingContext& dc) override`: 繪製外圍軌道背景槽與當前進度比例的實心填充進度條。
- `void ApplyTheme(const theme::Theme& theme) override`: 根據主題動態更新軌道色彩與進度高亮主色彩。

---

## Fluent DSL 支援

可透過 `#include "ui/dsl/Fluent.hpp"` 鏈式建立與綁定：

```cpp
using namespace cppui::dsl;

auto progress = ProgressBar()
    .Minimum(0.0f)
    .Maximum(100.0f)
    .Value(45.0f)
    .Orientation(layout::Orientation::Horizontal)
    .Width(200.0f);
```
