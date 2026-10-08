# Colors 預定義常數調色盤 (cppui::graphics::Colors)

`Colors` 命名空間提供對標 Windows GDI+ (`System.Drawing.Color`) 與 W3C CSS 擴充調色盤之完整預定義常數集合。所有色彩皆宣告為 `COMPAT_INLINE_VAR constexpr Color`，在編譯期完全求值，具有**零執行期開銷**與卓越的跨編譯器相容性。

---

## 1. 元資料與規格 (Metadata)

* **命名空間**：`cppui::graphics::Colors`（亦於 `cppui::core::Colors` 提供常用子集）
* **標頭檔**：`#include "ui/graphics/Colors.hpp"`
* **型別定義**：所有常數型別皆為 [`cppui::core::Color`](graphics_color.md)
* **宣告巨集**：`COMPAT_INLINE_VAR constexpr`
  - 在 C++17 及以上環境自動宣告為 `inline constexpr`，避免多重定義 (ODR Violation)。
  - 在 C++14 環境下自動降級為檔案內部靜態常數。

---

## 2. 語法與使用範式 (Syntax & Usage)

```cpp
#include "ui/graphics/Colors.hpp"

// 直接引用命名空間色彩常數
cppui::core::Color bgColor = cppui::graphics::Colors::AliceBlue;
cppui::core::Color accent  = cppui::graphics::Colors::SteelBlue;
```

---

## 3. 完整色彩常數分類清冊 (Categorized Palette Table)

涵蓋 140+ 種標準現代預定義色彩：

### 3.1 紅色與粉色系 (Reds & Pinks)

| 常數名稱 | (R, G, B, A) | 十六進位 ARGB | 預覽說明 |
| :--- | :--- | :--- | :--- |
| `Colors::Red` | `(255, 0, 0, 255)` | `#FFFF0000` | 純紅色 |
| `Colors::DarkRed` | `(139, 0, 0, 255)` | `#FF8B0000` | 深紅 |
| `Colors::Crimson` | `(220, 20, 60, 255)` | `#FFDC143C` | 緋紅 |
| `Colors::Firebrick` | `(178, 34, 34, 255)` | `#FFB22222` | 耐火磚紅 |
| `Colors::IndianRed` | `(205, 92, 92, 255)` | `#FFCD5C5C` | 印度紅 |
| `Colors::LightCoral` | `(240, 128, 128, 255)`| `#FFF08080` | 淺珊瑚色 |
| `Colors::Salmon` | `(250, 128, 114, 255)`| `#FFFA8072` | 鮭魚粉 |
| `Colors::DarkSalmon` | `(233, 150, 122, 255)`| `#FFE9967A` | 深鮭魚色 |
| `Colors::LightSalmon`| `(255, 160, 122, 255)`| `#FFFFA07A` | 淺鮭魚色 |
| `Colors::Pink` | `(255, 192, 203, 255)`| `#FFFFC0CB` | 粉紅 |
| `Colors::LightPink` | `(255, 182, 193, 255)`| `#FFFFB6C1` | 淺粉紅 |
| `Colors::HotPink` | `(255, 105, 180, 255)`| `#FFFF69B4` | 暖粉紅 |
| `Colors::DeepPink` | `(255, 20, 147, 255)` | `#FFFF1493` | 深粉紅 |
| `Colors::MediumVioletRed` | `(199, 21, 133, 255)`| `#FFC71585`| 中紫紅 |
| `Colors::PaleVioletRed` | `(219, 112, 147, 255)`| `#FFDB7093`| 蒼紫紅 |

### 3.2 橙色與黃色系 (Oranges & Yellows)

