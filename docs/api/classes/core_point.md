# Point 結構 (cppui::core::Point)

表示二維笛卡爾座標系中的點 $(x, y)$。

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

struct Point {
    float x = 0.0f;
    float y = 0.0f;

    COMPAT_CONSTEXPR_14 Point() noexcept = default;
    COMPAT_CONSTEXPR_14 Point(float px, float py) noexcept : x(px), y(py) {}

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Point& other) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Point& other) const noexcept;
};

} // namespace core
} // namespace cppui
```

---

## 簡介 (Description)

`Point` 是一個緊湊的二維浮點數數值結構（佔用 8 個位元組），用於表示螢幕視窗座標、滑鼠指標位置、圖元頂點或排版位移向量。其所有建構函式與比較運算子皆標記 `COMPAT_CONSTEXPR_14` 與 `noexcept`，可直接在編譯期作為常數求值。

---

## 欄位 (Fields)

| 欄位名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `x` | `float` | `0.0f` | 二維空間中的水平 X 軸座標（由左向右遞增）。 |
| `y` | `float` | `0.0f` | 二維空間中的垂直 Y 軸座標（由上向下遞增）。 |

---

## 建構函式 (Constructors)

| 建構函式 | 說明 |
| :--- | :--- |
| `COMPAT_CONSTEXPR_14 Point() noexcept` | 預設建構函式，將座標初始化為原點 $(0.0f, 0.0f)$。 |
| `COMPAT_CONSTEXPR_14 Point(float px, float py) noexcept` | 使用指定的 X 與 Y 浮點數值初始化座標點。 |

---

## 運算子 (Operators)

### `operator==`
判斷兩個 `Point` 是否完全相等。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Point& other) const noexcept;
```
* **參數**：`other` - 欲比較的目標座標點。
* **傳回值**：若 `x == other.x` 且 `y == other.y` 則傳回 `true`；否則傳回 `false`。

### `operator!=`
判斷兩個 `Point` 是否不相等。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Point& other) const noexcept;
```

---

## 備註 (Remarks)

* **記憶體佈局**：`sizeof(Point) == 8`，連續 2 個 32-bit 單精度浮點數。保證 4 字節對齊，適合直接作為頂點陣列傳遞給 GPU 頂點著色器。
* **線程安全性**：作為不可變 POD 數值型別，在多執行緒環境下進行唯讀並行存取完全執行緒安全。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

void ExamplePoint() {
    using namespace cppui::core;

    // 編譯期常數求值 (C++14+)
    COMPAT_CONSTEXPR_14 Point ptA(10.5f, 20.0f);
    COMPAT_CONSTEXPR_14 Point ptB(10.5f, 20.0f);

    static_assert(ptA == ptB, "Points should be equal at compile time");

    compat::println("Point A: ({}, {})", ptA.x, ptA.y);
}
```

---

## 請參閱 (See Also)
* [`Size` 結構參考](core_size.md)
* [`Rect` 結構參考](core_rect.md)
* [`cppui::core` 命名空間](../namespaces/cppui_core.md)
