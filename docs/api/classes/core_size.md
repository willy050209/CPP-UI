# Size 結構 (cppui::core::Size)

表示二維空間中的幾何尺寸 $(width, height)$。

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

struct Size {
    float width = 0.0f;
    float height = 0.0f;

    COMPAT_CONSTEXPR_14 Size() noexcept = default;
    COMPAT_CONSTEXPR_14 Size(float w, float h) noexcept : width(w), height(h) {}

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept;

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Size& other) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Size& other) const noexcept;
};

} // namespace core
} // namespace cppui
```

---

## 簡介 (Description)

`Size` 結構描述控制項、視窗客戶區或圖元的二維寬度與高度。它是 UI 佈局系統（測量 `Measure` 與排列 `Arrange`）中最核心的資料傳遞結構。

---

## 欄位 (Fields)

| 欄位名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `width` | `float` | `0.0f` | 二維空間中的水平尺寸寬度。 |
| `height` | `float` | `0.0f` | 二維空間中的垂直尺寸高度。 |

---

## 方法 (Methods)

### `IsEmpty()`
檢查尺寸是否為空（寬度或高度小於或等於 0）。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept;
```
* **傳回值**：若 `width <= 0.0f` 或 `height <= 0.0f` 則傳回 `true`；否則傳回 `false`。
* **備註**：當尺寸為空時，該元件在渲染階段將會被圖元剔除演算法（Culling）跳過，避免無效的光柵化。

---

## 運算子 (Operators)

### `operator==`
判斷兩個 `Size` 是否相等。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Size& other) const noexcept;
```

### `operator!=`
判斷兩個 `Size` 是否不相等。
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Size& other) const noexcept;
```

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"

void ExampleSize() {
    using namespace cppui::core;

    Size availableSize(800.0f, 600.0f);
    Size zeroSize(0.0f, 100.0f);

    if (zeroSize.IsEmpty()) {
        compat::println("zeroSize 不具備有效可視區域。");
    }

    compat::println("可用區域尺寸: {} x {}", availableSize.width, availableSize.height);
}
```

---

## 請參閱 (See Also)
* [`Point` 結構參考](core_point.md)
* [`Rect` 結構參考](core_rect.md)
* [`cppui::core` 命名空間](../namespaces/cppui_core.md)
