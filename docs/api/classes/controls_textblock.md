# TextBlock 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: `include/ui/controls/TextBlock.hpp`
- **繼承關係**: 繼承自 `core::UIElement`

---

## 概述

`TextBlock` 是橋接 M3 文字管線的高效能輕量文字展示控制項。支援多行自動折行（`TextWrapping`）、文字對齊（`TextAlignment`）、字型描述（`FontDescriptor`）以及精確文字邊界度量。

---

## 核心成員方法

- `compat::string_view GetText() const noexcept / void SetText(compat::string_view text)`: 取得或設定文字內容（變更時觸發 `InvalidateMeasure()`）。
- `void SetFont(const text::FontDescriptor& font)`: 設定字型規格。
- `void SetFontSize(float size)`: 設定字體尺寸。
- `void SetFontFamily(compat::string_view family)`: 設定字體系列名稱。
- `void SetForeground(core::Color foreground)`: 設定前景文字色彩（**僅**觸發 `InvalidateVisual()`，不觸發排版計算）。
- `void SetTextWrapping(text::TextWrapping wrapping)`: 設定自動折行規則。
- `void SetTextAlignment(text::TextAlignment alignment)`: 設定段落對齊方式。
- `Size MeasureOverride(Size availableSize) override`: 透過 `TextLayoutEngine::MeasureText` 計算所需尺寸。
- `void OnRender(rendering::DrawingContext& dc) override`: 產生文字繪製指令。
