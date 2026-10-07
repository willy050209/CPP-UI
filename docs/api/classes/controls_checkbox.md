# CheckBox 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/CheckBox.hpp`](file:///D:/P/CPP/CPPUI/include/ui/controls/CheckBox.hpp)
- **繼承關係**: 繼承自 [`controls::Button`](file:///D:/P/CPP/CPPUI/docs/api/classes/controls_button.md)

---

## 概述

`CheckBox` 是具備雙態/核取標記切換的核取方塊控制項。包含可點擊切換的向量向量方框、向量打勾線條（Checkmark）、自訂標籤文字/內容子元件，並派發 `Checked` 與 `Unchecked` 路由事件。

---

## 靜態路由事件

- `static const input::RoutedEvent CheckBox::CheckedEvent`: 當核取方塊轉為勾選狀態時觸發（`RoutingStrategy::Bubble`）。
- `static const input::RoutedEvent CheckBox::UncheckedEvent`: 當核取方塊轉為未勾選狀態時觸發（`RoutingStrategy::Bubble`）。

---

## 核心成員方法

### 狀態管理
- `CheckBox()`: 預設建構函式。
- `explicit CheckBox(compat::string_view label)`: 帶標籤文字建構函式。
- `COMPAT_NODISCARD bool IsChecked() const noexcept`: 查詢目前是否處於勾選狀態。
- `void SetChecked(bool checked)`: 設定勾選狀態，若值變更則觸發對應路由事件與畫面重繪。
- `void Toggle()`: 反轉目前的勾選狀態。

### 事件監聽捷徑
- `void OnCheckedChanged(std::function<void(bool)> handler)`: 註冊狀態變更回呼函式，當勾選或取消勾選時傳入當前布林值。

### 排版與繪製
- `core::Size MeasureOverride(core::Size availableSize) override`: 測量包含 18px 方框、8px 間距與標籤子元件之整體尺寸。
- `void ArrangeOverride(core::Size finalSize) override`: 將方框置於左側，標籤子元件水平置於右側並垂直置中排列。
- `void OnRender(rendering::DrawingContext& dc) override`: 繪製未選中/選中方框外觀、焦點光暈，以及在勾選時繪製抗鋸齒向量對稱勾號。
