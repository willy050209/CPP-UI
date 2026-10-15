# DirtyRectTracker 類別

- **命名空間**: `cppui::diagnostics`
- **標頭檔**: [`include/ui/diagnostics/DirtyRectTracker.hpp`](../../../include/ui/diagnostics/DirtyRectTracker.hpp)

---

## 概述

`DirtyRectTracker` 是局部重繪區域追蹤與 FPS 統計疊加層管理類別。在畫面渲染週期中自動記錄每一幀發生視覺無效化（`InvalidateVisual`）的邊界矩形，並在畫面上方即時繪製半透明動態淡出的重繪閃爍外框，同時支援即時 FPS 與繪圖指令計數浮水印，協助開發者定位過度重繪（Overdraw）瓶頸。

---

## 核心成員方法

### 單例與選項
- `static DirtyRectTracker& Get() noexcept`: 取得追蹤器全域單例。
- `void SetOptions(const DiagnosticsOptions& options)`: 套用全域診斷選項設定。
- `COMPAT_NODISCARD const DiagnosticsOptions& GetOptions() const noexcept`: 讀取全域診斷選項。

### 重繪區域記錄
- `void RecordDirtyRect(const core::Rect& rect)`: 記錄當前幀的局部重繪區域邊界。
- `void ClearDirtyRects()`: 清除累積之歷史重繪區域。
- `COMPAT_NODISCARD size_t GetDirtyRectCount() const noexcept`: 取得目前已記錄的重繪矩形總數。

### 渲染疊加層
- `void Render(rendering::DrawingContext& dc)`: 於畫面最頂層疊加繪製半透明髒區高亮與右上角 FPS 統計儀表。
