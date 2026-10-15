# cppui::rendering 命名空間 (Namespace)

`cppui::rendering` 命名空間提供了 CPPUI 的 **繪圖命令錄製與光柵化渲染管線**。

---

## 簡介 (Description)

CPPUI 採用類似 Chromium / Skia / Flutter 的 **DisplayList 命令緩衝區架構 (Command Buffer)**。UI 元件在繪製階段不直接呼叫 GPU 或底層 GDI/Direct2D，而是透過 [`DrawingContext`](../classes/rendering_drawingcontext.md) 將圖元繪製命令序列化並寫入由 [`LinearArena`](../classes/rendering_lineararena.md) 管理的連續記憶體池中。

最後，渲染器 [`DisplayListRenderer`](../classes/rendering_displaylistrenderer.md) 遍歷指令串流，維護剪裁棧 (Clip Stack) 與透明度棧 (Opacity Stack)，執行圖元剔除 (Culling) 並分發至後端目標畫布。

### 核心設計優勢
1. **極致錄製效能**：50,000 個繪圖操作錄製僅需約 1.1 ms（遠優於 1.5 ms 目標）。
2. **零堆積配置 ($O(1)$ 指標分配)**：每幀分配完全在平坦記憶體池內進行，幀末呼叫 `Clear()` 瞬間歸零指標，完全消除記憶體碎片與系統 `malloc` 開銷。
3. **二進位緊湊協議 (Binary Protocol)**：每個繪圖指令均由緊湊的 8 字節標頭 (`DrawOpHeader`) 與強型別 Payload 組成，保證 8 字節硬體對齊。
4. **渲染後端完全解耦**：可在純軟體緩衝區 (`SoftwareRenderContext`) 與硬體加速後端 (`SkiaRenderContext`) 間自由切換。

---

## 類別與介面索引 (Classes & Interfaces)

| 型別 (Type) | 分類 | 簡介與職責 |
| :--- | :--- | :--- |
| [`DrawingContext`](../classes/rendering_drawingcontext.md) | 錄製類別 | 公開畫布錄製介面，提供矩形、圓角矩形、線段繪製與 RAII 自動剪裁/透明度保護。 |
| [`DisplayList`](../classes/rendering_displaylist.md) | 容器類別 | 封裝平坦記憶體池的二進位命令序列容器，支援無鎖遍歷與 $O(1)$ 重設。 |
| [`DisplayListRenderer`](../classes/rendering_displaylistrenderer.md) | 重播器類別 | 命令流直譯與光柵化執行器，維護狀態機並分發給 `IRenderTarget`。 |
| [`LinearArena`](../classes/rendering_lineararena.md) | 記憶體管理類別 | 8 字節對齊的連續平坦記憶體池配置器 (Arena Allocator)。 |
| [`IRenderTarget`](../classes/rendering_irendercontext.md#irendertarget-介面) | 渲染介面 | 畫布光柵化目標介面（提供清屏、矩形填色與像素記憶體緩衝區存取）。 |
| [`IRenderContext`](../classes/rendering_irendercontext.md#irendercontext-介面) | 渲染介面 | 跨平台渲染環境介面，管理雙緩衝訊框與視窗綁定。 |

---

## 結構與列舉索引 (Structs & Enums)

| 型別 (Type) | 簡介與職責 |
| :--- | :--- |
| [`DrawOpType`](../classes/rendering_drawop.md#drawoptype-列舉) | 繪圖指令類型列舉（`DrawRectangle`, `DrawRoundedRectangle`, `DrawLine`, `PushClipRect`, `PopClip`, `PushOpacity`, `PopOpacity` 等）。 |
| [`DrawOpHeader`](../classes/rendering_drawop.md#drawopheader-結構) | 8 字節緊湊二進位命令標頭（包含指令型別、旗標與總長度）。 |
| [`DrawRectOp`](../classes/rendering_drawop.md#指令負載結構) | 實心填色矩形指令負載。 |
| [`DrawRoundedRectOp`](../classes/rendering_drawop.md#指令負載結構) | 圓角矩形指令負載。 |
| [`DrawLineOp`](../classes/rendering_drawop.md#指令負載結構) | 線段繪製指令負載。 |
| [`RenderStats`](../classes/rendering_displaylistrenderer.md#renderstats-結構) | 渲染效能統計資料結構（總操作數、已渲染數、已剔除數）。 |

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "src/rendering/backend/SoftwareRenderContext.hpp"

void ExampleRendering() {
    using namespace cppui::core;
    using namespace cppui::rendering;

    // 1. 初始化 DisplayList 與 DrawingContext
    DisplayList displayList;
    DrawingContext dc(displayList);

    // 2. 錄製繪圖指令
    dc.DrawRectangle(Rect(10, 10, 200, 100), Colors::SteelBlue);
    {
        // 使用 RAII 自動彈出局部剪裁區域
        DrawingContext::AutoClip clip(dc, Rect(20, 20, 150, 80));
        dc.DrawRoundedRectangle(Rect(20, 20, 100, 50), 8.0f, Colors::White);
    }

    // 3. 準備渲染目標與重播器
    SoftwareRenderTarget renderTarget;
    renderTarget.Resize(800, 600);
    renderTarget.Clear(Colors::Black);

    DisplayListRenderer renderer;
    RenderStats stats = renderer.Render(displayList, &renderTarget);

    compat::println("渲染完成: 總計 {} 指令，實際繪製 {}，剪裁剔除 {}", 
        stats.totalOps, stats.renderedOps, stats.culledOps);
}
```

---

## 請參閱 (See Also)
* [CPPUI API 首頁](../README.md)
* [`DrawingContext` 類別參考](../classes/rendering_drawingcontext.md)
* [`DisplayList` 類別參考](../classes/rendering_displaylist.md)
* [`DisplayListRenderer` 類別參考](../classes/rendering_displaylistrenderer.md)
* [`LinearArena` 類別參考](../classes/rendering_lineararena.md)
* [DisplayList 二進位錄製深度解析](../guides/display_list_recording.md)
