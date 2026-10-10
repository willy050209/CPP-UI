# PixelOps 類別 (cppui::graphics::PixelOps)

`PixelOps` 類別定義於 `include/ui/graphics/PixelOps.hpp`，提供零抽象開銷之高效像素遍歷演算法。它支援直接像素數值、二維空間坐標與 `Color` 多型呼叫簽名，並全面受控於 [`ParallelOptions`](graphics_paralleloptions.md) 進行單執行緒或動態視窗分塊平行化排程。

---

## 1. 元資料與規格 (Metadata & Specification)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/PixelOps.hpp"`
* **特性**：
  - 純靜態方法與模板內嵌最佳化，完全消除虛擬函式調用開銷。
  - 支援可變點陣圖與唯讀點陣圖雙軌多載。
  - 編譯期 Concepts / SFINAE 自動推導多種 Lambda 參數形式。
  - 與 [`ParallelOptions`](graphics_paralleloptions.md) 深度整合，支援橫向列切割與 Window Slicing。

---

## 2. 語法與宣告 (Syntax & Declaration)

```cpp
namespace cppui {
namespace graphics {

class PixelOps {
public:
    // 全圖像素遍歷
    template <typename Func>
    static void ForEach(Bitmap& bmp, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential());

    template <typename Func>
    static void ForEach(const Bitmap& bmp, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential());

    // 條件式像素遍歷
    template <typename Pred, typename Func>
    static void ForEachIf(Bitmap& bmp, Pred&& pred, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential());

    template <typename Pred, typename Func>
    static void ForEachIf(const Bitmap& bmp, Pred&& pred, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential());
};

} // namespace graphics
} // namespace cppui
```

---

## 3. 支援之函式簽名 (Supported Callback Signatures)

`PixelOps::ForEach` 與 `PixelOps::ForEachIf` 內部採用編譯期靜態分發，使用者傳入之 Callable（函式指標、Functor 或 Lambda）可任意採用下列任一簽名形式：

| 簽名形式 | 適用場景 | 範例 Lambda 寫法 |
| :--- | :--- | :--- |
| **純像素引用** | 快速原地反相、遮罩運算，無需關注空間坐標。 | `[](uint32_t& pixel) { pixel ^= 0x00FFFFFF; }` |
| **坐標與像素** | 依據空間位置產生圖樣、漸層或進行坐標映射。 | `[](uint32_t x, uint32_t y, uint32_t& pixel) { ... }` |
| **色彩物件引用** | 語意化直觀調整色彩通道（自動封裝與解包 ARGB）。 | `[](uint32_t x, uint32_t y, core::Color& c) { c.r = 255; }` |
| **唯讀像素讀取** | 統計直方圖、分析最大值/最小值。 | `[](uint32_t x, uint32_t y, const uint32_t& pixel) { ... }` |

---

## 4. API 說明 (API Reference)

### 4.1 `ForEach`
```cpp
template <typename Func>
static void ForEach(Bitmap& bmp, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential());
```
* **功能**：高速遍歷整幅影像中的所有像素點並執行指定運算。
* **參數**：
  - `bmp`：目標點陣圖實例。
  - `func`：處理函數或 Lambda。
  - `options`：平行化配置（預設單執行緒循序）。
* **複雜度**：$O(W \times H)$，中央掃描以直接指針步進運算。

### 4.2 `ForEachIf`
```cpp
template <typename Pred, typename Func>
static void ForEachIf(Bitmap& bmp, Pred&& pred, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential());
```
* **功能**：條件式遍歷。針對每一個像素點先評估 `pred` 條件謂詞，若結果為 `true` 才執行 `func`。
* **參數**：
  - `bmp`：目標點陣圖實例。
  - `pred`：條件謂詞（支援 `pred(pixel)`、`pred(x, y, pixel)`、`pred(x, y, color)`）。
  - `func`：處理函數。
  - `options`：平行化配置。

---

## 5. 範例程式碼 (Usage Examples)

### 5.1 快速全彩反相 (單執行緒)
```cpp
#include "ui/graphics/PixelOps.hpp"

using namespace cppui::graphics;

Bitmap bmp(800, 600);
// 反相 RGB 通道，保留 Alpha 通道
PixelOps::ForEach(bmp, [](uint32_t& p) {
    p ^= 0x00FFFFFF;
});
```

### 5.2 多執行緒局部條件色度取代 (Chroma-Key 綠幕去背)
```cpp
// 搜尋綠色通道佔比過高之像素並替換為透明，以 4 執行緒執行
ParallelOptions opts = ParallelOptions::Parallel(4, 32);

PixelOps::ForEachIf(bmp, 
    // 條件謂詞: 判斷是否為綠幕像素
    [](uint32_t, uint32_t, const core::Color& c) {
        return c.g > 180 && c.r < 100 && c.b < 100;
    },
    // 符合條件時執行操作: 設定為全透明
    [](uint32_t, uint32_t, core::Color& c) {
        c.a = 0;
    }, 
    opts
);
```
