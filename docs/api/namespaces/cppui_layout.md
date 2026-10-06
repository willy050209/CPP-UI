# cppui::layout 命名空間

`cppui::layout` 提供現代化聲明式 UI 所需之核心版面容器、幾何約束列舉與頂層排版管理器。

---

## 核心類型

### 類別 (Classes)
- [`Panel`](../classes/layout_panel.md): 多子元件版面容器抽象基底。
- [`StackPanel`](../classes/layout_stackpanel.md): 單一路徑方向線性流式版面容器。
- [`Border`](../classes/layout_border.md): 裝飾性邊框、圓角、背景與 Padding 容器。
- [`Canvas`](../classes/layout_canvas.md): 絕對座標定位版面容器。
- [`LayoutManager`](../classes/layout_layoutmanager.md): 頂層視窗排版調度器。

### 列舉 (Enums)
- `HorizontalAlignment`: `Left`, `Center`, `Right`, `Stretch`。
- `VerticalAlignment`: `Top`, `Center`, `Bottom`, `Stretch`。
- `Visibility`: `Visible`, `Hidden`, `Collapsed`。
- `Orientation`: `Vertical`, `Horizontal`。
