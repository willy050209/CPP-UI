# ScrollViewer 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/ScrollViewer.hpp`](../../../include/ui/controls/ScrollViewer.hpp)
- **繼承關係**: 繼承自 [`controls::ContentControl`](controls_contentcontrol.md)

---

## 概述

`ScrollViewer` 是捲動視區容器控制項，用於容納尺寸超出容器邊界的單一內容（如大型列表、文字區塊或自訂面板）。
`ScrollViewer` 預設啟用邊界剪裁（`ClipToBounds = true`），自動計算內容真實延伸尺寸（Extent）與可視視區尺寸（Viewport），並在內容高度超出視區時呈現自適應捲軸軌道與滑塊。卷軸採用 `OnPostRender` 後置渲染機制，確保浮動於內容之上，且所有滾動出界或未捲入之內容均受到硬體/像素級邊界剪裁防護。

---

## 建構子

- `ScrollViewer()`: 建立空白 `ScrollViewer`，預設啟用剪裁並註冊指標與滾輪事件處理器。
- `explicit ScrollViewer(std::shared_ptr<core::UIElement> content)`: 建立帶有指定子內容的 `ScrollViewer`。

---

## 核心成員方法

### 捲動偏移與視區查詢
- `COMPAT_NODISCARD float GetVerticalOffset() const noexcept`: 取得目前垂直捲動偏移像素值。
- `void SetVerticalOffset(float offset)`: 設定垂直捲動偏移量（自動夾取於 `[0, ExtentHeight - ViewportHeight]` 區間）。
- `COMPAT_NODISCARD float GetHorizontalOffset() const noexcept`: 取得目前水平捲動偏移像素值。
- `void SetHorizontalOffset(float offset)`: 設定水平捲動偏移量（自動夾取於 `[0, ExtentWidth - ViewportWidth]` 區間）。
- `COMPAT_NODISCARD float GetExtentHeight() const noexcept`: 取得內容之整體垂直高度。
- `COMPAT_NODISCARD float GetExtentWidth() const noexcept`: 取得內容之整體水平寬度。
- `COMPAT_NODISCARD float GetViewportHeight() const noexcept`: 取得目前容器可視區域高度。
- `COMPAT_NODISCARD float GetViewportWidth() const noexcept`: 取得目前容器可視區域寬度。

### 滾動操作與導航捷徑
- `void ScrollToHome()`: 捲動至內容最頂端（偏移量歸零）。
- `void ScrollToEnd()`: 捲動至內容最底端。
- `void LineUp()`: 向上捲動一個步進距離（`m_scrollStep`，預設 32px）。
- `void LineDown()`: 向下捲動一個步進距離。
- `void PageUp()`: 向上捲動一整頁視區高度。
- `void PageDown()`: 向下捲動一整頁視區高度。

### 卷軸規格設定
- `COMPAT_NODISCARD float GetScrollStep() const noexcept`: 取得單次滾輪或按鍵之步進像素距離。
- `void SetScrollStep(float step) noexcept`: 設定步進像素距離（最小值為 1.0f）。
- `COMPAT_NODISCARD float GetScrollBarWidth() const noexcept`: 取得垂直卷軸之寬度（預設 10px）。
- `void SetScrollBarWidth(float width) noexcept`: 設定垂直卷軸寬度（最小值為 4.0f）。

### 排版與渲染管線
- `core::Size MeasureOverride(core::Size availableSize) override`: 測量階段，向子內容提供未受限垂直高度以計算實際延伸尺寸。
- `void ArrangeOverride(core::Size finalSize) override`: 排列階段，配置視區大小並依 `(-horizontalOffset, -verticalOffset)` 配置子節點。
- `void OnRender(rendering::DrawingContext& dc) override`: 繪製容器背景與邊框。
- `void OnPostRender(rendering::DrawingContext& dc) override`: 後置渲染階段，於子節點內容上方繪製卷軸軌道與滑塊。
- `void ApplyTheme(const theme::Theme& theme) override`: 響應主題明暗模式動態切換卷軸軌道、滑塊與互動色彩。

---

## 範例程式碼

```cpp
#include "ui/controls/ScrollViewer.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/controls/TextBlock.hpp"

using namespace cppui;

auto scrollViewer = std::make_shared<controls::ScrollViewer>();
scrollViewer->SetWidth(400.0f);
scrollViewer->SetHeight(300.0f);

auto stackPanel = std::make_shared<layout::StackPanel>(layout::Orientation::Vertical);
stackPanel->SetSpacing(8.0f);

for (int32_t i = 1; i <= 50; ++i) {
    stackPanel->AddChild(std::make_shared<controls::TextBlock>("清單項目 #" + std::to_string(i)));
}

scrollViewer->SetContent(stackPanel);
```
