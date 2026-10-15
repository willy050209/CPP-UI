# DiagnosticsOptions 結構

- **命名空間**: `cppui::diagnostics`
- **標頭檔**: [`include/ui/diagnostics/DiagnosticsOptions.hpp`](../../../include/ui/diagnostics/DiagnosticsOptions.hpp)

---

## 概述

`DiagnosticsOptions` 是 CPPUI 執行期診斷與除錯工具的全域開關設定結構。開發者可透過配置此結構，控制是否顯示 FPS 計數器、啟用局部重繪高亮外框閃爍、或啟用 F12 視覺樹檢查器。

---

## 成員欄位

| 欄位名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `enableDirtyRectOverlay` | `bool` | `false` | 是否在畫面上以半透明色塊疊加顯示局部無效化重繪區域。 |
| `showFpsCounter` | `bool` | `false` | 是否在畫面右上角顯示即時 FPS 與繪圖指令統計儀表。 |
| `enableVisualTreeInspector` | `bool` | `true` | 是否允許透過 F12 快速鍵呼叫 [`VisualTreeInspector`](diagnostics_visualtreeinspector.md)。 |
| `overlayDurationMs` | `uint32_t` | `250` | 局部重繪高亮矩形的淡出停留時間（毫秒）。 |

---

## 使用範例

```cpp
#include "ui/diagnostics/DirtyRectTracker.hpp"

cppui::diagnostics::DiagnosticsOptions opts;
opts.showFpsCounter = true;
opts.enableDirtyRectOverlay = true;

cppui::diagnostics::DirtyRectTracker::Get().SetOptions(opts);
```
