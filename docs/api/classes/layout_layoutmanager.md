# LayoutManager 類別

- **命名空間**: `cppui::layout`
- **標頭檔**: `include/ui/layout/LayoutManager.hpp`

---

## 概述

`LayoutManager` 是頂層視窗排版調度器，負責協調根視覺樹的兩階段排版管線（`Measure` 與 `Arrange`），並維護即時排版統計度量數據（`totalMeasures`, `totalArranges`）。

---

## 核心成員方法

- `static LayoutManager& Get()`: 取得全域單例物件。
- `void UpdateLayout(core::UIElement& root, core::Size rootSize)`: 當根節點存在髒標記或視窗尺寸變更時觸發排版。
- `LayoutStats GetStats() const noexcept`: 取得排版調度統計數據。
- `void ResetStats() noexcept`: 重設統計計數器。
