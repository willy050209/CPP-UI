# ComboBox 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/ComboBox.hpp`](../../../include/ui/controls/ComboBox.hpp)
- **繼承關係**: 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`ComboBox` 是下拉式選單控制項。結合選取文字展示框、右側下拉幾何箭頭按鈕與底層浮層清單 [`Popup`](controls_popup.md)，支援選項動態增刪、滑鼠懸停反白、鍵盤方向鍵巡覽與選取變更回呼通知，同時具備視窗邊界智慧翻轉與點擊外部自動收合 (Light-Dismiss) 能力。

---

## 核心成員方法

### 項目集合操作
- `void AddItem(compat::string_view text)`: 新增純文字選項項目至清單結尾。
- `void InsertItem(size_t index, compat::string_view text)`: 於指定索引處插入文字選項。
- `void RemoveItem(size_t index)`: 移除指定索引處之選項。
- `void ClearItems()`: 清空清單內所有選項項目。
- `COMPAT_NODISCARD size_t GetItemCount() const noexcept`: 取得清單內選項總數。
- `COMPAT_NODISCARD compat::string_view GetItem(size_t index) const`: 依索引取得指定項目的文字。

### 選取狀態管理
- `COMPAT_NODISCARD int32_t GetSelectedIndex() const noexcept`: 取得當前選取的項目索引；若未選取則傳回 `-1`。
- `void SetSelectedIndex(int32_t index)`: 設定當前選取的索引，並同步更新外觀顯示。
- `COMPAT_NODISCARD compat::string_view GetSelectedItem() const`: 取得目前所選取項目的文字視圖。
- `void OnSelectionChanged(SelectionChangedHandler handler)`: 註冊選取變更回呼函式 `void(ComboBox&, int32_t)`。

### 展開與收合控制
- `COMPAT_NODISCARD bool IsDropDownOpen() const noexcept`: 查詢下拉清單目前是否展開。
- `void SetIsDropDownOpen(bool open)`: 程式化控制下拉清單展開或收合。
- `COMPAT_NODISCARD float GetMaxDropDownHeight() const noexcept`: 取得下拉浮層之最大高度上限。
- `void SetMaxDropDownHeight(float height)`: 設定下拉浮層之最大高度上限。

---

## 使用範例

```cpp
#include "ui/controls/ComboBox.hpp"

auto combo = std::make_shared<cppui::controls::ComboBox>();
combo->SetWidth(200.0f);
combo->SetHeight(32.0f);
combo->AddItem("選項一：高畫質 1080p");
combo->AddItem("選項二：超高畫質 4K");
combo->AddItem("選項三：流暢模式 720p");
combo->SetSelectedIndex(0);

combo->OnSelectionChanged([](cppui::controls::ComboBox& sender, int32_t index) {
    compat::println("已選擇項目索引：{}, 文字：{}", index, sender.GetSelectedItem());
});
```
