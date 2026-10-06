# Panel 類別

- **命名空間**: `cppui::layout`
- **標頭檔**: `include/ui/layout/Panel.hpp`
- **繼承關係**: 繼承自 `core::UIElement`

---

## 概述

`Panel` 是所有多子元件佈局容器（如 `StackPanel`、`Canvas`）的基底類別。它管理子 `UIElement` 集合，並在增刪子節點時自動維護底層 `Visual` 樹的親緣關係與排版髒標記。

---

## 核心成員方法

- `void AddChild(std::shared_ptr<core::UIElement> child)`: 加入子元件，建立 Visual 親子關聯並觸發 `InvalidateMeasure()`。
- `void RemoveChild(const std::shared_ptr<core::UIElement>& child)`: 移除子元件並觸發 `InvalidateMeasure()`。
- `void ClearChildren()`: 清空所有子元件。
- `size_t GetChildrenCount() const noexcept`: 取得子元件數量。
- `core::UIElement* GetChild(size_t index) const`: 依索引存取子元件。
- `const std::vector<std::shared_ptr<core::UIElement>>& GetChildren() const noexcept`: 取得子元件唯讀集合。
