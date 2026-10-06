# 高分屏 DPI 與座標轉換指南 (DPI Scaling & Coordinate Systems)

說明 CPPUI 中 **裝置獨立像素 (Device Independent Pixels, DIP)**、**實體像素 (Physical Pixels)** 與 Windows 高分屏 (Per-Monitor V2) 的座標轉換最佳實踐。

---

## 1. 概念背景 (Concepts)

在現代顯示器（4K、Retina 螢幕）盛行的環境下，作業系統通常會設定 125%、150% 或 200% 的顯示縮放比例。
若直接使用物理像素進行 UI 排版，元件將會因為螢幕 PPI 過高而變得極其細小無法閱讀。

### 核心定義
* **DIP (裝置獨立像素 / 邏輯座標)**：
  標準以 96 DPI（100% 縮放）為基準的虛擬單位。1 DIP 在 100% 縮放時等於 1 個物理像素，在 200% 縮放時等於 2 個物理像素。
  $$\text{Physical Pixels} = \text{DIP} \times \text{DpiScale}$$
* **物理像素 (Physical Pixels)**：
  顯示器硬體真實發光像素網格與 Framebuffer 緩衝區的長寬數值。

---

## 2. 雙重座標轉換模型 (Dual Coordinate Model)

在 CPPUI 中：
1. **視窗尺寸**：
   * `window->GetSize()`：回傳**邏輯尺寸 (DIP)**，供 UI 排版樹 (Layout Engine) 使用。
   * `window->GetFramebufferSize()`：回傳**物理尺寸**，供光柵化畫布配置記憶體使用。
2. **滑鼠輸入事件 (`RawMouseEventData`)**：
   * `event.mouse.position`：自動換算為**視窗客戶區邏輯座標 (DIP)**。
   * `event.mouse.physicalPosition`：原生未經縮放的**物理像素座標**。

---

## 3. Windows Per-Monitor V2 自動感知

在 Win32 實作 [`PlatformWindowWin32`](file:///D:/P/CPP/CPPUI/src/pal/win32/PlatformWindowWin32.cpp) 中，視窗建立前即啟用 Per-Monitor V2 模式：

```cpp
SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
```

### 處理動態跨螢幕拖曳 (`WM_DPICHANGED`)
當使用者將視窗從 100% 螢幕拖拉至 200% 4K 螢幕時：
1. 視窗接收到 DPI 改變通知。
2. 內部重新計算 `m_dpiScale = newDpi / 96.0f`。
3. 派發 [`RawEventType::DpiChanged`](../classes/pal_rawinputevent.md) 事件給應用程式。
4. 應用程式更新 Framebuffer 尺寸並重繪。

---

## 4. 座標實戰範例 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/RawInputEvent.hpp"

void ProcessMouseClick(cppui::pal::IPlatformWindow* window, 
                       const cppui::pal::RawInputEvent& e) {
    using namespace cppui::core;

    float dpi = window->GetDpiScale();

    // 邏輯座標 (UI 判定)
    Point logicalPos = e.mouse.position;

    // 物理座標 (光柵化定位)
    Point physicalPos = e.mouse.physicalPosition;

    compat::println("DPI: {}% | 邏輯: ({}, {}) | 物理: ({}, {})",
        static_cast<int>(dpi * 100),
        logicalPos.x, logicalPos.y,
        physicalPos.x, physicalPos.y);

    // 驗證數學轉換
    // physicalPos.x ≈ logicalPos.x * dpi
}
```

---

## 請參閱 (See Also)
* [`RawInputEvent` 結構參考](../classes/pal_rawinputevent.md)
* [`IPlatformWindow` 介面參考](../classes/pal_iplatformwindow.md)
* [快速入門指南](quickstart.md)
