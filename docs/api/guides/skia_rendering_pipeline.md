# Skia 向量渲染管線指南 (Skia Rendering Pipeline Guide)

本指南介紹 CPPUI 如何將最新 Google Skia (m151) 整合為核心 2D 向量繪圖後端，以及雙引擎架構（Skia + Software Fallback）的設計與部署方式。

---

## 1. 為什麼選擇 Google Skia？

Google Skia 是 Chrome、Android、Flutter 等現代圖形產品背後的頂級 2D 繪圖引擎。導入 Skia 為 CPPUI 帶來顯著的圖形渲染能力提升：
* **高品質抗鋸齒 (High-Quality Antialiasing)**：圓角矩形、多邊形、貝茲曲線皆具備次像素平滑邊緣。
* **路徑裁剪與透明度層疊 (Path Clipping & Opacity Layers)**：原生支援高效複雜裁切與圖層透明度混合。
* **色彩漸層與陰影 (Shaders & Shadows)**：支援多色線性漸層、徑向漸層與物理光影渲染。
* **GPU 加速就緒**：架構上可無縫擴充 Direct3D 12, Vulkan, Metal 或 OpenGL 後端。

---

## 2. 雙引擎架構設計 (Dual-Engine Architecture)

為了確保在未安裝 Skia 的輕量開發環境、舊版編譯器（如 C++14 矩陣編譯）或無網路的 CI 環境中專案依然能正常建置，CPPUI 採用了**無縫雙引擎架構**：

```
+-----------------------------------------------------------+
|                   CPPUI 控制項與渲染轉譯層                  |
|                 (DisplayList & DrawOp)                    |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|              DisplayListRenderer 指令回放分發器             |
+-----------------------------------------------------------+
         |                                          |
         | (若 target.GetSkCanvas() != nullptr)     | (回退路徑)
         v                                          v
+------------------------+             +------------------------+
|      SkCanvas 原生調用  |             |  SoftwareRenderContext |
|  drawRect / drawRRect  |             | (純 C++ 軟體光柵化引擎)  |
|  save / restore / clip |             | (含 8x8 ASCII 字符點陣) |
+------------------------+             +------------------------+
```

### 2.1 介面抽象與原生 Canvas 探測
在 `IRenderTarget` 介面中加入了非侵入式的探針函式：
```cpp
virtual void* GetSkCanvas() noexcept { return nullptr; }
```
`SkiaRenderTarget` 覆寫此函式並回傳原生 `SkCanvas*` 指標。在 `DisplayListRenderer::Render()` 執行期間：
1. 若檢測到 `target.GetSkCanvas()` 非空，立即走高效原生 Skia 指令迴圈，將 `PushClipRectOp`, `PopClip`, `PushOpacity`, `PopOpacity`, `DrawRect`, `DrawRoundedRect`, `DrawLine`, `DrawText` 轉發至 `SkCanvas`。
2. 若為一般渲染目標，則回退至原有的純 C++ 軟體光柵化實作。

---

## 3. 下載與安裝 Skia (m151)

專案提供了跨平台的自動化下載與配置腳本：

### 使用 PowerShell 下載預編譯 Skia (Windows x64)
```powershell
powershell.exe -ExecutionPolicy Bypass -File "scripts\setup_skia.ps1"
```
該腳本會執行：
1. 從經過驗證的 Release 下載相容之 Skia m151 prebuilt x64 二進位發行包。
2. 解壓縮至專案下的 `third_party/skia/` 目錄（包含 `include/` 與 `out/Release-x64/skia.lib`）。
3. 驗證標頭檔與程式庫完整性。

---

## 4. CMake 建置配置

CPPUI 提供了完整的 CMake Find 模組 `cmake/FindSkia.cmake`。

### 啟用 Skia 繪圖後端
```powershell
cmake.exe -B build -S . -DCPPUI_ENABLE_SKIA=ON
cmake.exe --build build --config Release
```

當啟用此選項且偵測到 Skia 庫時：
* CMake 自動定義預處理器巨集 `CPPUI_HAS_SKIA=1`。
* 自動連結 `Skia::Skia` 標靶。
* `SkiaRenderContext` 啟用原生 `SkSurfaces::Raster` 畫布渲染。

若未指定 `-DCPPUI_ENABLE_SKIA=ON` 或未下載 Skia：
* `CPPUI_HAS_SKIA` 設為 `0`。
* `SkiaRenderContext` 自動降級委派給內部 `SoftwareRenderContext`，不依賴任何外部 `.lib`。

---

## 5. 程式碼使用範例

```cpp
#include "compat/Compat.hpp"
#include "src/rendering/backend/SkiaRenderContext.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"

void DrawCardWithSkia(void* nativeWindow, uint32_t width, uint32_t height) {
    using namespace cppui::rendering;

    SkiaRenderContext context;
    context.Init(nativeWindow, width, height);

    // 確認當前運行之繪圖引擎
    compat::println("目前繪圖後端: {}", context.IsSkiaActive() ? "Google Skia m151" : "軟體光柵回退");

    IRenderTarget* target = context.BeginFrame();
    if (target) {
        DisplayList dl;
        // 繪製半透明圓角卡片
        dl.PushOpacity(0.95f);
        dl.DrawRoundedRect(cppui::core::Rect(20, 20, 300, 200), 12.0f, cppui::core::Color::White());
        dl.DrawText("Skia Vector Rendering", cppui::core::Point(40, 50), cppui::core::Color::Black());
        dl.PopOpacity();

        DisplayListRenderer renderer;
        renderer.Render(dl, *target);
    }
    context.EndFrame();
}
```

---

## 6. 請參閱 (See Also)
* [`SkiaRenderContext` 類別參考](../classes/rendering_skiarendercontext.md)
* [`DisplayListRenderer` 繪圖轉譯器](../classes/rendering_displaylistrenderer.md)
* [`跨平台 PAL 架構指南`](cross_platform_architecture.md)
