# cppui::text 命名空間 (Namespace)

`cppui::text` 命名空間提供 CPPUI 跨平台現代 UI 框架之**工業級文字整形與多行排版管線 (Text Pipeline)**。底層採用雙軌架構（Windows 原生 DirectWrite 工業級引擎 + 跨平台 HarfBuzz/ICU 抽象插槽），支援複合字形塑形、彩色 Emoji、Unicode UAX #9 雙向文字 (BiDi) 分析、Unicode UAX #14 斷行規範、多行自動折行、省略號修剪以及高精準度的雙向命中測試 (Hit-Testing)。

---

## 包含標頭檔 (Headers)

```cpp
#include "ui/text/Font.hpp"
#include "ui/text/FontManager.hpp"
#include "ui/text/TextShaper.hpp"
#include "ui/text/BiDiAnalyzer.hpp"
#include "ui/text/LineBreaker.hpp"
#include "ui/text/TextLayout.hpp"
#include "ui/text/TextLayoutEngine.hpp"
```

---

## 列舉類型 (Enumerations)

| 列舉名稱 | 說明 |
| :--- | :--- |
| [`FontWeight`](../classes/text_font.md#fontweight) | 字型粗細等級 (`Thin` ~ `Black`，100 ~ 950)。 |
| [`FontStyle`](../classes/text_font.md#fontstyle) | 字型樣式傾斜度 (`Normal`, `Oblique`, `Italic`)。 |
| [`FontStretch`](../classes/text_font.md#fontstretch) | 字型橫向伸展等級 (`UltraCondensed` ~ `UltraExpanded`)。 |
| [`BiDiDirection`](../classes/text_bidianalyzer.md#bididirection) | 雙向文字段落總體文本流向 (`LeftToRight`, `RightToLeft`, `Mixed`)。 |
| [`BreakOpportunity`](../classes/text_linebreaker.md#breakopportunity) | Unicode 換行機會等級 (`None`, `CanBreak`, `MustBreak`)。 |
| [`TextAlignment`](../classes/text_textlayout.md#textalignment) | 段落水平對齊模式 (`Left`, `Center`, `Right`, `Justify`)。 |
| [`TextWrapping`](../classes/text_textlayout.md#textwrapping) | 自動折行策略 (`NoWrap`, `WordWrap`, `CharacterWrap`)。 |
| [`TextTrimming`](../classes/text_textlayout.md#texttrimming) | 超出邊界時之省略號修剪策略 (`None`, `CharacterEllipsis`, `WordEllipsis`)。 |

---

## 結構體與類別 (Classes & Structs)

| 類型名稱 | 種類 | 說明 |
| :--- | :--- | :--- |
| [`FontDescriptor`](../classes/text_font.md) | `class` | 包含字型家族名稱、字級尺寸、粗細與樣式的不可變字型規格。 |
| [`FontMetrics`](../classes/text_font.md#fontmetrics) | `struct` | 字型度量數值（`ascent`, `descent`, `lineGap`, `capHeight`, `xHeight` 等）。 |
| [`FontManager`](../classes/text_fontmanager.md) | `class` | 跨平台字型中樞，支援系統字型匹配、本機/記憶體字型動態載入、全域預設字族管理與度量快取。 |
| [`ShapedGlyph`](../classes/text_textshaper.md#shapedglyph) | `struct` | 單一塑形字形（OpenType Glyph ID、Cluster 索引、X/Y Advance 與 Offset）。 |
| [`ShapedRun`](../classes/text_textshaper.md#shapedrun) | `class` | 一組同字型、同流向的連續塑形字符切片（提供 `compat::span` 視圖存取）。 |
| [`TextShaper`](../classes/text_textshaper.md) | `class` | 將 UTF-8 字串塑形為 OpenType 字符度量列表之核心塑形機。 |
| [`BiDiRun`](../classes/text_bidianalyzer.md#bidirun) | `struct` | 雙向文字分析單一方向區段（起始索引、長度、嵌入等級、是否為 RTL）。 |
| [`BiDiAnalyzer`](../classes/text_bidianalyzer.md) | `class` | Unicode UAX #9 雙向文字分析器。 |
| [`LineBreak`](../classes/text_linebreaker.md#linebreak) | `struct` | 換行點索引與斷行等級。 |
| [`LineBreaker`](../classes/text_linebreaker.md) | `class` | Unicode UAX #14 斷行規則分析器（含 CJK 標點禁則處理）。 |
| [`TextLine`](../classes/text_textlayout.md#textline) | `struct` | 排版後的單行文字邊界、基線、字串範圍與折行標記。 |
| [`TextHitTestResult`](../classes/text_textlayout.md#texthittestresult) | `struct` | 命中測試結果（命中字元位置、是否為後半字元、游標繪製矩形）。 |
| [`TextLayout`](../classes/text_textlayout.md) | `class` | 完整多行文字排版成果物件（支援行切片視圖、雙向命中測試與度量）。 |
| [`TextLayoutEngine`](../classes/text_textlayoutengine.md) | `class` | 高階多行排版求解器與高速文字度量 (`MeasureText`) 工廠。 |

---

## 程式碼範例 (Code Example)

```cpp
#include "ui/text/FontManager.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include "compat/Compat.hpp"

using namespace cppui::text;

void Example() {
    // 1. 取得系統繁體中文字型規格
    FontDescriptor font = FontManager::Get().GetDefaultChineseFont(16.0f)
                                            .WithWeight(FontWeight::SemiBold);

    // 2. 建立限制寬度 300px 的多行自動折行排版
    compat::string_view content = "CPPUI 工業級文字排版引擎支援繁體中文、多行自動折行與精準雙向命中測試。";
    TextLayout layout = TextLayoutEngine::Get().CreateLayout(
        content, font, 300.0f, 1000.0f,
        TextAlignment::Left, TextWrapping::WordWrap, TextTrimming::None
    );

    compat::println("總排版行數: {}", layout.GetLineCount());
    compat::println("總排版尺寸: {:.1f} x {:.1f}", layout.GetSize().width, layout.GetSize().height);

    // 3. 雙向命中測試：以滑鼠座標查詢字元與游標框
    TextHitTestResult hit = layout.HitTestPoint(cppui::core::Point(45.0f, 12.0f));
    compat::println("命中字元索引: {}, 游標 Rect: ({:.1f}, {:.1f}, {:.1f}, {:.1f})",
                    hit.textPosition, hit.cursorRect.x, hit.cursorRect.y, hit.cursorRect.width, hit.cursorRect.height);
}
```
