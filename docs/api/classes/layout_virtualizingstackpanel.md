# VirtualizingStackPanel 類別

- **命名空間**: `cppui::layout`
- **標頭檔**: [`include/ui/layout/VirtualizingStackPanel.hpp`](../../../include/ui/layout/VirtualizingStackPanel.hpp)
- **繼承關係**: 繼承自 [`layout::Panel`](layout_panel.md)

---

## 概述

`VirtualizingStackPanel` 是高效能視區虛擬化線性堆疊排版容器。針對十萬至數百萬筆極大規模清單，它僅針對目前視區（Viewport）可見範圍及上下緩衝區（Buffer Overscan）建立與排列 UI 容器節點，並透過回收池（Container Recycling Pool）在滾動時就地重用離開視區之節點，將記憶體消耗與 Measure / Arrange 排版開銷降至 $O(1)$ 常數複雜度。

---

## 核心成員方法

### 資料筆數與項目尺寸
- `COMPAT_NODISCARD size_t GetItemCount() const noexcept`: 取得總資料項目數。
- `void SetItemCount(size_t count)`: 設定總資料筆數，自動觸發視區估算。
- `COMPAT_NODISCARD float GetItemExtent() const noexcept`: 取得項目預設跨度像素（垂直時為高度，水平時為寬度）。
- `void SetItemExtent(float extent)`: 設定項目預設跨度。
- `void SetItemHeight(float height)`: 垂直排列時設定項目高度之捷徑方法。

### 容器產生與回收委派
- `void SetContainerGenerator(ContainerGenerator generator)`: 設定產生新容器節點的工廠函式 `std::shared_ptr<core::UIElement>(size_t index)`。
- `void SetContainerRecycler(ContainerRecycler recycler)`: 設定回收重用綁定回呼 `void(size_t index, core::UIElement* container)`，當已存在的容器節點滑入新項目時呼叫以更新資料。

### 視區與滾動聯動
- `void UpdateViewport(float scrollOffset, float viewportSize)`: 由外層 [`ScrollViewer`](../controls_scrollviewer.md) 呼叫以傳遞當前滾動偏移量與視區大小。
- `COMPAT_NODISCARD size_t GetFirstVisibleIndex() const noexcept`: 取得目前視區頂端可見之首個項目索引。
- `COMPAT_NODISCARD size_t GetLastVisibleIndex() const noexcept`: 取得目前視區底部可見之最後項目索引。
- `COMPAT_NODISCARD size_t GetRealizedContainerCount() const noexcept`: 取得目前已實體化且活躍之容器節點總數。

---

## 使用範例

```cpp
#include "ui/layout/VirtualizingStackPanel.hpp"
#include "ui/controls/TextBlock.hpp"

auto panel = std::make_shared<cppui::layout::VirtualizingStackPanel>();
panel->SetItemCount(1000000); // 一百萬筆資料
panel->SetItemHeight(32.0f);

panel->SetContainerGenerator([](size_t index) {
    auto text = std::make_shared<cppui::controls::TextBlock>();
    text->SetText("資料項目 #" + std::to_string(index));
    return text;
});

panel->SetContainerRecycler([](size_t index, cppui::core::UIElement* container) {
    if (auto* text = dynamic_cast<cppui::controls::TextBlock*>(container)) {
        text->SetText("資料項目 #" + std::to_string(index));
    }
});
```
