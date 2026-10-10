# Slider 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/Slider.hpp`](../../../include/ui/controls/Slider.hpp)
- **繼承關係**: 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`Slider` 是數值範圍拖曳滑桿控制項（對標 WPF Slider 與 WinForms TrackBar）。
內部包含軌道槽（Track）與圓形滑塊（Thumb），支援滑鼠點擊軌道跳躍定位、滑鼠拖曳滑塊即時調節、鍵盤方向鍵微調（`SmallChange`）與 PageUp / PageDown 粗調（`LargeChange`）。

---

## 靜態路由事件

- `static const input::RoutedEvent Slider::ValueChangedEvent`: 當滑桿數值變更時引發（`RoutingStrategy::Bubble`）。

---

## 建構子

- `Slider()`: 建立預設滑桿（範圍 0.0 ~ 100.0，初始值 0.0，水平方向）。
- `Slider(float min, float max, float value)`: 建立具備指定數值上下限與初始值的滑桿。

---

## 核心成員方法

### 數值與範圍控制
- `COMPAT_NODISCARD float GetMinimum() const noexcept / void SetMinimum(float min)`: 取得或設定數值下限。
- `COMPAT_NODISCARD float GetMaximum() const noexcept / void SetMaximum(float max)`: 取得或設定數值上限。
- `COMPAT_NODISCARD float GetValue() const noexcept / void SetValue(float val)`: 取得或設定目前數值。數值自動夾取於 `[Minimum, Maximum]` 範圍，變更時觸發 `ValueChangedEvent`。

### 步進量配置
- `COMPAT_NODISCARD float GetSmallChange() const noexcept / void SetSmallChange(float change) noexcept`: 取得或設定方向鍵微調步進量（預設 1.0）。
- `COMPAT_NODISCARD float GetLargeChange() const noexcept / void SetLargeChange(float change) noexcept`: 取得或設定翻頁鍵（PageUp / PageDown）或大步進粗調量（預設 10.0）。

### 方向配置
- `COMPAT_NODISCARD layout::Orientation GetOrientation() const noexcept / void SetOrientation(layout::Orientation orientation)`: 取得或設定滑桿排列方向（`Orientation::Horizontal` 或 `Orientation::Vertical`）。

### 事件監聽捷徑
- `void OnValueChanged(std::function<void(float)> handler)`: 註冊數值變更回呼函式，參數為最新數值。

### 排版、互動與繪製管線
- `core::Size MeasureOverride(core::Size availableSize) override`: 測量階段，計算水平預設高度 24px、垂直預設寬度 24px 之期望尺寸。
- `void OnRender(rendering::DrawingContext& dc) override`: 繪製未激活底軌道、已填充啟用區段軌道與圓形滑塊（Thumb）。支援 Hover 與 Dragging 高亮反饋。
- `void ApplyTheme(const theme::Theme& theme) override`: 隨主題更新軌道底色、啟用高亮色與滑塊色彩。

---

## Fluent DSL 支援

可透過 `#include "ui/dsl/Fluent.hpp"` 鏈式建立與綁定：

```cpp
using namespace cppui::dsl;

auto slider = Slider(0.0f, 100.0f, 50.0f)
    .SmallChange(2.0f)
    .LargeChange(10.0f)
    .OnValueChanged([](float val) {
        // 處理數值變更
    });
```
