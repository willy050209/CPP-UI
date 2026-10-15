# SkiaRenderContext 類別 (cppui::rendering::SkiaRenderContext)

Google Skia 2D 向量繪圖核心渲染後端，提供高效能的向量畫布渲染、GPU/CPU 表面管理與流暢指令回放。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::rendering`](../namespaces/cppui_rendering.md)
* **標頭檔**：`#include "src/rendering/backend/SkiaRenderContext.hpp"`
* **介面標頭檔**：`#include <ui/rendering/backend/IRenderContext.hpp>`
* **模組**：`CPPUI Rendering Engine`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace rendering {

class SkiaRenderContext : public IRenderContext {
public:
    SkiaRenderContext();
    ~SkiaRenderContext() override;

    bool Init(void* nativeWindowHandle, uint32_t width, uint32_t height) override;
    void Resize(uint32_t width, uint32_t height) override;
    IRenderTarget* BeginFrame() override;
    void EndFrame() override;
    void Destroy() override;

    COMPAT_NODISCARD bool IsSkiaActive() const noexcept;
};

} // namespace rendering
} // namespace cppui
```

---

## 簡介 (Description)

`SkiaRenderContext` 是 CPPUI 的高畫質、高效能 2D 向量渲染核心後端：
1. **Google Skia (m151) 整合**：支援最新的 Google Skia 2D 引擎，使用 `SkSurfaces::Raster` 或 GPU 後端，具備抗鋸齒幾何形狀、路徑裁剪、高質量陰影、複雜貝茲曲線繪製等能力。
2. **原生 DisplayList 轉譯**：配合 `DisplayListRenderer`，直接將 `DisplayList` 中的 `DrawOp` 指令流映射為原生 `SkCanvas` 呼叫（如 `drawRect`, `drawRRect`, `save`, `restore`, `clipRect`）。
3. **雙引擎優雅回退 (Dual-Engine Fallback)**：
   * 當專案以 `-DCPPUI_ENABLE_SKIA=ON` 建置且包含 Skia 庫時，`CPPUI_HAS_SKIA=1`，`IsSkiaActive()` 為 `true`，全面啟用向量引擎加速。
   * 當處於未安裝 Skia 的輕量環境、相容性 CI 測試或 C++14 矩陣編譯時，`SkiaRenderContext` 自動降級委派給內部 `SoftwareRenderContext`，確保編譯與測試 100% 綠燈無中斷。

---

## 方法說明 (Method Specifications)

### `Init(void* nativeWindowHandle, uint32_t width, uint32_t height)`
初始化渲染上下文。
* **參數**：
  * `nativeWindowHandle`：原生視窗代碼（可為 Win32 HWND 或通用視窗指標）。
  * `width`：視窗客戶區像素寬度。
  * `height`：視窗客戶區像素高度。
* **傳回值**：若初始化成功傳回 `true`；若表面建立失敗傳回 `false`。

### `Resize(uint32_t width, uint32_t height)`
當視窗尺寸或 DPI 改變時重塑底層 Skia 表面緩衝區。

### `BeginFrame()`
啟動新的一幀繪圖週期，清除畫布並取得當前繪圖目標 `IRenderTarget*`。若啟用了 Skia，該目標內部持有原生 `SkCanvas*`。

### `EndFrame()`
結束當前幀繪製，觸發 `SkSurface` 的 flush 操作，將渲染內容提交至緩衝區。

### `IsSkiaActive()`
```cpp
COMPAT_NODISCARD bool IsSkiaActive() const noexcept;
```
傳回布林值，指示當前是否正在使用原生 Google Skia 引擎（`true`）或回退軟體光柵化引擎（`false`）。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "src/rendering/backend/SkiaRenderContext.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"

void RenderWithSkia(void* windowHandle, uint32_t width, uint32_t height) {
    using namespace cppui::rendering;

    SkiaRenderContext context;
    if (!context.Init(windowHandle, width, height)) {
        compat::println("初始化 SkiaRenderContext 失敗");
        return;
    }

    compat::println("引擎狀態: {}", context.IsSkiaActive() ? "Skia m151 原生向量引擎" : "純 C++ 軟體回退引擎");

    IRenderTarget* target = context.BeginFrame();
    if (target) {
        DisplayList list;
        // 建構繪圖指令...
        DisplayListRenderer renderer;
        renderer.Render(list, *target);
    }
    context.EndFrame();
}
```

---

## 請參閱 (See Also)
* [`IRenderContext` 介面參考](rendering_irendercontext.md)
* [`DisplayListRenderer` 繪圖轉譯器](rendering_displaylistrenderer.md)
* [`Skia 向量渲染管線指南`](../guides/skia_rendering_pipeline.md)
