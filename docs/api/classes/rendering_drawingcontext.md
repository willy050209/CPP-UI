# DrawingContext 類別 (cppui::rendering::DrawingContext)

公開畫布錄製介面，提供 UI 控制項產生純量繪製命令，將操作序列化寫入 [`DisplayList`](rendering_displaylist.md)。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::rendering`](../namespaces/cppui_rendering.md)
* **標頭檔**：`#include <ui/rendering/DrawingContext.hpp>`
* **模組**：`CPPUI Rendering`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace rendering {

class DrawingContext {
public:
    explicit DrawingContext(DisplayList& displayList) noexcept;
    ~DrawingContext() = default;

    // 圖元繪製
    void DrawRectangle(const core::Rect& rect, core::Color color);
    void DrawRoundedRectangle(const core::Rect& rect, float radius, core::Color color);
    void DrawLine(core::Point p1, core::Point p2, core::Color color, float strokeWidth = 1.0f);

    // 剪裁棧操作
    void PushClip(const core::Rect& rect);
    void PopClip();

    // 透明度棧操作
    void PushOpacity(float opacity);
    void PopOpacity();

    // RAII 守衛
    class AutoClip;
    class AutoOpacity;
};

} // namespace rendering
} // namespace cppui
```

---

## 簡介 (Description)

`DrawingContext` 是 CPPUI 渲染模型中最關鍵的隔離邊界。它類似 HTML5 Canvas 或 WPF 的 `DrawingContext`，但**不直接進行任何光柵化或呼叫 GPU**。

當呼叫 `DrawRectangle` 等方法時，`DrawingContext` 會將二進位指令協議打包成 [`DrawRectOp`](rendering_drawop.md) 並直接壓入底層 `DisplayList` 的平坦記憶體池。這使得 UI 控制項的渲染執行時間達到極致的微秒 ($O(1)$) 級別。

---

## 圖元繪製方法 (Primitive Drawing)

### `DrawRectangle`
繪製填色實心矩形。
```cpp
void DrawRectangle(const core::Rect& rect, core::Color color);
```
* **參數**：
  * `rect`：矩形的位置與大小。若為空 (`rect.IsEmpty()`) 則忽略。
  * `color`：填充顏色。若為完全透明 (`color.a == 0`) 則忽略。

### `DrawRoundedRectangle`
繪製填色圓角矩形。
```cpp
void DrawRoundedRectangle(const core::Rect& rect, float radius, core::Color color);
```
* **參數**：
  * `rect`：外圍邊界矩形。
  * `radius`：圓角半徑 (像素)。若 `radius <= 0.5f` 內部將自動優化為純矩形光柵化。
  * `color`：填充顏色。

### `DrawLine`
繪製指定線寬的實心線段。
```cpp
void DrawLine(core::Point p1, core::Point p2, core::Color color, float strokeWidth = 1.0f);
```
* **參數**：
  * `p1`, `p2`：線段端點座標。
  * `color`：線條顏色。
  * `strokeWidth`：線寬（像素，預設 `1.0f`）。

---

## 狀態機操作與 RAII 守衛 (State Management)

### `PushClip(rect)` / `PopClip()`
限制後續所有繪製命令在指定矩形 `rect` 內部可見。支援巢狀呼叫。每次 `PushClip` 必須嚴格對應一次 `PopClip`。

### `PushOpacity(opacity)` / `PopOpacity()`
將後續繪製圖元的透明度乘以 `opacity` (範圍 $0.0f \sim 1.0f$)。支援層層疊加。

### RAII 自動彈出守衛：`AutoClip` 與 `AutoOpacity`
為避免手動忘記呼叫 `PopClip` 或在異常/早期 `return` 時破壞狀態棧，強烈推薦使用 RAII 守衛：

```cpp
{
    DrawingContext::AutoClip clip(dc, Rect(0, 0, 100, 100));
    DrawingContext::AutoOpacity opacity(dc, 0.5f);

    dc.DrawRectangle(Rect(10, 10, 50, 50), Colors::Red);
    // 作用域結束時，自動安全彈出 Opacity 與 Clip！
}
```

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"

void RecordCustomCard(cppui::rendering::DisplayList& dl) {
    using namespace cppui::core;
    using namespace cppui::rendering;

    DrawingContext dc(dl);

    // 1. 繪製卡片底色 (圓角矩形)
    dc.DrawRoundedRectangle(Rect(50, 50, 300, 200), 12.0f, Colors::SteelBlue);

    // 2. 限制在卡片內容區內繪製
    {
        DrawingContext::AutoClip clip(dc, Rect(60, 60, 280, 180));

        // 半透明白色內層裝飾
        DrawingContext::AutoOpacity opacity(dc, 0.8f);
        dc.DrawRectangle(Rect(70, 70, 260, 40), Colors::White);

        // 分隔線
        dc.DrawLine(Point(70, 120), Point(330, 120), Colors::Black, 2.0f);
    }
}
```

---

## 請參閱 (See Also)
* [`DisplayList` 命令串流容器](rendering_displaylist.md)
* [`DisplayListRenderer` 重播器](rendering_displaylistrenderer.md)
* [DisplayList 二進位錄製深度解析](../guides/display_list_recording.md)
