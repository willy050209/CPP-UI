# Color 結構 (cppui::core::Color / cppui::graphics::Colors::Color)

`Color` 結構表示 32 位元 RGBA 色彩值。它作為 CPPUI 全系統（包含核心排版、控制項樣式、向量繪圖與點陣影像）的統一基礎色彩資料結構，具備 `constexpr` 編譯期求值支援、4 位元組記憶體緊湊對齊與高效的色彩模型轉換運算（ARGB、HSL）。

---

## 1. 元資料與規格 (Metadata & Specification)

* **命名空間**：[`cppui::core`](../namespaces/cppui_core.md)（亦於 `cppui::graphics::Colors` 中引出）
* **標頭檔**：`#include "ui/core/Geometry.hpp"` 或 `#include "ui/graphics/Colors.hpp"`
* **記憶體大小**：`sizeof(Color) == 4` 位元組（與圖形硬體頂點色彩及 32-bit 像素格式 100% 對齊）。
* **記憶體佈局**：由 4 個 `uint8_t` 成員依序組成（未預乘 Alpha 格式）：
  ```
  +---------------+---------------+---------------+---------------+
  |  r (8-bit)    |  g (8-bit)    |  b (8-bit)    |  a (8-bit)    |
  +---------------+---------------+---------------+---------------+
  0               8               16              24              31 bit
  ```

---

## 2. 結構宣告與欄位 (Structure Definition & Fields)

```cpp
namespace cppui {
namespace core {

struct Color {
    uint8_t r = 0;   // 紅色通道 (Red): 0 ~ 255
    uint8_t g = 0;   // 綠色通道 (Green): 0 ~ 255
    uint8_t b = 0;   // 藍色通道 (Blue): 0 ~ 255
    uint8_t a = 255; // 不透明度通道 (Alpha): 0 (完全透明) ~ 255 (完全不透明)

    // 建構子
    COMPAT_CONSTEXPR_14 Color() noexcept = default;
    COMPAT_CONSTEXPR_14 Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept;

    // 靜態工廠方法
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Color FromArgb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Color FromArgb(uint8_t r, uint8_t g, uint8_t b) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Color FromArgb(uint32_t argb) noexcept;
    COMPAT_NODISCARD static COMPAT_CONSTEXPR_14 Color FromArgb(uint8_t a, const Color& baseColor) noexcept;

    // 成員方法與色彩運算
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 uint32_t ToArgb() const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept;
    COMPAT_NODISCARD float GetBrightness() const noexcept;
    COMPAT_NODISCARD float GetHue() const noexcept;
    COMPAT_NODISCARD float GetSaturation() const noexcept;

    // 比較運算子
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const Color& other) const noexcept;
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const Color& other) const noexcept;
};

} // namespace core
} // namespace cppui
```

---

## 3. 成員函數與靜態工廠詳細說明 (API Reference)

### 3.1 建構子 (Constructors)

| 建構子 | 參數 | 說明 |
| :--- | :--- | :--- |
| `Color()` | 無 | 預設建構黑色不透明色彩：`r = 0, g = 0, b = 0, a = 255`。 |
| `Color(r, g, b, a = 255)` | `uint8_t red, green, blue, alpha` | 以指定之 RGBA 8 位元分量進行建構。預設 `alpha = 255`（不透明）。 |

### 3.2 靜態工廠方法 (`FromArgb`)

| 靜態方法 | 參數 | 回傳值 | 說明 |
| :--- | :--- | :--- | :--- |
| `FromArgb(a, r, g, b)` | `uint8_t a, r, g, b` | `Color` | 以 Alpha、Red、Green、Blue 順序建構（對標 GDI+ 經典簽名）。 |
| `FromArgb(r, g, b)` | `uint8_t r, g, b` | `Color` | 以完全不透明（Alpha=255）建構指定 RGB 色彩。 |
| `FromArgb(argb)` | `uint32_t argb` | `Color` | 解析 32 位元整數（格式為 `0xAARRGGBB`），提取各通道位元組建構。 |
| `FromArgb(a, baseColor)` | `uint8_t a, const Color& baseColor` | `Color` | 保留來源色彩之 RGB 色值，僅將不透明度替換為新的 `a` 值。常用於半透明遮罩與陰影計算。 |

### 3.3 成員方法與色彩轉換

