# ContentControl 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/ContentControl.hpp`](../../../include/ui/controls/ContentControl.hpp)
- **繼承關係**: 繼承自 [`controls::Control`](controls_control.md)

---

## 概述

`ContentControl` 是容納單一內容元素的控制項基底類別（如按鈕、標籤面板）。支援動態傳入純字串（自動包裝為內部唯讀 [`TextBlock`](controls_textblock.md) 並繼承前景色彩）或任意自訂 [`UIElement`](core_uielement.md) 子元素，自動在排版與繪製階段套用四周 `Padding` 與邊框內縮。

---

## 核心成員方法

### 內容管理
- `ContentControl()`: 預設建構函式。
- `explicit ContentControl(compat::string_view text)`: 以字串內容建構。
- `explicit ContentControl(std::shared_ptr<core::UIElement> content)`: 以自訂視覺元素建構。
- `COMPAT_NODISCARD std::shared_ptr<core::UIElement> GetContent() const noexcept`: 取得目前承載之子元件指標。
- `void SetContent(std::shared_ptr<core::UIElement> content)`: 動態設定子元件，自動更新視覺樹階層並觸發 `InvalidateMeasure()`。
- `void SetContent(compat::string_view text)`: 動態將內容設定為文字字串。
- `COMPAT_NODISCARD compat::string_view GetContentText() const noexcept`: 若目前內容為文字，回傳其文字視圖。
- `void SetContentText(compat::string_view text)`: 更新目前文字內容。

### 排版與繪製
- `core::Size MeasureOverride(core::Size availableSize) override`: 先利用 `DeflateSize` 縮減可用空間度量內容子元件，再透過 `InflateSize` 膨脹回傳整體所需空間。
- `void ArrangeOverride(core::Size finalSize) override`: 將內容子元件排列於內縮後的客戶區域。
- `void OnRender(rendering::DrawingContext& dc) override`: 呼叫基底 `RenderBackgroundAndBorder` 繪製外觀。
