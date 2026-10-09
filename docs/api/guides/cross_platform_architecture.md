# CPPUI 跨平台抽象架構指南 (Cross-Platform Architecture Guide)

本指南說明 CPPUI 如何從早期 Win32 依賴架構過渡並解耦為完全跨平台、支援零桌面環境 (Headless) 的純 C++ 視窗與渲染架構。

---

## 1. 架構核心理念

傳統 C++ GUI 框架往往深度耦合特定作業系統的 API（例如 Windows HWND、GDI DC、DirectWrite、macOS NSWindow、Linux X11/Wayland）。CPPUI 透過嚴謹的三層解耦機制實現真正的可移植性：

```
+-----------------------------------------------------------+
|                   CPPUI 控制項與佈局層                     |
|           (DSL, Widgets, LayoutEngine, Animation)         |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                   CPPUI 跨平台抽象層 (PAL)                 |
|             (IPlatformWindow, RawInputEvent, RunLoop)     |
+-----------------------------------------------------------+
         |                       |                       |
         v                       v                       v
+-----------------------+ +-----------------------+ +-----------------------+
|  PlatformWindowWin32  | |  PlatformWindowX11    | | PlatformWindowGeneric |
| (HWND, DIBits, WinMsg)| | (Xlib, XImage, poll)  | |(純記憶體, 事件模擬注入)|
+-----------------------+ +-----------------------+ +-----------------------+
```

---

## 2. 視窗與展示抽象 (PAL)

### 2.1 統一視窗工廠 (`PlatformWindowFactory`)
使用者或測試框架透過 [`PlatformWindowFactory::Create`](../classes/pal_platformwindowfactory.md) 建立視窗：
* 在 Windows 平台上，動態建立啟用 Per-Monitor V2 DPI 感知的 `PlatformWindowWin32`。
* 在 Linux 平台上，工廠主動偵測 `DISPLAY` 環境變數：
  * 若處於 WSLg 或 Linux 實體桌面環境且編譯啟用 X11，動態實例化原生 [`PlatformWindowX11`](../classes/pal_platformwindowx11.md)。
  * 若處於無顯示卡/無 Display 之 Headless CI Docker 容器環境，自動安全回退至純 C++ 記憶體視窗 [`PlatformWindowGeneric`](../classes/pal_platformwindowgeneric.md)。
* 同時提供明確指定入口：
  * `PlatformWindowFactory::CreateX11()`：強制建立 Linux 原生實體視窗。
  * `PlatformWindowFactory::CreateGeneric()`：允許在任何平台強制建立純記憶體視窗進行 CI 離線測試。

### 2.2 幀緩衝呈現抽象 (`PresentFramebuffer`)
過去 Demo 範例直接呼叫 Win32 GDI API（`GetDC(hwnd)`, `StretchDIBits(...)`, `ReleaseDC(hwnd, hdc)`），導致程式碼無法在其他作業系統編譯。

現在所有視窗實作皆實作 `IPlatformWindow::PresentFramebuffer`：
```cpp
virtual void PresentFramebuffer(compat::span<const uint32_t> pixels, uint32_t width, uint32_t height) = 0;
```
上層渲染引擎只需傳入光柵化後的 ARGB32/BGRA32 像素緩衝區視圖（以現代 `compat::span<const uint32_t>` 借用語意傳遞），各平台自適應將像素呈現到畫面上或儲存至記憶體中。

---

## 3. 文字排版與光柵化解耦 (Text Engine Decoupling)

CPPUI 文字管線由 `FontManager`、`TextShaper`、`TextLayoutEngine` 組成：
1. **條件編譯 DirectWrite**：
   `DirectWriteTextEngine.hpp` 僅在 `#ifdef _WIN32` 下引入。
2. **純 C++ 多行換行與點擊測試回退**：
   在非 Windows 系統或未啟用 DirectWrite 的情況下，`TextLayoutEngine` 自動使用純 C++ 多行換行演算法，解析 `\n` 與寬度換行，並依據字符等寬或字型度量精確計算：
   * `HitTestPoint(float x, float y)`：將螢幕點擊座標映射至字符索引。
   * `HitTestTextPosition(uint32_t textPosition)`：取得字符的光標矩陣邊界。
3. **內建 8x8 ASCII 軟體光柵字模庫**：
   `SoftwareRenderContext` 內建 95 個標準可顯示 ASCII 字符的 8x8 點陣字模表 (`s_font8x8`)。當系統缺乏原生字型引擎時，軟體光柵化引擎仍可直接在記憶體畫布上繪製清晰的文字與標籤，達成真正的零外部依賴。

---

## 4. Headless 自動化與 CI 矩陣測試

借助 `PlatformWindowGeneric`，開發者可以在沒有 X11 Server、Wayland 或 Windows 桌面 Session 的 Docker 容器及 GitHub Actions CI Runner 中執行完整測試：
```cpp
auto window = PlatformWindowFactory::CreateGeneric(desc);
auto genericWin = std::dynamic_pointer_cast<PlatformWindowGeneric>(window);

// 模擬使用者注入點擊事件
genericWin->InjectEvent(RawInputEvent::MakeMouseDown(100.0f, 50.0f, MouseButton::Left));
genericWin->PollEvents();

// 驗證 Framebuffer 渲染結果
auto pixels = genericWin->GetFramebufferPixels();
assert(!pixels.empty());
```

---

## 5. 請參閱 (See Also)
* [`IPlatformWindow` 介面參考](../classes/pal_iplatformwindow.md)
* [`PlatformWindowGeneric` 通用視窗](../classes/pal_platformwindowgeneric.md)
* [`PlatformWindowFactory` 工廠類別](../classes/pal_platformwindowfactory.md)
* [`Skia 向量渲染管線指南`](skia_rendering_pipeline.md)