| 常數名稱 | (R, G, B, A) | 十六進位 ARGB | 預覽說明 |
| :--- | :--- | :--- | :--- |
| `Colors::Orange` | `(255, 165, 0, 255)` | `#FFFFA500` | 純橙色 |
| `Colors::DarkOrange` | `(255, 140, 0, 255)` | `#FFFF8C00` | 深橙色 |
| `Colors::OrangeRed` | `(255, 69, 0, 255)` | `#FFFF4500` | 橙紅色 |
| `Colors::Tomato` | `(255, 99, 71, 255)` | `#FFFF6347` | 番茄紅 |
| `Colors::Coral` | `(255, 127, 80, 255)` | `#FFFF7F50` | 珊瑚色 |
| `Colors::Yellow` | `(255, 255, 0, 255)` | `#FFFFFF00` | 純黃色 |
| `Colors::Gold` | `(255, 215, 0, 255)` | `#FFFFD700` | 金色 |
| `Colors::Goldenrod` | `(218, 165, 32, 255)` | `#FFDAA520` | 金麒麟黃 |
| `Colors::DarkGoldenrod` | `(184, 134, 11, 255)` | `#FFB8860B` | 暗金黃 |
| `Colors::LightGoldenrodYellow` | `(250, 250, 210, 255)` | `#FFFAFAD2` | 淺秋黃 |
| `Colors::Khaki` | `(240, 230, 140, 255)`| `#FFF0E68C` | 卡其色 |
| `Colors::DarkKhaki` | `(189, 183, 107, 255)`| `#FFBDB76B` | 暗卡其色 |
| `Colors::LightYellow`| `(255, 255, 224, 255)`| `#FFFFFFE0` | 淺黃 |
| `Colors::LemonChiffon` | `(255, 250, 205, 255)`| `#FFFFFACD` | 檸檬雪紡 |
| `Colors::PapayaWhip` | `(255, 239, 213, 255)`| `#FFFFEFD5` | 木瓜牛奶 |
| `Colors::Moccasin` | `(255, 228, 181, 255)`| `#FFFFE4B5` | 鹿皮黃 |
| `Colors::PeachPuff` | `(255, 218, 185, 255)`| `#FFFFDAB9` | 粉撲桃色 |

### 3.3 綠色系 (Greens)

| 常數名稱 | (R, G, B, A) | 十六進位 ARGB | 預覽說明 |
| :--- | :--- | :--- | :--- |
| `Colors::Green` | `(0, 128, 0, 255)` | `#FF008000` | 標準綠色 |
| `Colors::DarkGreen` | `(0, 100, 0, 255)` | `#FF006400` | 深綠色 |
| `Colors::Lime` | `(0, 255, 0, 255)` | `#FF00FF00` | 萊姆綠 (亮綠) |
| `Colors::LimeGreen` | `(50, 205, 50, 255)` | `#FF32CD32` | 酸橙綠 |
| `Colors::ForestGreen`| `(34, 139, 34, 255)` | `#FF228B22` | 森林綠 |
| `Colors::SeaGreen` | `(46, 139, 87, 255)` | `#FF2E8B57` | 海洋綠 |
| `Colors::MediumSeaGreen` | `(60, 179, 113, 255)`| `#FF3CB371` | 中海綠 |
| `Colors::DarkSeaGreen` | `(143, 188, 143, 255)`| `#FF8FBC8F` | 暗海綠 |
| `Colors::LightGreen` | `(144, 238, 144, 255)`| `#FF90EE90` | 淺綠色 |
| `Colors::PaleGreen` | `(152, 251, 152, 255)`| `#FF98FB98` | 蒼綠色 |
| `Colors::SpringGreen`| `(0, 255, 127, 255)` | `#FF00FF7F` | 春綠色 |
| `Colors::MediumSpringGreen` | `(0, 250, 154, 255)`| `#FF00FA9A` | 中春綠 |
| `Colors::LawnGreen` | `(124, 252, 0, 255)` | `#FF7CFC00` | 草坪綠 |
| `Colors::Chartreuse` | `(127, 255, 0, 255)` | `#FF7FFF00` | 黃綠色 |
| `Colors::GreenYellow`| `(173, 255, 47, 255)` | `#FFADFF2F` | 綠黃色 |
| `Colors::Olive` | `(128, 128, 0, 255)` | `#FF808000` | 橄欖綠 |
| `Colors::OliveDrab` | `(107, 142, 35, 255)`| `#FF6B8E23` | 暗橄欖綠 |
| `Colors::DarkOliveGreen` | `(85, 107, 47, 255)`| `#FF556B2F` | 深橄欖綠 |
| `Colors::YellowGreen`| `(154, 205, 50, 255)` | `#FF9ACD32` | 黃綠 |

