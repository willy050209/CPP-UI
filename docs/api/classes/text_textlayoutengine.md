# TextLayoutEngine 類別 (Class)

`TextLayoutEngine` 是 CPPUI 的高階多行排版求解器，負責根據輸入文字、字型規格與佈局約束（最大寬度、最大高度、折行、對齊、省略號修剪）建立完整的 `TextLayout` 物件，並提供極速的單次尺寸度量捷徑 (`MeasureText`)。

---

## 語法宣告 (Syntax)

```cpp
#include "ui/text/TextLayoutEngine.hpp"

namespace cppui {
namespace text {

class TextLayoutEngine;

} // namespace text
} // namespace cppui
```

---

## 靜態方法 (Static Methods)

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `Get()` | `TextLayoutEngine&` | 取得進程唯一單例實例。 |

---

## 成員方法 (Methods)

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `CreateLayout(text, font, maxWidth, maxHeight, alignment, wrapping, trimming)` | `TextLayout` | 依據字串、字型與排版約束建立完整多行 `TextLayout` 物件。 |
| `MeasureText(text, font, maxWidth = 1e9f)` | `core::Size` | 快速文字尺寸度量（專供 M4 佈局階段 Measure/Arrange 高速調用）。 |

---

## 效能備註 (Remarks)
* `MeasureText` 內部整合字型快取與排版管線，在現代 x64 系統上達到每秒百萬次查詢（平均單次查詢延遲低於 40 微秒）。
* 當輸入字串為空時，`MeasureText` 自動返回寬度 0 與對應字型的標準單行高度，避免 UI 坍塌。

---

## 程式碼範例 (Example)

```cpp
#include "ui/text/TextLayoutEngine.hpp"
#include "ui/text/FontManager.hpp"
#include "compat/Compat.hpp"

using namespace cppui::text;
using namespace cppui::core;

void EngineExample() {
    TextLayoutEngine& engine = TextLayoutEngine::Get();
    FontDescriptor font = FontManager::Get().GetDefaultFont(16.0f);

    // 1. 高速度量文字長度
    Size textSize = engine.MeasureText("Hello CPPUI Performance!", font, 300.0f);
    compat::println("度量寬高: {:.2f} x {:.2f}", textSize.width, textSize.height);

    // 2. 建立置中對齊且帶有省略號的排版
    TextLayout layout = engine.CreateLayout(
        "Long Text with ellipsis...",
        font,
        150.0f,
        40.0f,
        TextAlignment::Center,
        TextWrapping::NoWrap,
        TextTrimming::CharacterEllipsis
    );
    compat::println("排版行數: {}", layout.GetLineCount());
}
```
