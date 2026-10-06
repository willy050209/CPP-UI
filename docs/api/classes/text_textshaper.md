# TextShaper 類別 (Class) & 塑形字形

`TextShaper` 負責將未處理的 UTF-8 字串與字型規格輸入至排版引擎，透過 OpenType 特性解析產出帶有幾何度量（Glyph ID、集群對應、X/Y 推進量與視覺偏移）的塑形字符切片 (`ShapedRun`)。

---

## 語法宣告 (Syntax)

```cpp
#include "ui/text/TextShaper.hpp"

namespace cppui {
namespace text {

struct ShapedGlyph;
class ShapedRun;
class TextShaper;

} // namespace text
} // namespace cppui
```

---

## ShapedGlyph 結構體成員

| 成員變數 | 類型 | 說明 |
| :--- | :--- | :--- |
| `glyphId` | `uint16_t` | OpenType 字形識別碼 (Glyph Index)。 |
| `clusterIndex` | `uint32_t` | 對應於原始 UTF-8 字串中的集群起始位置。 |
| `xAdvance` | `float` | 橫向推進寬度（繪製下一個字形時的 X 軸位移）。 |
| `yAdvance` | `float` | 縱向推進高度（通常橫排文字為 0）。 |
| `xOffset` | `float` | 橫向微調視覺偏移量 (Advance Offset)。 |
| `yOffset` | `float` | 縱向微調視覺偏移量 (Ascender Offset)。 |

---

## ShapedRun 成員方法

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `GetFont()` | `const FontDescriptor&` | 取得此字符切片使用的字型規格。 |
| `IsRtl()` | `bool` | 判定此切片是否為由右至左書寫 (Right-To-Left)。 |
| `GetTotalWidth()` | `float` | 取得整段字符切片的總累加寬度。 |
| `GetGlyphs()` | `compat::span<const ShapedGlyph>` | 取得所有塑形字形的連續緩衝區唯讀視圖（現代借用語意）。 |
| `GetGlyphCount()` | `size_t` | 取得字形數量。 |

---

## TextShaper 成員方法

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `TextShaper()` | 構造函式 | 初始化字形塑形器。 |
| `Shape(compat::string_view text, const FontDescriptor& font)` | `std::vector<ShapedRun>` | 將 UTF-8 字串塑形為帶有幾何度量的 `ShapedRun` 列表。 |

---

## 程式碼範例 (Example)

```cpp
#include "ui/text/TextShaper.hpp"
#include "ui/text/FontManager.hpp"
#include "compat/compat.hpp"

using namespace cppui::text;

void ShapeExample() {
    TextShaper shaper;
    FontDescriptor font = FontManager::Get().GetDefaultFont(18.0f);

    compat::string_view text = "Hello CPPUI!";
    std::vector<ShapedRun> runs = shaper.Shape(text, font);

    for (const auto& run : runs) {
        compat::println("Run 寬度: {:.2f}, 字形數: {}", run.GetTotalWidth(), run.GetGlyphCount());
        for (const auto& g : run.GetGlyphs()) {
            compat::println("  GlyphID: {}, xAdvance: {:.2f}", g.glyphId, g.xAdvance);
        }
    }
}
```
