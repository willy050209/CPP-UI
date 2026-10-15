# cppui::diagnostics 命名空間

- **標頭檔目錄**: `include/ui/diagnostics/`

---

## 概述

`cppui::diagnostics` 命名空間提供 CPPUI 執行期即時診斷、元素檢查器 (DevTools)、局部重繪區域視覺化與效能分析工具。

---

## 包含類別與結構

| 型別名稱 | 分類 | 說明 |
| :--- | :--- | :--- |
| [`VisualTreeInspector`](../classes/diagnostics_visualtreeinspector.md) | 類別 (單例) | 視覺樹即時檢查器，支援 F12 喚出、元素吸管拾取模式與邊界除錯框。 |
| [`DirtyRectTracker`](../classes/diagnostics_dirtyrecttracker.md) | 類別 (單例) | 局部無效化重繪區域追蹤器，支援畫面髒區半透明閃爍高亮與 FPS 統計疊加層。 |
| [`DiagnosticsOptions`](../classes/diagnostics_diagnosticsoptions.md) | 結構體 | 全域執行期診斷選項，控制 FPS 顯示、重繪高亮淡出時間與 F12 檢查器開關。 |

---

## 使用範例

```cpp
#include "ui/diagnostics/DiagnosticsOptions.hpp"
#include "ui/diagnostics/DirtyRectTracker.hpp"
#include "ui/diagnostics/VisualTreeInspector.hpp"

// 開啟 FPS 計數器與重繪區域高亮
cppui::diagnostics::DiagnosticsOptions opts;
opts.showFpsCounter = true;
opts.enableDirtyRectOverlay = true;
opts.enableVisualTreeInspector = true;

cppui::diagnostics::DirtyRectTracker::Get().SetOptions(opts);

// 程式化切換 VisualTreeInspector
cppui::diagnostics::VisualTreeInspector::Get().Toggle();
```
