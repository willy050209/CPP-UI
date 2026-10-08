# Pen 類別 (cppui::graphics::Pen)

`Pen` 類別代表幾何圖形輪廓描邊的畫筆。它封裝了線條的色彩、寬度、虛線樣式（[`DashStyle`](graphics_enums.md#1-線條虛線樣式-dashstyle)）、端點封口形狀（[`LineCap`](graphics_enums.md#2-線段端點樣式-linecap)）與線段轉角連接樣式（[`LineJoin`](graphics_enums.md#3-線段轉角連接樣式-linejoin)）。

類別內部封裝讀寫鎖（`mutable std::shared_mutex`），支援多執行緒並發讀取與安全獨佔修改。

---

## 1. 元資料與規格 (Metadata)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/Pen.hpp"`
* **實作檔**：`src/graphics/Pen.cpp`

---

## 2. 類別語法與宣告 (Class Syntax)

```cpp
namespace cppui {
namespace graphics {

class Brush;

class Pen {
public:
    // 建構子
    explicit Pen(core::Color color, float width = 1.0f);
    explicit Pen(const Brush& brush, float width = 1.0f);

    // 執行緒安全拷貝與移動語意
    Pen(const Pen& other);
    Pen(Pen&& other) noexcept;
    Pen& operator=(const Pen& other);
    Pen& operator=(Pen&& other) noexcept;
    ~Pen() = default;

    // 色彩與線寬存取
    COMPAT_NODISCARD core::Color GetColor() const;
    void SetColor(core::Color color);

    COMPAT_NODISCARD float GetWidth() const;
    void SetWidth(float width);

    // 線段拓撲樣式存取
    COMPAT_NODISCARD DashStyle GetDashStyle() const;
    void SetDashStyle(DashStyle style);

    COMPAT_NODISCARD LineCap GetLineCap() const;
    void SetLineCap(LineCap cap);

    COMPAT_NODISCARD LineJoin GetLineJoin() const;
    void SetLineJoin(LineJoin join);

    // 深拷貝副本
    COMPAT_NODISCARD Pen Clone() const;
};

} // namespace graphics
} // namespace cppui
```

---

## 3. 成員函數與例外規格 (API Reference)

### 3.1 建構子 (Constructors)

| 建構子 | 參數 | 說明 | 異常處理 |
| :--- | :--- | :--- | :--- |
| `Pen(color, width = 1.0f)` | `Color color, float width` | 以指定色彩與線寬像素值建構畫筆。 | 若 `width < 0.0f`，擲出 `std::invalid_argument`。 |
| `Pen(brush, width = 1.0f)` | `const Brush& brush, float width` | 從給定之畫刷提取色彩建構畫筆。 | 若 `width < 0.0f`，擲出 `std::invalid_argument`。 |

### 3.2 屬性存取方法 (Properties)

| 方法 | 參數 | 回傳值 | 說明 | 執行緒安全性 / 異常防護 |
| :--- | :--- | :--- | :--- | :--- |
| `GetColor()` | 無 | `core::Color` | 取得目前線條色彩。 | `std::shared_lock` (多讀安全)。 |
| `SetColor(color)` | `core::Color color` | `void` | 修改線條色彩。 | `std::unique_lock` (獨佔保護)。 |
| `GetWidth()` | 無 | `float` | 取得目前線條粗細（像素值）。 | `std::shared_lock` (多讀安全)。 |
| `SetWidth(width)` | `float width` | `void` | 修改線條粗細。必須 $\ge 0.0f$。 | 若 `width < 0.0f` 擲出 `std::invalid_argument`。 |
| `GetDashStyle()` | 無 | `DashStyle` | 取得目前虛線樣式。 | `std::shared_lock` (多讀安全)。 |
| `SetDashStyle(style)` | `DashStyle style` | `void` | 設定虛線樣式（如 `Solid`, `Dash`, `Dot` 等）。 | `std::unique_lock` (獨佔保護)。 |
| `GetLineCap()` | 無 | `LineCap` | 取得目前端點樣式。 | `std::shared_lock` (多讀安全)。 |
| `SetLineCap(cap)` | `LineCap cap` | `void` | 設定端點形狀（如 `Flat`, `Round`, `ArrowAnchor`）。 | `std::unique_lock` (獨佔保護)。 |
| `GetLineJoin()` | 無 | `LineJoin` | 取得轉角連接樣式。 | `std::shared_lock` (多讀安全)。 |
| `SetLineJoin(join)` | `LineJoin join` | `void` | 設定轉角連接型態（如 `Miter`, `Bevel`, `Round`）。 | `std::unique_lock` (獨佔保護)。 |
| `Clone()` | 無 | `Pen` | 深拷貝建立獨立畫筆物件。 | 讀取時受鎖保護。 |

---

## 4. 範例程式碼 (Examples)

### 4.1 建立幾何描邊畫筆

```cpp
#include "ui/graphics/Pen.hpp"
#include "ui/graphics/Colors.hpp"
#include "ui/graphics/GraphicsTypes.hpp"
#include "ui/graphics/Graphics.hpp"

void DrawTechnicalGrid(cppui::graphics::Graphics& g) {
    // 1. 建立工程用藍色破折虛線筆
    cppui::graphics::Pen gridPen(cppui::graphics::Colors::SteelBlue, 1.5f);
    gridPen.SetDashStyle(cppui::graphics::DashStyle::Dash);

    for (float x = 0.0f; x < 500.0f; x += 50.0f) {
        g.DrawLine(gridPen, x, 0.0f, x, 500.0f);
    }

    // 2. 建立紅色粗圓角外框筆
    cppui::graphics::Pen borderPen(cppui::graphics::Colors::Red, 4.0f);
    borderPen.SetLineJoin(cppui::graphics::LineJoin::Round);
    borderPen.SetLineCap(cppui::graphics::LineCap::Round);

    g.DrawRectangle(borderPen, core::Rect(50, 50, 400, 400));
}
```

---

## 5. 請參閱 (See Also)

* [`Pens` 常用標準畫筆集合](graphics_pens.md)
* [`Color` 色彩結構詳解](graphics_color.md)
* [`Brush` 畫刷家族詳解](graphics_brush.md)
* [`GraphicsTypes` 列舉型別字典](graphics_enums.md)
* [`Graphics` 繪圖中樞排程器](graphics_graphics.md)
