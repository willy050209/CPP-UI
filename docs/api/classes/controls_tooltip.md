# ToolTip 類別

- **命名空間**: `cppui::controls`
- **標頭檔**: [`include/ui/controls/ToolTip.hpp`](../../../include/ui/controls/ToolTip.hpp)
- **繼承關係**: 繼承自 [`controls::ContentControl`](controls_contentcontrol.md)

---

## 概述

`ToolTip` 是輕量級懸停輔助提示控制項。用於承載純文字或自訂豐富視覺內容（圖示、格式化文字等），在使用者將游標懸停於目標元件上方時自動浮動呈現。內部樣式與配色自動遵循 [`ThemeManager`](../theme_thememanager.md) 定義的主題色彩與圓角邊界。

---

## 核心成員方法

### 建構子
- `ToolTip()`: 建構預設空白提示實例。
- `explicit ToolTip(compat::string_view text)`: 建構具備純文字內容之提示實例。

### 樣式客製
- `void ApplyTheme(const theme::Theme& theme) override`: 套用系統主題的背景底色、文字顏色與外框陰影。

---

## 搭配 ToolTipService 使用

通常不直接手動操作 `ToolTip` 的開啟與關閉，而是透過 [`ToolTipService`](controls_tooltipservice.md) 靜態附加於任意 [`UIElement`](../core_uielement.md)：

```cpp
#include "ui/controls/Button.hpp"
#include "ui/controls/ToolTipService.hpp"

auto btn = std::make_shared<cppui::controls::Button>("送出");
cppui::controls::ToolTipService::SetToolTip(btn.get(), "點擊以儲存變更並上傳至伺服器");
```
