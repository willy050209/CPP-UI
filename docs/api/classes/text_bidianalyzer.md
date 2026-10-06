# BiDiAnalyzer 類別 (Class) & 雙向文字分析

`BiDiAnalyzer` 實作 Unicode UAX #9 雙向文字 (Bidirectional Text) 分析演算法，用於分析混排段落（例如拉丁/中文 LTR 與阿拉伯文/希伯來文 RTL），將段落切分為具備嵌入等級（Embedding Levels）的雙向區段，為視覺順序重排提供依據。

---

## 語法宣告 (Syntax)

```cpp
#include "ui/text/BiDiAnalyzer.hpp"

namespace cppui {
namespace text {

enum class BiDiDirection : uint8_t;
struct BiDiRun;
class BiDiAnalyzer;

} // namespace text
} // namespace cppui
```

---

## BiDiDirection 列舉值

| 列舉常數 | 數值 | 說明 |
| :--- | :--- | :--- |
| `LeftToRight` | `0` | 由左至右流向 (LTR)。 |
| `RightToLeft` | `1` | 由右至左流向 (RTL)。 |
| `Mixed` | `2` | 混排段落（同時包含 LTR 與 RTL 文字）。 |

---

## BiDiRun 結構體成員

| 成員變數 | 類型 | 說明 |
| :--- | :--- | :--- |
| `startIndex` | `size_t` | 在原始字串中的起始字節/字元索引。 |
| `length` | `size_t` | 此雙向切片的長度。 |
| `embeddingLevel`| `uint8_t` | Unicode 雙向嵌入等級（偶數表示 LTR，奇數表示 RTL）。 |
| `isRtl` | `bool` | 是否為由右至左書寫。 |

---

## BiDiAnalyzer 成員方法

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `BiDiAnalyzer()` | 構造函式 | 初始化雙向文字分析器。 |
| `Analyze(text, defaultDir)` | `std::vector<BiDiRun>` | 分析 UTF-8 段落文字並切分為雙向文字切片列表。 |
| `DetectBaseDirection(text)` | `BiDiDirection` | 判定段落的基礎文本主流向。 |

---

## 程式碼範例 (Example)

```cpp
#include "ui/text/BiDiAnalyzer.hpp"
#include "compat/compat.hpp"

using namespace cppui::text;

void BiDiExample() {
    BiDiAnalyzer analyzer;
    compat::string_view mixed = "Welcome to CPPUI (مرحبا)!";

    BiDiDirection baseDir = analyzer.DetectBaseDirection(mixed);
    compat::println("基礎方向: {}", (baseDir == BiDiDirection::Mixed) ? "Mixed" : "LTR");

    std::vector<BiDiRun> runs = analyzer.Analyze(mixed);
    for (const auto& r : runs) {
        compat::println("Run [{}, len={}], isRtl: {}, level: {}",
                        r.startIndex, r.length, r.isRtl ? "TRUE" : "FALSE", r.embeddingLevel);
    }
}
```