#### `ToArgb`
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 uint32_t ToArgb() const noexcept;
```
* **功能**：將色彩打包為標準 32 位元整數（格式為 `0xAARRGGBB`，高位元組為 Alpha，依序為 Red, Green, Blue）。
* **計算公式**：`((a << 24) | (r << 16) | (g << 8) | b)`。

#### `IsEmpty`
```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsEmpty() const noexcept;
```
* **功能**：判斷色彩是否為全 0 的未初始化空值（`r == 0 && g == 0 && b == 0 && a == 0`）。

#### `GetBrightness` (HSL 明度)
```cpp
COMPAT_NODISCARD float GetBrightness() const noexcept;
```
* **功能**：計算色彩在 HSL 色彩空間中的明度（Lightness/Brightness）。
* **數值區間**：$0.0f \sim 1.0f$。
* **計算公式**：$\text{Brightness} = \frac{\min(R, G, B) + \max(R, G, B)}{2 \times 255.0f}$。

#### `GetHue` (HSL 色相)
```cpp
COMPAT_NODISCARD float GetHue() const noexcept;
```
* **功能**：計算色彩在色環上的色相角度（Hue）。
* **數值區間**：$0.0f \sim 360.0f$（單位：度）。若為黑白灰等無彩度顏色，傳回 $0.0f$。

#### `GetSaturation` (HSL 飽和度)
```cpp
COMPAT_NODISCARD float GetSaturation() const noexcept;
```
* **功能**：計算色彩在 HSL 色彩空間中的色彩飽和度（Saturation）。
* **數值區間**：$0.0f \sim 1.0f$。若色相無差異或極端明暗傳回 $0.0f$。

### 3.4 比較運算子

```cpp
bool operator==(const Color& other) const noexcept;
bool operator!=(const Color& other) const noexcept;
```
* 僅當兩個物件之 `r`, `g`, `b`, `a` 四個分量完全相同時判定為相等（`==`）。

---

## 4. 程式碼範例 (Code Examples)

### 4.1 建構色彩與 Alpha 半透明處理

```cpp
#include "ui/core/Geometry.hpp"
#include "compat/compat.hpp"

void ColorBasicsExample() {
    using cppui::core::Color;

    // 1. 基本 RGB 建構 (不透明)
    Color blue = Color(0, 120, 215);

    // 2. 從十六進位 Hex 整數建構 (0xAARRGGBB)
    Color coral = Color::FromArgb(0xFFFF7F50);

    // 3. 快速建立 50% 半透明版本 (Alpha = 128)
    Color semiCoral = Color::FromArgb(128, coral);

    compat::println("Coral RGBA: ({}, {}, {}, {}) -> Hex: 0x{:08X}",
        coral.r, coral.g, coral.b, coral.a, coral.ToArgb());
    compat::println("SemiCoral Alpha: {}", semiCoral.a);
}
```

### 4.2 HSL 色彩分析與對比度判斷

```cpp
#include "ui/core/Geometry.hpp"
#include "compat/compat.hpp"

// 依據背景色彩明度自動決定前景色應為黑色或白色 (提升易讀性)
cppui::core::Color GetContrastingTextColor(cppui::core::Color background) {
    float brightness = background.GetBrightness(); // 0.0 ~ 1.0
    if (brightness > 0.5f) {
        return cppui::core::Color(0, 0, 0);       // 背景偏亮 -> 黑色文字
    } else {
        return cppui::core::Color(255, 255, 255); // 背景偏暗 -> 白色文字
    }
}
```

### 4.3 編譯期常數計算 (Constexpr)

```cpp
#include "ui/core/Geometry.hpp"

// C++14+ 編譯期常數評估
constexpr cppui::core::Color BrandColor = cppui::core::Color::FromArgb(255, 41, 128, 185);
constexpr uint32_t HexValue = BrandColor.ToArgb();
static_assert(HexValue == 0xFF2980B9, "Color packaging must match compile-time value!");
```

---

## 5. 請參閱 (See Also)

* [`Colors` 預定義調色盤常數集合](graphics_colors.md)
* [`Brush` 畫刷家族詳解](graphics_brush.md)
* [`Bitmap` 點陣影像詳解](graphics_bitmap.md)
* [`Graphics` 繪圖中樞排程器](graphics_graphics.md)
* [`cppui::core` 命名空間](../namespaces/cppui_core.md)