### 3.4 藍色與青色系 (Blues & Cyans)

| 常數名稱 | (R, G, B, A) | 十六進位 ARGB | 預覽說明 |
| :--- | :--- | :--- | :--- |
| `Colors::Blue` | `(0, 0, 255, 255)` | `#FF0000FF` | 純藍色 |
| `Colors::DarkBlue` | `(0, 0, 139, 255)` | `#FF00008B` | 深藍色 |
| `Colors::MediumBlue` | `(0, 0, 205, 255)` | `#FF0000CD` | 中藍色 |
| `Colors::MidnightBlue`| `(25, 25, 112, 255)` | `#FF191970` | 午夜藍 |
| `Colors::Navy` | `(0, 0, 128, 255)` | `#FF000080` | 海軍藍 |
| `Colors::RoyalBlue` | `(65, 105, 225, 255)` | `#FF4169E1` | 皇家藍 |
| `Colors::SteelBlue` | `(70, 130, 180, 255)` | `#FF4682B4` | 鐵藍色 (經典主題色) |
| `Colors::DodgerBlue` | `(30, 144, 255, 255)` | `#FF1E90FF` | 道奇藍 |
| `Colors::DeepSkyBlue`| `(0, 191, 255, 255)` | `#FF00BFFF` | 深天藍 |
| `Colors::CornflowerBlue` | `(100, 149, 237, 255)`| `#FF6495ED` | 矢車菊藍 |
| `Colors::SkyBlue` | `(135, 206, 235, 255)`| `#FF87CEEB` | 天藍色 |
| `Colors::LightSkyBlue`| `(135, 206, 250, 255)`| `#FF87CEFA` | 淺天藍 |
| `Colors::LightBlue` | `(173, 216, 230, 255)`| `#FFADD8E6` | 淺藍色 |
| `Colors::PowderBlue` | `(176, 224, 230, 255)`| `#FFB0E0E6` | 粉末藍 |
| `Colors::CadetBlue` | `(95, 158, 160, 255)` | `#FF5F9EA0` | 軍服藍 |
| `Colors::Cyan` / `Aqua` | `(0, 255, 255, 255)` | `#FF00FFFF` | 青色 / 水藍色 |
| `Colors::DarkCyan` | `(0, 139, 139, 255)` | `#FF008B8B` | 深青色 |
| `Colors::Teal` | `(0, 128, 128, 255)` | `#FF008080` | 鴨翅綠 (藍綠色) |
| `Colors::DarkTurquoise` | `(0, 206, 209, 255)`| `#FF00CED1` | 暗綠松石色 |
| `Colors::Turquoise` | `(64, 224, 208, 255)` | `#FF40E0D0` | 綠松石色 |
| `Colors::MediumTurquoise` | `(72, 209, 204, 255)`| `#FF48D1CC`| 中綠松石色 |
| `Colors::Aquamarine` | `(127, 255, 212, 255)`| `#FF7FFFD4` | 碧綠色 |
| `Colors::LightSeaGreen` | `(32, 178, 170, 255)`| `#FF20B2AA` | 淺海綠 |

### 3.5 紫色系 (Purples & Violets)

