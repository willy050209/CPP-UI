# SkiaImageBridge 類別 (cppui::graphics::SkiaImageBridge)

`SkiaImageBridge` 類別定義於 `include/ui/graphics/SkiaImageBridge.hpp`，提供 CPPUI 圖形子系統與 Google Skia 向量著色核心之間的零拷貝資料交換與硬體加速濾鏡橋接。

---

## 1. 元資料與規格 (Metadata & Specification)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/SkiaImageBridge.hpp"`
* **實作檔**：`src/graphics/SkiaImageBridge.cpp`
* **巨集條件**：受 `CPPUI_HAS_SKIA` 編譯常數保護，系統未啟用 Skia 時自動優雅回退。

---

## 2. 語法與宣告 (Syntax & Declaration)

```cpp
namespace cppui {
namespace graphics {

class SkiaImageBridge {
public:
    /// <summary>查詢系統是否具備原生 Skia 支援</summary>
    COMPAT_NODISCARD static bool IsSkiaAvailable() noexcept;

    /// <summary>
    /// 嘗試使用 Skia 向量著色管線執行零拷貝高斯模糊加速
    /// </summary>
    static bool TryGaussianBlur(Bitmap& bmp, float sigmaX, float sigmaY);
};

} // namespace graphics
} // namespace cppui
```

---

## 3. 核心機制剖析

### 3.1 零記憶體拷貝映射 (`SkBitmap::installPixels`)
當啟用 Skia 時，`SkiaImageBridge` 不會重新複製整個像素陣列，而是透過：
```cpp
SkImageInfo info = SkImageInfo::Make(width, height, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
SkBitmap srcBitmap;
srcBitmap.installPixels(info, rawPixels, bmp.GetStride());
```
直接將 `Bitmap` 的連續實體記憶體指標借給 Skia 畫布，達到**零記憶體複製 (Zero-Copy)**。

### 3.2 向量著色與濾鏡加速
藉由 `SkImageFilters::Blur` 與 `SkCanvas::drawImage`，Skia 可自動運用 CPU SIMD 向量指令集（如 SSE4.2、AVX2、ARM Neon）或底層 GPU 著色器完成百萬級像素平滑處理。

### 3.3 雙引擎自動回退機制
若系統未安裝 Skia 或建置旗標未開啟，`IsSkiaAvailable()` 傳回 `false`，`ImageFilters::GaussianBlur` 會自動切換至 [`ConvolutionEngine`](graphics_convolution.md) 的本機多核心定點數卷積管線，呼叫端無需撰寫任何分支程式碼。
