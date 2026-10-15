# LineBreaker 類別 (Class) & Unicode 斷行

`LineBreaker` 遵循 Unicode UAX #14 斷行規範 (Unicode Line Breaking Algorithm)，用於計算文字中的所有潛在換行點，支援中英混排、空格斷行與 CJK 標點符號禁則處理（行首禁止標點與行尾禁止標點）。

---

## 語法宣告 (Syntax)

```cpp
#include "ui/text/LineBreaker.hpp"

namespace cppui {
namespace text {

enum class BreakOpportunity : uint8_t;
struct LineBreak;
class LineBreaker;

} // namespace text
} // namespace cppui
```

---

## BreakOpportunity 列舉值

| 列舉常數 | 數值 | 說明 |
| :--- | :--- | :--- |
| `None` | `0` | 禁止斷行（例如多字節 UTF-8 中間或特定標點前方）。 |
| `CanBreak` | `1` | 允許軟折行（例如空格之後、中文字元之間）。 |
| `MustBreak` | `2` | 強制硬換行（例如遇到 `\n` 或 `\r\n`）。 |

---

## LineBreak 結構體成員

| 成員變數 | 類型 | 說明 |
| :--- | :--- | :--- |
| `position` | `size_t` | 斷行機會所在的文字字節位置。 |
| `opportunity` | `BreakOpportunity` | 此處之斷行機會等級。 |

---

## LineBreaker 成員方法

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `LineBreaker()` | 構造函式 | 初始化斷行器。 |
| `FindBreaks(compat::string_view text)` | `std::vector<LineBreak>` | 計算 UTF-8 字串中的所有潛在換行點。 |

---

## CJK 標點禁則規範支援
1. **行首禁則**：句號 (`。`)、逗號 (`，`)、問號 (`？`)、感嘆號 (`！`)、右括號 (`）`)、右引號 (`」`) 等符號前方嚴禁折行 (`BreakOpportunity::None`)。
2. **行尾禁則**：左括號 (`（`)、左引號 (`「`) 等符號後方嚴禁折行 (`BreakOpportunity::None`)。
3. **CJK 表意文字間換行**：中日韓漢字之間預設允許軟折行 (`BreakOpportunity::CanBreak`)。

---

## 程式碼範例 (Example)

```cpp
#include "ui/text/LineBreaker.hpp"
#include "compat/Compat.hpp"

using namespace cppui::text;

void LineBreakExample() {
    LineBreaker breaker;
    compat::string_view sample = "第一行內容\n第二行包含，標點符號。";

    std::vector<LineBreak> breaks = breaker.FindBreaks(sample);
    for (const auto& lb : breaks) {
        if (lb.opportunity == BreakOpportunity::MustBreak) {
            compat::println("位置 {} 處發現強制硬換行符號", lb.position);
        }
    }
}
```
