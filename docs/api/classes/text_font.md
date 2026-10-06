# FontDescriptor 類別 (Class) & 字型規格

`FontDescriptor` 封裝字型家族名稱、字級尺寸（DIP）、粗細等級、樣式傾斜度與伸展比例，是 CPPUI 文字排版與繪圖管線中的核心規格描述型別。

---

## 語法宣告 (Syntax)

```cpp
#include "ui/text/Font.hpp"

namespace cppui {
namespace text {

class FontDescriptor;
struct FontMetrics;
enum class FontWeight : uint16_t;
enum class FontStyle : uint8_t;
enum class FontStretch : uint8_t;

} // namespace text
} // namespace cppui
```

---

## FontWeight 列舉值

| 列舉常數 | 數值 | 說明 |
| :--- | :--- | :--- |
| `Thin` | `100` | 極細體 (Thin / Hairline) |
| `ExtraLight` | `200` | 特細體 (Extra Light / Ultra Light) |
| `Light` | `300` | 細體 (Light) |
| `SemiLight` | `350` | 次細體 (Semi Light) |
| `Normal` / `Regular` | `400` | 標準常規體 (Normal / Regular) |
| `Medium` | `500` | 中等體 (Medium) |
| `SemiBold` | `600` | 次粗體 (Semi Bold / Demi Bold) |
| `Bold` | `700` | 粗體 (Bold) |
| `ExtraBold` | `800` | 特粗體 (Extra Bold / Ultra Bold) |
| `Black` | `900` | 黑體 (Black / Heavy) |
| `ExtraBlack` | `950` | 特黑體 (Extra Black) |

---

## FontStyle 列舉值

| 列舉常數 | 數值 | 說明 |
| :--- | :--- | :--- |
| `Normal` | `0` | 正常直立體。 |
| `Oblique` | `1` | 機械傾斜體 (Oblique)。 |
| `Italic` | `2` | 專用斜體字型 (Italic)。 |

---

## FontMetrics 結構體成員

| 成員變數 | 類型 | 說明 |
| :--- | :--- | :--- |
| `ascent` | `float` | 基線 (Baseline) 以上的高度。 |
| `descent` | `float` | 基線以下的高度。 |
| `lineGap` | `float` | 推薦之額外行間距。 |
| `capHeight` | `float` | 大寫英文字母之視覺頂部高度。 |
| `xHeight` | `float` | 小寫字母 `x` 之主體高度。 |
| `underlinePosition` | `float` | 底線相對於基線之垂直偏移。 |
| `underlineThickness`| `float` | 底線筆刷粗細。 |

### 成員方法：
* `COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetLineHeight() const noexcept`：回傳推薦總行高 (`ascent + descent + lineGap`)。

---

## FontDescriptor 成員方法 (Methods)

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `FontDescriptor()` | 構造函式 | 預設建立 `"Segoe UI"`, `14.0f`, `FontWeight::Normal`。 |
| `FontDescriptor(family, size, weight, style, stretch)` | 構造函式 | 以指定參數構造字型描述。唯讀字串參數強制採用 `compat::string_view`。 |
| `GetFamily()` | `compat::string_view` | 取得字型家族名稱視圖（零拷貝）。 |
| `GetSize()` | `float` | 取得字級大小（以 DIP / 像素為單位）。 |
| `GetWeight()` | `FontWeight` | 取得粗細等級。 |
| `GetStyle()` | `FontStyle` | 取得字型樣式傾斜度。 |
| `GetStretch()` | `FontStretch` | 取得橫向伸展等級。 |
| `WithSize(float size)` | `FontDescriptor` | 鏈式構造器：產生覆蓋指定尺寸之新物件。 |
| `WithWeight(FontWeight weight)`| `FontDescriptor` | 鏈式構造器：產生覆蓋指定粗細之新物件。 |
| `WithStyle(FontStyle style)` | `FontDescriptor` | 鏈式構造器：產生覆蓋指定樣式之新物件。 |

---

## 程式碼範例 (Example)

```cpp
#include "ui/text/Font.hpp"
#include "compat/compat.hpp"

using namespace cppui::text;

void FontExample() {
    // 建立基礎字型描述
    FontDescriptor baseFont("Microsoft JhengHei UI", 14.0f);

    // 透過流式方法衍生粗斜體標題字型
    FontDescriptor heading = baseFont.WithSize(22.0f)
                                     .WithWeight(FontWeight::Bold)
                                     .WithStyle(FontStyle::Italic);

    compat::println("Family: {}, Size: {:.1f}, Weight: {}",
                    heading.GetFamily(), heading.GetSize(), static_cast<int>(heading.GetWeight()));
}
```
