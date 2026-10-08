# Button 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/Button.hpp`](../../../include/ui/controls/Button.hpp)
- **繼承關係**: 繼承自 [`controls::ContentControl`](controls_contentcontrol.md)

---

## 概述

`Button` 是現代化互動按鈕控制項。內部封裝完整的互動狀態機，支援滑鼠懸停（Hover）、下壓（Pressed）、停用（Disabled）、獲焦霓虹外框光暈（Focus Ring）以及鍵盤空白鍵（`Space` 0x20）與 Enter 鍵（0x0D）觸發，並發布標準 `Click` 冒泡路由事件。

---

## 靜態路由事件

- `static const input::RoutedEvent Button::ClickEvent`: 標識按鈕點擊完成的冒泡路由事件（`RoutingStrategy::Bubble`）。

---

## 核心成員方法

### 狀態查詢與啟用控制
- `COMPAT_NODISCARD bool IsPressed() const noexcept`: 查詢目前是否處於按下滑鼠左鍵狀態。
- `COMPAT_NODISCARD bool IsHovered() const noexcept`: 查詢滑鼠游標是否懸停於按鈕上方。
- `COMPAT_NODISCARD bool IsEnabled() const noexcept`: 查詢按鈕是否處於可用啟用狀態。
- `void SetEnabled(bool enabled)`: 啟用或停用按鈕互動能力。若停用則無法被獲焦且不響應輸入。

### 色彩與樣式定制
- `void SetHoverBackground(core::Color color)`: 設定游標懸停時的背景色彩。
- `void SetPressedBackground(core::Color color)`: 設定按下滑鼠時的背景色彩。
- `void SetFocusBorderBrush(core::Color color)`: 設定獲取鍵盤焦點時的外圍光暈邊框色彩。

### 事件監聽捷徑
- `void OnClick(ClickEventHandler handler)`: 註冊帶按鈕引用的點擊回呼常式 `void(Button&)`.
- `void OnClick(std::function<void()> handler)`: 註冊無參數的簡潔點擊回呼常式。

### 繪製與狀態呈現
- `void OnRender(rendering::DrawingContext& dc) override`: 依據 Hover、Pressed、Disabled 及 IsFocused 動態選擇筆刷，並輸出圓角幾何背景、外框與焦點光圈。