| 常數名稱 | (R, G, B, A) | 十六進位 ARGB | 預覽說明 |
| :--- | :--- | :--- | :--- |
| `Colors::Purple` | `(128, 0, 128, 255)` | `#FF800080` | 純紫色 |
| `Colors::DarkMagenta`| `(139, 0, 139, 255)` | `#FF8B008B` | 深洋紅 |
| `Colors::Magenta` / `Fuchsia` | `(255, 0, 255, 255)` | `#FFFF00FF` | 洋紅色 / 紫紅色 |
| `Colors::Indigo` | `(75, 0, 130, 255)` | `#FF4B0082` | 靛藍色 |
| `Colors::BlueViolet` | `(138, 43, 226, 255)` | `#FF8A2BE2` | 藍紫色 |
| `Colors::DarkViolet` | `(148, 0, 211, 255)` | `#FF9400D3` | 暗紫色 |
| `Colors::DarkOrchid` | `(153, 50, 204, 255)` | `#FF9932CC` | 暗蘭紫 |
| `Colors::MediumOrchid`| `(186, 85, 211, 255)`| `#FFBA55D3` | 中蘭紫 |
| `Colors::Orchid` | `(218, 112, 214, 255)`| `#FFDA70D6` | 蘭花紫 |
| `Colors::MediumPurple`| `(147, 112, 219, 255)`| `#FF9370DB`| 中紫色 |
| `Colors::MediumSlateBlue` | `(123, 104, 238, 255)` | `#FF7B68EE` | 中石板藍 |
| `Colors::SlateBlue` | `(106, 90, 205, 255)` | `#FF6A5ACD` | 石板藍 |
| `Colors::DarkSlateBlue` | `(72, 61, 139, 255)`| `#FF483D8B` | 深石板藍 |
| `Colors::Plum` | `(221, 160, 221, 255)`| `#FFDDA0DD` | 李子紫 |
| `Colors::Violet` | `(238, 130, 238, 255)`| `#FFEE82EE` | 紫羅蘭色 |
| `Colors::Thistle` | `(216, 191, 216, 255)`| `#FFD8BFD8` | 薊紫 |
| `Colors::Lavender` | `(230, 230, 250, 255)`| `#FFE6E6FA` | 薰衣草紫 |

### 3.6 棕色系 (Browns)

| 常數名稱 | (R, G, B, A) | 十六進位 ARGB | 預覽說明 |
| :--- | :--- | :--- | :--- |
| `Colors::Brown` | `(165, 42, 42, 255)` | `#FFA52A2A` | 棕色 |
| `Colors::Maroon` | `(128, 0, 0, 255)` | `#FF800000` | 栗色 |
| `Colors::SaddleBrown`| `(139, 69, 19, 255)` | `#FF8B4513` | 馬鞍棕 |
| `Colors::Sienna` | `(160, 82, 45, 255)` | `#FFA0522D` | 赭色 |
| `Colors::Chocolate` | `(210, 105, 30, 255)` | `#FFD2691E` | 巧克力色 |
| `Colors::Peru` | `(205, 133, 63, 255)` | `#FFCD853F` | 秘魯褐 |
| `Colors::SandyBrown` | `(244, 164, 96, 255)` | `#FFF4A460` | 沙棕色 |
| `Colors::BurlyWood` | `(222, 184, 135, 255)`| `#FFDEB887` | 原木色 |
| `Colors::Tan` | `(210, 180, 140, 255)`| `#FFD2B48C` | 褐色 / 茶色 |
| `Colors::RosyBrown` | `(188, 143, 143, 255)`| `#FFBC8F8F` | 玫瑰褐 |

### 3.7 白色與淺色系 (Whites & Lights)

| 常數名稱 | (R, G, B, A) | 十六進位 ARGB | 預覽說明 |
| :--- | :--- | :--- | :--- |
| `Colors::White` | `(255, 255, 255, 255)`| `#FFFFFFFF` | 純白色 |
| `Colors::Snow` | `(255, 250, 250, 255)`| `#FFFFFAFA` | 雪白 |
| `Colors::Honeydew` | `(240, 255, 240, 255)`| `#FFF0FFF0` | 蜜瓜綠白 |
| `Colors::MintCream` | `(245, 255, 250, 255)`| `#FFF5FFFA` | 薄荷奶油白 |
| `Colors::Azure` | `(240, 255, 255, 255)`| `#FFF0FFFF` | 蔚藍白 |
| `Colors::AliceBlue` | `(240, 248, 255, 255)`| `#FFF0F8FF` | 愛麗絲藍 |
| `Colors::GhostWhite` | `(248, 248, 255, 255)`| `#FFF8F8FF` | 幽靈白 |
| `Colors::WhiteSmoke` | `(245, 245, 245, 255)`| `#FFF5F5F5` | 白煙色 |
| `Colors::Seashell` | `(255, 245, 238, 255)`| `#FFFFF5EE` | 海貝色 |
| `Colors::Beige` | `(245, 245, 220, 255)`| `#FFF5F5DC` | 米色 |
| `Colors::OldLace` | `(253, 245, 230, 255)`| `#FFFDF5E6` | 蕾絲白 |
| `Colors::FloralWhite`| `(255, 250, 240, 255)`| `#FFFFFAF0` | 花卉白 |
| `Colors::Ivory` | `(255, 255, 240, 255)`| `#FFFFFFF0` | 象牙白 |
| `Colors::AntiqueWhite`| `(250, 235, 215, 255)`| `#FFFAEBD7`| 古董白 |
| `Colors::Linen` | `(250, 240, 230, 255)`| `#FFFAF0E6` | 亞麻白 |
| `Colors::Cornsilk` | `(255, 248, 220, 255)`| `#FFFFF8DC` | 玉米絲色 |

