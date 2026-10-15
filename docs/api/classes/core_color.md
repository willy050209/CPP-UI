# Color 結構 (cppui::core::Color)

表示 32 位元 RGBA 色彩值，並提供標準色彩常數命名空間 `Colors`。

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

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    COMPAT_CONSTEXPR_14 Color() noexcept = default;
    COMPAT_CONSTEXPR_14 Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept;

    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Color FromArgb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept;
};

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

## 簡介 (Description)

`Color` 結構封裝了未經預乘的 32 位元 RGBA 色彩（每個通道佔用 8 位元，取值區間為 $0 \sim 255$）。`sizeof(Color) == 4`，與多數圖形硬體頂點色彩與光柵化像素格式完全對齊。

---

## 欄位 (Fields)

| 欄位名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `r` | `uint8_t` | `0` | 紅色通道 (Red) 強度。 |
| `g` | `uint8_t` | `0` | 綠色通道 (Green) 強度。 |
| `b` | `uint8_t` | `0` | 藍色通道 (Blue) 強度。 |
| `a` | `uint8_t` | `255` | Alpha 不透明度通道（`255` 為完全不透明，`0` 為完全透明）。 |

---

## 靜態方法 (Static Methods)

### `FromArgb`
以 Alpha、Red、Green、Blue 順序建構色彩。
```cpp
COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Color FromArgb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept;
```
* **參數**：
  * `a`：Alpha 通道。
  * `r`：Red 通道。
  * `g`：Green 通道。
  * `b`：Blue 通道。
* **傳回值**：包含指定通道數值的 `Color` 實例。

---

## 預設色彩常數 (`Colors`)

所有常數皆使用 `COMPAT_INLINE_VAR constexpr` 宣告於標頭檔中，在 C++17+ 自動內聯，在 C++14 下自動降級為檔案級 `static`，具備卓越的跨編譯器相容性：

| 常數名稱 | 數值 (R, G, B, A) | 十六進位等價表示 |
| :--- | :--- | :--- |
| `Colors::Transparent` | `(0, 0, 0, 0)` | `#00000000` |
| `Colors::Black` | `(0, 0, 0, 255)` | `#FF000000` |
| `Colors::White` | `(255, 255, 255, 255)` | `#FFFFFFFF` |
| `Colors::Red` | `(255, 0, 0, 255)` | `#FFFF0000` |
| `Colors::Green` | `(0, 255, 0, 255)` | `#FF00FF00` |
| `Colors::Blue` | `(0, 0, 255, 255)` | `#FF0000FF` |
| `Colors::SteelBlue` | `(70, 130, 180, 255)` | `#FF4682B4` |

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"

void ExampleColor() {
    using namespace cppui::core;

    // 1. 使用預設調色盤常數
    Color background = Colors::SteelBlue;

    // 2. 自訂半透明色彩
    Color semiRed = Color(255, 0, 0, 128);

    // 3. 使用 FromArgb 建構
    Color custom = Color::FromArgb(200, 100, 150, 250);

    compat::println("色彩 RGBA: ({}, {}, {}, {})", 
        custom.r, custom.g, custom.b, custom.a);
}
```

---

## 請參閱 (See Also)
* [**Color 結構進階技術文件 (Graphics)**](graphics_color.md)
* [**Colors 預定義常數調色盤 (140+ 種標準色)**](graphics_colors.md)
* [`Rect` 結構參考](core_rect.md)
* [`DrawingContext` 參考](rendering_drawingcontext.md)
* [`cppui::core` 命名空間](../namespaces/cppui_core.md)

