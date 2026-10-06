# TextLayout 類別 (Class) & 排版結果模型

`TextLayout` 代表一個完整的多行文字排版成果物件，封裝行切片度量、段落對齊、省略號裁切，並提供文字編輯器核心所必需的**雙向精確命中測試 (Hit-Testing: 座標 $\leftrightarrow$ 字元索引)**。

---

## 語法宣告 (Syntax)

```cpp
#include "ui/text/TextLayout.hpp"

namespace cppui {
namespace text {

enum class TextAlignment : uint8_t;
enum class TextWrapping : uint8_t;
enum class TextTrimming : uint8_t;
struct TextLine;
struct TextHitTestResult;
class TextLayout;

} // namespace text
} // namespace cppui
```

---

## 排版控制列舉

### TextAlignment（水平對齊）
* `Left`: 靠左對齊 (Leading)。
* `Center`: 置中對齊 (Center)。
* `Right`: 靠右對齊 (Trailing)。
* `Justify`: 兩端分散對齊 (Justified)。

### TextWrapping（自動折行）
* `NoWrap`: 不自動折行（單行溢出）。
* `WordWrap`: 單詞/斷行機會優先折行。
* `CharacterWrap`: 字元級硬折行。

### TextTrimming（省略號修剪）
* `None`: 直接裁切或溢出。
* `CharacterEllipsis`: 字元級超出時附加省略號 (`...`)。
* `WordEllipsis`: 單詞級超出時附加省略號 (`...`)。

---

## TextHitTestResult 結構體成員

| 成員變數 | 類型 | 說明 |
| :--- | :--- | :--- |
| `textPosition` | `size_t` | 命中命中的字元索引（在字串中的偏移）。 |
| `isTrailingHit`| `bool` | 是否命中該字元的後半部（用於決定游標應繪製於該字元之左側或右側）。 |
| `isInside` | `bool` | 點擊座標是否精確位於文字包圍盒內。 |
| `cursorRect` | `core::Rect` | 推薦之游標繪製幾何矩形（寬度通常為 1~2px，高度為該行字高）。 |

---

## TextLayout 成員方法

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `GetSize()` | `core::Size` | 取得排版之總寬高尺寸（以 DIP 浮點數表示）。 |
| `GetLineCount()` | `size_t` | 取得排版總行數。 |
| `GetLines()` | `compat::span<const TextLine>` | 取得所有排版行的切片視圖（現代借用語意，零拷貝）。 |
| `HitTestPoint(Point point)` | `TextHitTestResult` | 依據局部點擊座標執行命中測試，查詢字元位置與游標矩形。 |
| `HitTestTextPosition(size_t pos)`| `core::Rect` | 依據字元索引查詢其游標邊界幾何矩形（用於繪製閃爍插入游標）。 |
| `GetText()` | `compat::string_view` | 取得原始排版文字視圖。 |
| `GetFont()` | `const FontDescriptor&` | 取得此排版使用的字型規格。 |
| `GetNativeHandle()` | `void*` | 取得底層原生排版句柄（如 DirectWrite `IDWriteTextLayout*`）。 |

---

## 程式碼範例 (Example)

```cpp
#include "ui/text/TextLayoutEngine.hpp"
#include "ui/text/FontManager.hpp"
#include "compat/compat.hpp"

using namespace cppui::text;
using namespace cppui::core;

void LayoutHitTestExample() {
    FontDescriptor font = FontManager::Get().GetDefaultFont(14.0f);
    TextLayout layout = TextLayoutEngine::Get().CreateLayout(
        "Click here to test hit-testing!", font, 200.0f
    );

    // 模擬滑鼠在 (25, 10) 處點擊
    TextHitTestResult hit = layout.HitTestPoint(Point(25.0f, 10.0f));
    compat::println("點擊字元位置: {}, 游標 Rect: ({:.1f}, {:.1f}, {:.1f}, {:.1f})",
                    hit.textPosition, hit.cursorRect.x, hit.cursorRect.y, hit.cursorRect.width, hit.cursorRect.height);

    // 查詢字元 0 處的插入游標
    Rect caretRect = layout.HitTestTextPosition(0);
    compat::println("字首游標 Rect: ({:.1f}, {:.1f})", caretRect.x, caretRect.y);
}
```
