# VisualTreeInspector 類別

- **命名空間**: `cppui::diagnostics`
- **標頭檔**: [`include/ui/diagnostics/VisualTreeInspector.hpp`](../../../include/ui/diagnostics/VisualTreeInspector.hpp)
- **介面實作**: 實作 [`input::IInputInspectorHook`](../input_inputmanager.md)

---

## 概述

`VisualTreeInspector` 是內建的視覺樹除錯與檢查工具（DevTools）。支援以快捷鍵 F12 即時開關檢查介面、互動式元素吸管拾取模式（Element Picker）、樹狀階層結構視覺化、元件尺寸與邊界（RenderBounds / DesiredSize）屬性即時檢視，並在目標元件上方繪製半透明霓虹藍色除錯框。

---

## 核心成員方法

### 啟用與切換
- `static VisualTreeInspector& Get() noexcept`: 取得診斷器單例實例。
- `COMPAT_NODISCARD bool IsVisible() const noexcept`: 查詢檢查工具目前是否處於開啟顯示狀態。
- `void SetVisible(bool visible)`: 設定檢查工具之顯示或隱藏。
- `void Toggle()`: 切換開啟或隱藏狀態（按下 F12 鍵時自動觸發）。

### 元素吸管與拾取
- `COMPAT_NODISCARD bool IsPickerActive() const noexcept`: 查詢當前是否處於滑鼠懸停即時拾取模式。
- `void SetPickerActive(bool active) noexcept`: 啟用或關閉元素吸管拾取模式。
- `COMPAT_NODISCARD core::UIElement* GetSelectedElement() const noexcept`: 取得目前所選取之 UIElement 指標。
- `void SetSelectedElement(core::UIElement* element)`: 設定指定之選取除錯元件。

### 渲染與疊加
- `void Render(rendering::DrawingContext& dc, core::UIElement& root)`: 於畫面渲染階段最末端疊加繪製 DevTools 檢視側欄與選取元素外框。
