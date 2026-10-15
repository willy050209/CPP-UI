# Thickness 結構 (cppui::core::Thickness)

描述二維元件周圍四個方向（左、上、右、下）的厚度邊距。

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

struct Thickness {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    COMPAT_CONSTEXPR_14 Thickness() noexcept = default;
    COMPAT_CONSTEXPR_14 Thickness(float uniform) noexcept;
    COMPAT_CONSTEXPR_14 Thickness(float horizontal, float vertical) noexcept;
    COMPAT_CONSTEXPR_14 Thickness(float l, float t, float r, float b) noexcept;
};

} // namespace core
} // namespace cppui
```

---

## 簡介 (Description)

`Thickness` 結構被設計用來表示 UI 排版模型中的邊界度量值，常見用途包含：
* **外距 (Margin)**：控制項外部保留的空白距離。
* **內距 (Padding)**：控制項內容與邊框之間的內部緩衝距離。
* **邊框 (Border)**：繪製矩形或圓角矩形時四邊的邊框線寬度。

---

## 欄位 (Fields)

| 欄位名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `left` | `float` | `0.0f` | 左側邊距厚度。 |
| `top` | `float` | `0.0f` | 上方邊距厚度。 |
| `right` | `float` | `0.0f` | 右側邊距厚度。 |
| `bottom` | `float` | `0.0f` | 下方邊距厚度。 |

---

## 建構函式 (Constructors)

| 建構函式 | 說明 |
| :--- | :--- |
| `Thickness() noexcept` | 預設建構函式，四邊厚度皆為 `0.0f`。 |
| `Thickness(float uniform) noexcept` | 將四邊（左、上、右、下）設定為相同的單一數值。 |
| `Thickness(float horizontal, float vertical) noexcept` | 水平方向（左、右）設為 `horizontal`，垂直方向（上、下）設為 `vertical`。 |
| `Thickness(float l, float t, float r, float b) noexcept` | 分別指定左 (`l`)、上 (`t`)、右 (`r`)、下 (`b`) 四個獨立邊距數值。 |

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

void ExampleThickness() {
    using namespace cppui::core;

    // 1. 等距邊距 (全部 16.0f)
    Thickness uniformMargin(16.0f);

    // 2. 對稱邊距 (水平 20.0f, 垂直 10.0f)
    Thickness symmetricPadding(20.0f, 10.0f);

    // 3. 各邊獨立邊距
    Thickness customBorder(2.0f, 1.0f, 2.0f, 4.0f);

    compat::println("Padding: L={}, T={}, R={}, B={}",
        symmetricPadding.left, symmetricPadding.top, 
        symmetricPadding.right, symmetricPadding.bottom);
}
```

---

## 請參閱 (See Also)
* [`Rect` 結構參考](core_rect.md)
* [`Size` 結構參考](core_size.md)
* [`cppui::core` 命名空間](../namespaces/cppui_core.md)