### 3.8 灰色、黑色與透明 (Grays, Blacks & Transparent)

| 常數名稱 | (R, G, B, A) | 十六進位 ARGB | 預覽說明 |
| :--- | :--- | :--- | :--- |
| `Colors::Transparent`| `(0, 0, 0, 0)` | `#00000000` | 完全透明 |
| `Colors::Black` | `(0, 0, 0, 255)` | `#FF000000` | 純黑色 |
| `Colors::DarkSlateGray` | `(47, 79, 79, 255)`| `#FF2F4F4F`| 深岩石灰 |
| `Colors::DimGray` | `(105, 105, 105, 255)`| `#FF696969`| 昏暗灰 |
| `Colors::SlateGray` | `(112, 128, 144, 255)`| `#FF708090`| 灰石板色 |
| `Colors::LightSlateGray`| `(119, 136, 153, 255)`| `#FF778899`| 淺岩灰 |
| `Colors::Gray` | `(128, 128, 128, 255)`| `#FF808080`| 標準中灰 |
| `Colors::DarkGray` | `(169, 169, 169, 255)`| `#FFA9A9A9`| 深灰 |
| `Colors::Silver` | `(192, 192, 192, 255)`| `#FFC0C0C0`| 銀灰色 |
| `Colors::LightGray` | `(211, 211, 211, 255)`| `#FFD3D3D3`| 淺灰色 |
| `Colors::Gainsboro` | `(220, 220, 220, 255)`| `#FFDCDCDC`| 亮灰色 |

---

## 4. 範例程式碼 (Examples)

```cpp
#include "ui/graphics/Colors.hpp"
#include "ui/graphics/Pen.hpp"
#include "ui/graphics/Brush.hpp"
#include "ui/graphics/Graphics.hpp"

void SetupPaletteExample(cppui::graphics::Graphics& g) {
    // 1. 使用常數初始化 SolidBrush
    cppui::graphics::SolidBrush bgBrush(cppui::graphics::Colors::WhiteSmoke);
    g.FillRectangle(bgBrush, core::Rect(0, 0, 800, 600));

    // 2. 使用常數初始化 Pen
    cppui::graphics::Pen borderPen(cppui::graphics::Colors::SteelBlue, 2.0f);
    g.DrawRectangle(borderPen, core::Rect(50, 50, 200, 100));

    // 3. 常數色彩合成
    cppui::core::Color alertColor = cppui::core::Color::FromArgb(180, cppui::graphics::Colors::Crimson);
    cppui::graphics::SolidBrush alertBrush(alertColor);
    g.FillRoundedRectangle(alertBrush, core::Rect(300, 50, 200, 100), 8.0f);
}
```

---

## 5. 請參閱 (See Also)

* [`Color` 色彩結構詳解](graphics_color.md)
* [`Brushes` 常用標準純色畫刷集合](graphics_brushes.md)
* [`Pens` 常用標準畫筆集合](graphics_pens.md)
* [`Graphics` 繪圖中樞排程器](graphics_graphics.md)
