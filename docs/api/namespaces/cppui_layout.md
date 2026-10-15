# cppui::layout 命名空間

`cppui::layout` 提供現代化宣告式 UI 所需之核心版面容器、幾何約束列舉與頂層排版調度管理器。

---

## 核心類型

### 類別 (Classes)
- [`Panel`](../classes/layout_panel.md): 多子元件版面容器抽象基底。
- [`StackPanel`](../classes/layout_stackpanel.md): 單一路徑方向線性流式版面容器。
- [`Border`](../classes/layout_border.md): 裝飾性邊框、圓角、背景與 Padding 容器。
- [`Canvas`](../classes/layout_canvas.md): 絕對座標定位版面容器。
- [`Grid`](../classes/layout_grid.md): 強大靈活的二維網格佈局容器 (RowDefinition, ColumnDefinition)。
- [`VirtualizingStackPanel`](../classes/layout_virtualizingstackpanel.md): 高效能視區虛擬化與容器回收重用線性排版容器。
- [`LayoutManager`](../classes/layout_layoutmanager.md): 頂層視窗排版調度器，具備收斂迭代上限與防震盪保護。

### 列舉 (Enums)
- `HorizontalAlignment`: `Left`, `Center`, `Right`, `Stretch`。
- `VerticalAlignment`: `Top`, `Center`, `Bottom`, `Stretch`。
- `Visibility`: `Visible`, `Hidden`, `Collapsed`。
- `Orientation`: `Vertical`, `Horizontal`。
- `GridUnitType`: `Auto`, `Pixel`, `Star`。
