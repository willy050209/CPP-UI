# Rect 結構 (cppui::core::Rect)

表示由原點 $(x, y)$ 與尺寸 $(width, height)$ 定義的二維軸對齊矩形區域。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::core`](../namespaces/cppui_core.md)
* **標頭檔**：`#include <ui/core/Geometry.hpp>`
* **模組**：`CPPUI Core`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace core {

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    COMPAT_CONSTEXPR_14 Rect() noexcept = default;
    COMPAT_CONSTEXPR_14 Rect(float px, float py, float w, float h) noexcept;
    COMPAT_CONSTEXPR_14 Rect(const Point& origin, const Size& size) noexcept;

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Left() const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Top() const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Right() const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Bottom() const noexcept;

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Point Origin() const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Size GetSize() const noexcept;

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Rect Intersect(const Rect& other) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool Contains(const Point& p) const noexcept;

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Rect& other) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Rect& other) const noexcept;
};

} // namespace core
} // namespace cppui
```

---

## 簡介 (Description)

`Rect` 是 CPPUI 幾何系統中用途最廣的結構，廣泛應用於：
1. **控制項邊界 (Bounds)**：表示元件在畫布上的空間佈局位置與尺寸。
2. **渲染剪裁 (Clipping)**：表示限制繪圖範圍的 Clip 區域。
3. **命中測試 (Hit-Testing)**：判斷滑鼠指標或觸控事件是否落於元件內部。
4. **可視區域剔除 (Occlusion Culling)**：透過計算矩形交集，快速剔除視窗外的不可見圖元。

---

## 欄位 (Fields)

| 欄位名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `x` | `float` | `0.0f` | 矩形左上角原點的水平 X 座標。 |
| `y` | `float` | `0.0f` | 矩形左上角原點的垂直 Y 座標。 |
| `width` | `float` | `0.0f` | 矩形的水平寬度。 |
| `height` | `float` | `0.0f` | 矩形的垂直高度。 |

---

## 方法 (Methods)

### `Left()` / `Top()` / `Right()` / `Bottom()`
取得矩形的四邊邊界座標。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Left() const noexcept;   // 回傳 x
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Top() const noexcept;    // 回傳 y
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Right() const noexcept;  // 回傳 x + width
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float Bottom() const noexcept; // 回傳 y + height
```

### `Origin()` / `GetSize()`
取得矩形的原點座標或尺寸。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Point Origin() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Size GetSize() const noexcept;
```

### `IsEmpty()`
檢查矩形是否為空（寬度或高度小於或等於 0）。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept;
```

### `Intersect(const Rect& other)`
計算當前矩形與另一個矩形的交集矩形。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Rect Intersect(const Rect& other) const noexcept;
```
* **參數**：`other` - 另一個欲進行交集運算的矩形。
* **傳回值**：兩矩形重疊的交集矩形。若無重疊，則回傳全為 0 的空矩形 `Rect(0.0f, 0.0f, 0.0f, 0.0f)`。
* **備註**：此方法在 C++14 下為 `constexpr`，已被高度應用於 `DisplayListRenderer` 的 Clip Stack 累積剪裁運算中。

### `Contains(const Point& p)`
判斷指定座標點是否落在此矩形內部（包含邊界）。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool Contains(const Point& p) const noexcept;
```
* **參數**：`p` - 欲判斷的目標座標點。
* **傳回值**：若 `p.x >= x && p.x <= Right() && p.y >= y && p.y <= Bottom()` 則傳回 `true`；否則傳回 `false`。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

void ExampleRect() {
    using namespace cppui::core;

    Rect viewport(0.0f, 0.0f, 800.0f, 600.0f);
    Rect element(750.0f, 500.0f, 100.0f, 200.0f);

    // 計算視口與元件的交集
    Rect visiblePart = element.Intersect(viewport);
    if (!visiblePart.IsEmpty()) {
        compat::println("元件局部可見，交集大小: {} x {}", 
            visiblePart.width, visiblePart.height);
    } else {
        compat::println("元件完全位於視口外，可剔除繪製。");
    }

    // 滑鼠點擊命中測試
    Point click(780.0f, 520.0f);
    if (element.Contains(click)) {
        compat::println("點擊命中該元件！");
    }
}
```

---

## 請參閱 (See Also)
* [`Point` 結構參考](core_point.md)
* [`Size` 結構參考](core_size.md)
* [`cppui::core` 命名空間](../namespaces/cppui_core.md)
