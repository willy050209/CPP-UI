# ToolTipService 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/ToolTipService.hpp`](../../../include/ui/controls/ToolTipService.hpp)

---

## 概述

`ToolTipService` 是提供懸停提示 (ToolTip) 附加屬性與生命週期管理的全域靜態服務類別。允許開發者直接為任何 [`core::UIElement`](../core_uielement.md) 繫結字串提示或自訂 [`ToolTip`](controls_tooltip.md) 實例，由服務層自動追蹤指標懸停時間並協調頂層 [`OverlayLayer`](controls_overlaylayer.md) 進行顯示與自動隱藏。

---

## 核心靜態方法

### 提示設定與查詢
- `static void SetToolTip(core::UIElement* element, compat::string_view text)`: 為指定元件繫結純文字提示。
- `static void SetToolTip(core::UIElement* element, std::shared_ptr<ToolTip> toolTip)`: 為指定元件繫結自訂 `ToolTip` 控制項。
- `static std::shared_ptr<ToolTip> GetToolTip(core::UIElement* element)`: 取得指定元件目前所繫結的 `ToolTip` 實例。

### 手動顯示與隱藏
- `static void ShowToolTip(core::UIElement* element)`: 強制立即顯示指定元件關聯的提示氣泡。
- `static void HideToolTip(core::UIElement* element)`: 強制立即隱藏指定元件關聯的提示氣泡。

---

## 使用範例

```cpp
#include "ui/controls/Button.hpp"
#include "ui/controls/ToolTipService.hpp"

auto btn = std::make_shared<cppui::controls::Button>("重新整理");
cppui::controls::ToolTipService::SetToolTip(btn.get(), "重新整理目前檢視內容 (F5)");
```
