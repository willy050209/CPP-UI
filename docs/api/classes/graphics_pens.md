# Pens 標準畫筆集合 (cppui::graphics::Pens)

`Pens` 類別借鏡 GDI+ (`System.Drawing.Pens`)，為 CPPUI 提供 28 種常用色彩且線條寬度為 1.0f 像素之靜態唯讀畫筆集合（Stock Pens）。

---

## 1. 元資料與規格 (Metadata)

* **命名空間**：[`cppui::graphics`](../namespaces/cppui_graphics.md)
* **標頭檔**：`#include "ui/graphics/Pens.hpp"`
* **實作檔**：`src/graphics/Pens.cpp`
* **類別特性**：純靜態類別，不可實例化（`Pens() = delete`）。
* **畫筆規格**：
  - 寬度固定為 `1.0f` 像素。
  - 樣式固定為實線（`DashStyle::Solid`）、平整端點（`LineCap::Flat`）、斜接轉角（`LineJoin::Miter`）。
  - 回傳 `const Pen&` 常數引用，採用 Meyer's Singleton 延遲初始化，全生命週期零多餘配置且執行緒安全。

---

## 2. 類別語法與宣告 (Class Syntax)

```cpp
namespace cppui {
namespace graphics {

class Pens {
public:
    Pens() = delete;

    COMPAT_NODISCARD static const Pen& Transparent();
    COMPAT_NODISCARD static const Pen& Black();
    COMPAT_NODISCARD static const Pen& White();
    COMPAT_NODISCARD static const Pen& Red();
    COMPAT_NODISCARD static const Pen& Green();
    COMPAT_NODISCARD static const Pen& Blue();
    COMPAT_NODISCARD static const Pen& Yellow();
    COMPAT_NODISCARD static const Pen& Cyan();
    COMPAT_NODISCARD static const Pen& Magenta();
    COMPAT_NODISCARD static const Pen& Gray();
    COMPAT_NODISCARD static const Pen& LightGray();
    COMPAT_NODISCARD static const Pen& DarkGray();
    COMPAT_NODISCARD static const Pen& Orange();
    COMPAT_NODISCARD static const Pen& Purple();
    COMPAT_NODISCARD static const Pen& Brown();
    COMPAT_NODISCARD static const Pen& SteelBlue();
    COMPAT_NODISCARD static const Pen& Navy();
    COMPAT_NODISCARD static const Pen& Gold();
    COMPAT_NODISCARD static const Pen& Lime();
    COMPAT_NODISCARD static const Pen& Pink();
    COMPAT_NODISCARD static const Pen& DarkRed();
    COMPAT_NODISCARD static const Pen& DarkBlue();
    COMPAT_NODISCARD static const Pen& DarkGreen();
    COMPAT_NODISCARD static const Pen& Olive();
    COMPAT_NODISCARD static const Pen& Teal();
    COMPAT_NODISCARD static const Pen& Maroon();
    COMPAT_NODISCARD static const Pen& Coral();
    COMPAT_NODISCARD static const Pen& Silver();
};

} // namespace graphics
} // namespace cppui
```

---

## 3. 28 種標準畫筆清冊 (Stock Pens Catalog)

| 靜態方法 | 對應色彩常數 | 十六進位 ARGB | 典型描邊用途 |
| :--- | :--- | :--- | :--- |
| `Pens::Transparent()` | `Colors::Transparent` | `#00000000` | 隱藏輪廓線。 |
| `Pens::Black()` | `Colors::Black` | `#FF000000` | 標準黑色框線。 |
| `Pens::White()` | `Colors::White` | `#FFFFFFFF` | 深色背景上的反白框線。 |
| `Pens::Red()` | `Colors::Red` | `#FFFF0000` | 錯誤提示紅框。 |
| `Pens::Green()` | `Colors::Green` | `#FF008000` | 成功驗證框線。 |
| `Pens::Blue()` | `Colors::Blue` | `#FF0000FF` | 基礎藍線。 |
| `Pens::Yellow()` | `Colors::Yellow` | `#FFFFFF00` | 警戒框線。 |
| `Pens::Cyan()` | `Colors::Cyan` | `#FF00FFFF` | 青色輔助線。 |
| `Pens::Magenta()` | `Colors::Magenta` | `#FFFF00FF` | 洋紅色標線。 |
| `Pens::Gray()` | `Colors::Gray` | `#FF808080` | 次要邊界、分隔線。 |
| `Pens::LightGray()` | `Colors::LightGray` | `#FFD3D3D3` | 淺色控制項外框、表格格線。 |
| `Pens::DarkGray()` | `Colors::DarkGray` | `#FFA9A9A9` | 深色分隔線。 |
| `Pens::Orange()` | `Colors::Orange` | `#FFFFA500` | 橙色重點標線。 |
| `Pens::Purple()` | `Colors::Purple` | `#FF800080` | 紫色描邊。 |
| `Pens::Brown()` | `Colors::Brown` | `#FFA52A2A` | 棕色描邊。 |
| `Pens::SteelBlue()` | `Colors::SteelBlue` | `#FF4682B4` | 焦點指示外框 (Focus Rectangle)。 |
| `Pens::Navy()` | `Colors::Navy` | `#FF000080` | 海軍藍描邊。 |
| `Pens::Gold()` | `Colors::Gold` | `#FFFFD700` | 金色亮框。 |
| `Pens::Lime()` | `Colors::Lime` | `#FF00FF00` | 亮綠色指示線。 |
| `Pens::Pink()` | `Colors::Pink` | `#FFFFC0CB` | 粉紅色描邊。 |
| `Pens::DarkRed()` | `Colors::DarkRed` | `#FF8B0000` | 深紅外框。 |
| `Pens::DarkBlue()` | `Colors::DarkBlue` | `#FF00008B` | 深藍外框。 |
| `Pens::DarkGreen()` | `Colors::DarkGreen` | `#FF006400` | 深綠外框。 |
| `Pens::Olive()` | `Colors::Olive` | `#FF808000` | 橄欖綠外框。 |
| `Pens::Teal()` | `Colors::Teal` | `#FF008080` | 鴨翅綠線條。 |
| `Pens::Maroon()` | `Colors::Maroon` | `#FF800000` | 栗色外框。 |
| `Pens::Coral()` | `Colors::Coral` | `#FFFF7F50` | 珊瑚色標線。 |
| `Pens::Silver()` | `Colors::Silver` | `#FFC0C0C0` | 銀灰色邊界。 |

---

## 4. 範例程式碼 (Examples)

```cpp
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/Pens.hpp"
#include "ui/graphics/Brushes.hpp"

void DrawCardOutline(cppui::graphics::Graphics& g) {
    core::Rect rect(30.0f, 30.0f, 240.0f, 120.0f);

    // 填滿背景並繪製標準細邊框 (零重複記憶體開銷)
    g.FillRoundedRectangle(cppui::graphics::Brushes::White(), rect, 6.0f);
    g.DrawRoundedRectangle(cppui::graphics::Pens::LightGray(), rect, 6.0f);
}
```

---

## 5. 請參閱 (See Also)

* [`Pen` 畫筆類別詳解](graphics_pen.md)
* [`Brushes` 常用標準純色畫刷集合](graphics_brushes.md)
* [`Colors` 預定義常數調色盤](graphics_colors.md)
* [`Graphics` 繪圖中樞排程器](graphics_graphics.md)
