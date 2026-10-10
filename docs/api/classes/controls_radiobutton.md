# RadioButton 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/RadioButton.hpp`](../../../include/ui/controls/RadioButton.hpp)
- **繼承關係**: 繼承自 [`controls::Button`](controls_button.md)

---

## 概述

`RadioButton` 是單選按鈕控制項，繼承自 `Button`。介面上包含一個圓形指示標記與標籤文字（支援純文字或透過 `ContentControl` 承載之視覺子節點）。
當使用者點擊單選按鈕時，其狀態切換為已選取（`Checked`），並自動尋找相同父容器或相同 `GroupName` 下的其他 `RadioButton` 並將其取消選取（互斥排他機制）。

---

## 靜態路由事件

- `static const input::RoutedEvent RadioButton::CheckedEvent`: 當單選按鈕被選取時引發（`RoutingStrategy::Bubble`）。
- `static const input::RoutedEvent RadioButton::UncheckedEvent`: 當單選按鈕被取消選取時引發（`RoutingStrategy::Bubble`）。

---

## 建構子

- `RadioButton()`: 建立空白單選按鈕。
- `explicit RadioButton(compat::string_view label)`: 建立具有指定標籤文字的單選按鈕。

---

## 核心成員方法

### 選取狀態管理
- `COMPAT_NODISCARD bool IsChecked() const noexcept`: 查詢目前是否處於選取狀態。
- `void SetChecked(bool checked)`: 設定單選按鈕之選取狀態。當設為 `true` 時，自動搜尋同群組或同容器內之其他單選按鈕將其取消選取，並觸發 `CheckedEvent` 路由事件與 `OnCheckedChanged` 回呼。

### 群組管理 (GroupName)
- `COMPAT_NODISCARD compat::string_view GetGroupName() const noexcept`: 取得目前設定之群組名稱視圖。
- `void SetGroupName(compat::string_view name)`: 設定所屬群組名稱。當為非空字串時，互斥排他範圍擴展至同一視覺樹根節點下所有相同 `GroupName` 的單選按鈕；若為空字串，則預設以同一父容器為排他範圍。

### 事件監聽捷徑
- `void OnCheckedChanged(std::function<void(bool)> handler)`: 註冊選取狀態變更回呼函式，參數為最新的選取布林值。

### 排版與渲染管線
- `core::Size MeasureOverride(core::Size availableSize) override`: 測量階段，計算包含圓圈指示標記、間距與標籤文字所需期望尺寸。
- `void ArrangeOverride(core::Size finalSize) override`: 排列階段，配置圓圈與標籤文字相對位置。
- `void OnRender(rendering::DrawingContext& dc) override`: 繪製外環圓圈、選取時的內部實心圓核心以及標籤文字。
- `void ApplyTheme(const theme::Theme& theme) override`: 依據主題色彩代幣動態切換圓圈外框與選取填充色。

---

## Fluent DSL 支援

可透過 `#include "ui/dsl/Fluent.hpp"` 鏈式建立與綁定：

```cpp
using namespace cppui::dsl;

auto rb1 = RadioButton("選項 A")
    .GroupName("Options")
    .Checked(true)
    .OnCheckedChanged([](bool isChecked) {
        // 處理選取變更
    });

auto rb2 = RadioButton("選項 B")
    .GroupName("Options");
```
