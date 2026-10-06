# Visual 類別

- **命名空間**: `cppui::core`
- **標頭檔**: `include/ui/core/Visual.hpp`
- **繼承關係**: 基底視覺類別（Visual Tree 核心抽象）

---

## 概述

`Visual` 是 CPPUI 視覺樹（Visual Tree）的抽象基底類別，負責建立階層親緣關係、局部位移與邊界配置、反向 Z-Order 命中測試（Hit-Testing）、座標轉換以及階層繪製指令流發派（`Render` / `OnRender`）。

---

## 核心成員方法

### 親緣管理
- `void AddVisualChild(std::shared_ptr<Visual> child)`: 將子視覺節點加入集合並設定其父指標。
- `void RemoveVisualChild(const std::shared_ptr<Visual>& child)`: 移除子節點並清除其父指標。
- `void ClearVisualChildren()`: 清空所有子節點。
- `Visual* GetVisualParent() const noexcept`: 取得父節點指標。
- `size_t GetVisualChildrenCount() const noexcept`: 取得子節點總數。
- `Visual* GetVisualChild(size_t index) const`: 依索引存取子節點。

### 坐標轉換與命中測試
- `Point TransformToAncestor(Point p, const Visual* ancestor) const`: 將本節點局部座標轉換為指定祖先節點的座標系。
- `Point TransformFromAncestor(Point p, const Visual* ancestor) const`: 將指定祖先節點的座標系轉換為本節點局部座標。
- `Point PointToScreen(Point p) const`: 將本節點局部座標轉換為根視窗螢幕座標。
- `Visual* HitTest(Point localPoint)`: 執行反向 Z-Order 深度優先命中測試，返回被點擊之最頂層可見視覺節點。

### 繪製管線
- `void Render(rendering::DrawingContext& dc)`: 遞迴渲染本節點及其子節點，處理局部透明度與剪裁壓棧。
- `virtual void OnRender(rendering::DrawingContext& dc)`: 子類別自訂繪製掛鉤點。
