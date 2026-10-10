# IRenderContext 與 IRenderTarget 介面 (cppui::rendering)

定義渲染環境與光柵化畫布目標的跨平台底層介面。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::rendering`](../namespaces/cppui_rendering.md)
* **標頭檔**：`#include <ui/rendering/backend/IRenderContext.hpp>`
* **模組**：`CPPUI Rendering`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace rendering {

class IRenderTarget {
public:
    virtual ~IRenderTarget() = default;

    virtual uint32_t GetWidth() const noexcept = 0;
    virtual uint32_t GetHeight() const noexcept = 0;

    virtual void Clear(core::Color color) = 0;
    virtual void FillRect(const core::Rect& rect, core::Color color) = 0;

    virtual uint32_t* GetPixelBuffer() noexcept = 0;

    virtual void DrawText(compat::string_view text, core::Point origin, compat::string_view family, float size, uint16_t weight, uint8_t style, core::Color color, const core::Rect& clipRect = core::Rect());
    virtual void DrawTextLayout(void* nativeLayoutHandle, core::Point origin, core::Color color, const core::Rect& clipRect = core::Rect());
    virtual void* GetSkCanvas() noexcept { return nullptr; }
};

class IRenderContext {
public:
    virtual ~IRenderContext() = default;

    virtual bool Init(void* nativeWindowHandle, uint32_t width, uint32_t height) = 0;
    virtual void Resize(uint32_t width, uint32_t height) = 0;
    virtual IRenderTarget* BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Destroy() = 0;
};

} // namespace rendering
} // namespace cppui
```

---

## 簡介 (Description)

CPPUI 的繪圖光柵化後端被抽象為兩組介面：
1. **`IRenderTarget`（目標畫布）**：負責提供當前影格的物理像素寬高、基礎清屏、實心矩形填充以及原始像素指標（供軟體光柵器寫入）。
2. **`IRenderContext`（渲染環境）**：負責與作業系統原生視窗繫結、管理前/後雙緩衝區 (Double Buffering) 交換，並處理視窗尺寸調整。

---

## 現有實作 (Implementations)

* **`SoftwareRenderContext` / `SoftwareRenderTarget`**：
  純軟體 32-bit BGRA DIB 光柵化後端。在 Windows 下透過 `SetDIBitsToDevice` 將記憶體位圖零拷貝/直接寫入 GDI 裝置內容 (`HDC`)，提供最強向下相容性。
* **`SkiaRenderContext`**：
  封裝未來 Skia 硬體加速管線的環境後端。

---

## 方法說明 (Methods)

### `IRenderTarget` 方法
* `GetWidth()` / `GetHeight()`：取得畫布實體像素尺寸。
* `Clear(color)`：以指定顏色填滿整張畫布。
* `FillRect(rect, color)`：填充特定軸對齊矩形區域。
* `GetPixelBuffer()`：取得連續的 32-bit ARGB/BGRA 像素記憶體位址。
* `DrawText(text, origin, family, size, weight, style, color, clipRect)`：於指定座標光柵化單行純文字，支援指定局部剪裁矩形。
* `DrawTextLayout(nativeLayoutHandle, origin, color, clipRect)`：光柵化原生排版文字佈局，支援指定局部剪裁矩形與 Fast Culling。
* `GetSkCanvas()`：取得底層 `SkCanvas*`（若當前為 Skia 後端）。

### `IRenderContext` 生命週期
* `Init(handle, width, height)`：初始化渲染環境並綁定原生視窗 Handle (`HWND` 等)。
* `Resize(width, height)`：當視窗大小改變時重設 Framebuffer 緩衝區。
* `BeginFrame()`：開啟當前幀，鎖定並回傳可供繪製的 `IRenderTarget`。
* `EndFrame()`：結束當前幀繪製，執行畫面翻轉 (SwapBuffers / 畫面提交)。
* `Destroy()`：銷毀渲染管線並釋放原生資源。

---

## 範例程式碼 (Example)

```cpp
#include "src/rendering/backend/SoftwareRenderContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"

void RenderLoopCycle(cppui::rendering::IRenderContext* context, 
                     const cppui::rendering::DisplayList& dl) {
    using namespace cppui::rendering;

    // 1. 開啟當前幀
    IRenderTarget* target = context->BeginFrame();
    if (!target) return;

    // 2. 重播 DisplayList 指令
    DisplayListRenderer renderer;
    renderer.Render(dl, target);

    // 3. 提交並刷新至螢幕
    context->EndFrame();
}
```

---

## 請參閱 (See Also)
* [`DisplayListRenderer` 重播器](rendering_displaylistrenderer.md)
* [`IPlatformWindow` 原生視窗](pal_iplatformwindow.md)
* [`cppui::rendering` 命名空間](../namespaces/cppui_rendering.md)
