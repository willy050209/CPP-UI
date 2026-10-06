# CPPUI 演進目標地圖 (Roadmap)

本專案的核心目標為打造結合 **Skia 渲染底層**、**HarfBuzz/ICU 文字排版**、**Avalonia 視覺樹與雙階段佈局**，並具備 **Windows Forms 極速直覺開發體驗** 的現代 C++ 跨平台 UI 框架。

詳細目標地圖與六大核心階段驗收標準，請參見：
👉 [docs/roadmap/target_map.md](docs/roadmap/target_map.md)

---

## 核心六大階段概覽

1. **[M1] 平台視窗抽象與 Skia 渲染環境搭建** (`include/ui/pal/`, `src/pal/`, `src/rendering/backend/`)
   - 封裝 SDL3 / Win32 原生事件迴圈與視窗控制代碼。
   - 綁定 Skia `GrDirectContext`，初始化 GPU Framebuffer 與低延遲 RunLoop。

2. **[M2] 自研 DisplayList 與 Canvas 命令錄製系統** (`include/ui/rendering/`, `src/rendering/`)
   - 採用連續平坦記憶體池 `LinearArena` 錄製繪圖命令流。
   - 提供抽象 `DrawingContext`，並由 `DisplayListRenderer` 重播至 SkCanvas。

3. **[M3] ICU + HarfBuzz 文字整形與排版管線** (`include/ui/text/`, `src/text/`)
   - 支援字型匹配快取、Unicode BiDi 雙向文字分析、UBreakIterator 斷行。
   - HarfBuzz 複合字形塑形與多行折行測量 `TextLayoutEngine`。

4. **[M4] Visual Tree、Dirty Tracking 與雙階段佈局系統** (`include/ui/core/`, `include/ui/layout/`)
   - 實現 `Visual` 幾何節點與 `UIElement` 佈局容器。
   - Avalonia 風格雙階段 `Measure` 與 `Arrange` 管線。
   - 分離 `InvalidateMeasure`、`InvalidateArrange`、`InvalidateVisual` 三重標記。

5. **[M5] 輸入分發、焦點管理與路由事件** (`include/ui/input/`, `src/input/`)
   - 反向 Z-Order 幾何命中測試（Hit Testing）。
   - 雙向路由事件：Tunneling（預檢）與 Bubbling（冒泡），支援 `e.Handled` 阻斷。
   - 全域焦點管理器與 Tab 鍵導向鏈。

6. **[M6] 控制項庫與 Fluent RAD API 封裝** (`include/ui/controls/`, `src/controls/`)
   - 宣告式 Fluent Builder 現代 C++ DSL，支援 Lambda 極速事件掛載。
   - 融合 Avalonia 保留模式架構與 Windows Forms 閃電生產力。
