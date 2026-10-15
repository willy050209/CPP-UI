# TreeView 與 TreeViewItem 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/TreeView.hpp`](../../../include/ui/controls/TreeView.hpp)
- **繼承關係**: `TreeView` 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`TreeView` 是階層式樹狀檢視控制項，用於展示檔案目錄結構、組織圖或巢狀分類清單。支援節點階層縮排計算、折疊/展開幾何箭頭繪製、延遲載入 (Lazy Loading)、滑鼠點擊與鍵盤方向鍵節點巡覽（Left 收合/跳至父節點，Right 展開/跳至子節點，Up/Down 垂直巡覽）。

---

## TreeViewItem 類別

代表樹狀結構中的單一節點：
- `std::shared_ptr<TreeViewItem> AddItem(compat::string_view text)`: 於此節點下新增子項目。
- `void AddItem(std::shared_ptr<TreeViewItem> item)`: 掛載子節點實例。
- `COMPAT_NODISCARD compat::string_view GetText() const noexcept`: 取得節點標題文字。
- `void SetText(compat::string_view text)`: 設定節點標題文字。
- `COMPAT_NODISCARD bool IsExpanded() const noexcept`: 查詢節點是否處於展開狀態。
- `void SetExpanded(bool expanded)`: 設定節點展開或收合狀態。
- `COMPAT_NODISCARD bool HasChildren() const noexcept`: 查詢是否擁有子節點。
- `void SetExpandingHandler(ExpandingHandler handler)`: 設定延遲載入回呼 `void(TreeViewItem&)`，首次展開時動態載入子項目。

---

## TreeView 核心成員方法

### 根節點集合操作
- `std::shared_ptr<TreeViewItem> AddItem(compat::string_view text)`: 新增頂層根項目。
- `void AddItem(std::shared_ptr<TreeViewItem> item)`: 掛載頂層根節點。
- `void ClearItems()`: 清空所有節點。
- `COMPAT_NODISCARD const std::vector<std::shared_ptr<TreeViewItem>>& GetItems() const noexcept`: 取得頂層節點清單。

### 選取狀態管理
- `COMPAT_NODISCARD std::shared_ptr<TreeViewItem> GetSelectedItem() const noexcept`: 取得目前所選取之節點指標。
- `void SetSelectedItem(std::shared_ptr<TreeViewItem> item)`: 設定選取之節點。
- `void OnSelectionChanged(SelectionChangedHandler handler)`: 註冊選取變更回呼函式 `void(std::shared_ptr<TreeViewItem>)`.

### 外觀與間距調整
- `COMPAT_NODISCARD float GetIndent() const noexcept`: 取得每層階層之縮排像素寬度（預設 16.0f）。
- `void SetIndent(float indent) noexcept`: 設定每層階層縮排寬度。
- `COMPAT_NODISCARD float GetItemHeight() const noexcept`: 取得單行節點高度（預設 24.0f）。
- `void SetItemHeight(float height) noexcept`: 設定單行節點高度。

---

## 使用範例

```cpp
#include "ui/controls/TreeView.hpp"

auto tree = std::make_shared<cppui::controls::TreeView>();
tree->SetWidth(280.0f);
tree->SetHeight(400.0f);

auto rootNode = tree->AddItem("專案根目錄");
auto srcNode = rootNode->AddItem("src");
srcNode->AddItem("main.cpp");
srcNode->AddItem("App.cpp");

auto includeNode = rootNode->AddItem("include");
includeNode->AddItem("App.hpp");

rootNode->SetExpanded(true);
srcNode->SetExpanded(true);

tree->OnSelectionChanged([](std::shared_ptr<cppui::controls::TreeViewItem> selected) {
    if (selected) {
        compat::println("已選取節點：{}", selected->GetText());
    }
});
```
