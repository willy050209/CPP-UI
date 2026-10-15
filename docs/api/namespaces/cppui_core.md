# cppui::core 命名空間 (Namespace)

`cppui::core` 命名空間封裝了 CPPUI 框架的核心幾何資料結構與色彩數值型別。

---

## 簡介 (Description)

`cppui::core` 所屬的型別為純量數值結構（POD / Value Types），被設計用於頻繁的跨函式參數傳遞、計算、版面測量 (Measure/Arrange) 以及繪圖圖元描述。

### 核心設計原則
1. **數值語意 (Value Semantics)**：所有型別均支援輕量拷貝與移動，無任何動態堆積配置 (No Dynamic Heap Allocation)。
2. **編譯期求值 (`constexpr`)**：建構式、算術運算子與查詢方法全面標註 `COMPAT_CONSTEXPR_14`，支援在 C++14、C++17、C++20 下進行編譯期常數計算。
3. **安全檢查 (`COMPAT_NODISCARD`)**：純查詢與幾何計算方法標註 `COMPAT_NODISCARD`，防止呼叫者意外忽略回傳值（如忽略 `Rect::Intersect` 的結果）。

---

| 結構 / 類別 (Type) | 簡介與職責 |
| :--- | :--- |
| [`Visual`](../classes/core_visual.md) | 視覺樹抽象基底，維護親緣關係、位移、邊界、命中測試與繪製指令派發。 |
| [`UIElement`](../classes/core_uielement.md) | 具備 Margin、Alignment、Min/Max 幾何約束，`Measure` / `Arrange` 雙階段排版管線與髒標記冒泡追蹤。 |
| [`Point`](../classes/core_point.md) | 表示二維笛卡爾座標系上的浮點數點 $(x, y)$。 |
| [`Size`](../classes/core_size.md) | 表示二維空間中的寬度與高度 $(width, height)$。 |
| [`Rect`](../classes/core_rect.md) | 表示由原點與尺寸所定義的二維軸對齊矩形區域，支援幾何求交集與點包含測試。 |
| [`Color`](../classes/core_color.md) | 表示 32 位元 RGBA 色彩值，提供數值打包與標準色彩常數。 |
| [`Thickness`](../classes/core_thickness.md) | 表示四個方向（左、上、右、下）的厚度邊距，用於邊框、外距 (Margin) 與內距 (Padding)。 |
| [`CoordinateTransform`](../classes/core_coordinatetransform.md) | 坐標轉換管理類別，提供物理像素與設備無關邏輯像素（DIP）間的雙向型別安全轉換。 |

---

## 常數集合 (Constants)

### `cppui::core::Colors`
定義於標頭檔 `<ui/core/Geometry.hpp>` 中的標準色彩預設常數，使用 `COMPAT_INLINE_VAR constexpr` 宣告以確保向下相容 C++14 與 C++17+：

```cpp
namespace cppui {
namespace core {
namespace Colors {
    COMPAT_INLINE_VAR constexpr Color Transparent = Color(0, 0, 0, 0);
    COMPAT_INLINE_VAR constexpr Color Black       = Color(0, 0, 0, 255);
    COMPAT_INLINE_VAR constexpr Color White       = Color(255, 255, 255, 255);
    COMPAT_INLINE_VAR constexpr Color Red         = Color(255, 0, 0, 255);
    COMPAT_INLINE_VAR constexpr Color Green       = Color(0, 255, 0, 255);
    COMPAT_INLINE_VAR constexpr Color Blue        = Color(0, 0, 255, 255);
    COMPAT_INLINE_VAR constexpr Color SteelBlue   = Color(70, 130, 180, 255);
} // namespace Colors
} // namespace core
} // namespace cppui
```

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

void ExampleCoreUsage() {
    using namespace cppui::core;

    // 建立矩形與點
    Rect buttonRect(100.0f, 100.0f, 200.0f, 50.0f);
    Point mousePos(150.0f, 120.0f);

    // 命中測試
    if (buttonRect.Contains(mousePos)) {
        compat::println("滑鼠位於按鈕內部！");
    }

    // 計算兩個區域的交集
    Rect clipRect(0.0f, 0.0f, 180.0f, 180.0f);
    Rect visibleRect = buttonRect.Intersect(clipRect);
    compat::println("可見交集區域寬度: {}", visibleRect.width);
}
```

---

## 請參閱 (See Also)
* [CPPUI API 首頁](../README.md)
* [`Point` 類別參考](../classes/core_point.md)
* [`Rect` 類別參考](../classes/core_rect.md)
* [`Color` 類別參考](../classes/core_color.md)
