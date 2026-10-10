# ListBox 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/ListBox.hpp`](../../../include/ui/controls/ListBox.hpp)
- **繼承關係**: 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`ListBox` 是清單方塊控制項，用於展示文字項目集合並提供單選互動。
支援動態增刪項目、索引選取（`SelectedIndex`）與文字選取（`SelectedItem`）、滑鼠點擊項目選取、鍵盤方向鍵（Up / Down）與 Home / End 巡覽導航，並發布 `SelectedIndexChangedEvent` 路由事件。

---

## 靜態路由事件

- `static const input::RoutedEvent ListBox::SelectedIndexChangedEvent`: 當清單選取索引發生變更時引發（`RoutingStrategy::Bubble`）。

---

## 建構子

- `ListBox()`: 建立預設清單方塊控制項（初始為空清單，選取索引為 -1）。

---

## 核心成員方法

### 清單項目管理
- `void AddItem(compat::string_view item)`: 於清單末端加入新文字項目。
- `void InsertItem(size_t index, compat::string_view item)`: 於指定索引插入新項目。若 `index > GetItemCount()` 則擲出 `std::out_of_range`。
- `void RemoveItem(size_t index)`: 移除指定索引位置之項目。若 `index >= GetItemCount()` 則擲出 `std::out_of_range`。若移除之項目影響當前選取，將自動更新或清除選取索引。
- `void ClearItems()`: 清空清單內所有項目，選取索引重置為 -1。
- `COMPAT_NODISCARD size_t GetItemCount() const noexcept`: 取得目前清單項目總數。
- `COMPAT_NODISCARD compat::string_view GetItem(size_t index) const`: 取得指定索引位置之項目文字視圖。若 `index >= GetItemCount()` 則擲出 `std::out_of_range`。

### 選取狀態管理
- `COMPAT_NODISCARD int32_t GetSelectedIndex() const noexcept`: 取得目前選取項目的索引（若無選取則為 -1）。
- `void SetSelectedIndex(int32_t index)`: 設定選取項目索引。合法範圍為 `[-1, GetItemCount() - 1]`，若超出範圍則擲出 `std::out_of_range`。若索引變更則觸發 `SelectedIndexChangedEvent`。
- `COMPAT_NODISCARD compat::string_view GetSelectedItem() const noexcept`: 取得目前選取項目的文字內容視圖（若無選取則回傳空字串）。
- `void SetSelectedItem(compat::string_view item)`: 搜尋並選取第一個相符文字之項目，若未找到則設定選取索引為 -1。

### 外觀與項目配置
- `COMPAT_NODISCARD float GetItemHeight() const noexcept / void SetItemHeight(float height)`: 取得或設定每個項目的行高（預設 24px）。若 `height <= 0.0f` 則擲出 `std::invalid_argument`。

### 事件監聽捷徑
- `void OnSelectedIndexChanged(std::function<void(int32_t)> handler)`: 註冊選取索引變更回呼函式，參數為最新選取的項目索引。

### 排版與繪製管線
- `core::Size MeasureOverride(core::Size availableSize) override`: 測量階段，計算包含所有項目總高度與預設寬度之期望尺寸。
- `void OnRender(rendering::DrawingContext& dc) override`: 繪製清單背景與外框、繪製已選取項目的高亮背景矩形，並依序輸出各項目的文字。
- `void ApplyTheme(const theme::Theme& theme) override`: 依據主題代幣動態切換背景色、外框色、選取背景色與選取文字色。

---

## Fluent DSL 支援

可透過 `#include "ui/dsl/Fluent.hpp"` 鏈式建立與綁定：

```cpp
using namespace cppui::dsl;

auto listBox = ListBox()
    .ItemHeight(28.0f)
    .SelectedIndex(0)
    .OnSelectedIndexChanged([](int32_t index) {
        // 處理項目切換
    });

listBox->AddItem("第一項");
listBox->AddItem("第二項");
listBox->AddItem("第三項");
```
