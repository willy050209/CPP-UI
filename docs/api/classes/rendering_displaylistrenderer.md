# DisplayListRenderer 類別 (cppui::rendering::DisplayListRenderer)

命令重播光柵器，遍歷 [`DisplayList`](rendering_displaylist.md)，維護剪裁與透明度狀態棧，執行圖元幾何剔除並分發至畫布。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::rendering`](../namespaces/cppui_rendering.md)
* **標頭檔**：`#include <ui/rendering/DisplayListRenderer.hpp>`
* **模組**：`CPPUI Rendering`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace rendering {

struct RenderStats {
    size_t totalOps = 0;
    size_t renderedOps = 0;
    size_t culledOps = 0;
};

class DisplayListRenderer {
public:
    DisplayListRenderer() = default;
    ~DisplayListRenderer() = default;

    RenderStats Render(const DisplayList& displayList, IRenderTarget* target);
};

} // namespace rendering
} // namespace cppui
```

---

## 簡介 (Description)

`DisplayListRenderer` 是 DisplayList 架構中的執行引擎（類似 GPU 的 Direct Command Processor）。它從 `DisplayList` 讀取二進位串流，並即時在內部維護兩組狀態棧：
1. **Clip Stack（局部剪裁棧）**：利用 `core::Rect::Intersect` 計算當前最深層可見邊界，初始值為畫布總視口。
2. **Opacity Stack（透明度乘積棧）**：利用 `compat::clamp` 乘積計算當前不透明度，初始值為 `1.0f`。

### 智慧圖元剔除 (Smart Occlusion Culling)
在處理每個繪圖圖元時：
* 若圖元與 `currentClip` 交集為空 (`visibleRect.IsEmpty() == true`)，立即跳過，**不執行任何像素寫入**，並增加 `stats.culledOps`。
* 若當前透明度 `currentOpacity <= 0.0f`，立即剔除。

---

## 結構說明：`RenderStats`

每次重播呼叫傳回的統計資料，可用於即時效能監控與診斷：

| 欄位名稱 | 型別 | 說明 |
| :--- | :--- | :--- |
| `totalOps` | `size_t` | 本次呼叫處理的總操作數量。 |
| `renderedOps` | `size_t` | 實際通過剪裁測試並送入光柵化寫入的圖元數量。 |
| `culledOps` | `size_t` | 因位於視口外或完全被剪裁而剔除的操作數量。 |

---

## 方法說明 (Methods)

### `Render(displayList, target)`
重播並光柵化整份 DisplayList 指令串流。
```cpp
RenderStats Render(const DisplayList& displayList, IRenderTarget* target);
```
* **參數**：
  * `displayList`：欲重播的二進位繪圖命令流。
  * `target`：目標渲染畫布（實作 [`IRenderTarget`](rendering_irendercontext.md#irendertarget-介面) 的後端）。
* **傳回值**：包含統計資訊的 `RenderStats` 結構。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "src/rendering/backend/SoftwareRenderContext.hpp"

void RenderFrame(const cppui::rendering::DisplayList& dl, 
                 cppui::rendering::SoftwareRenderTarget& target) {
    using namespace cppui::rendering;

    DisplayListRenderer renderer;
    RenderStats stats = renderer.Render(dl, &target);

    compat::println("幀統計: 總數={}, 繪製={}, 剔除={}", 
        stats.totalOps, stats.renderedOps, stats.culledOps);
}
```

---

## 請參閱 (See Also)
* [`DisplayList` 命令串流](rendering_displaylist.md)
* [`DrawingContext` 錄製介面](rendering_drawingcontext.md)
* [`IRenderTarget` 渲染畫布](rendering_irendercontext.md)
