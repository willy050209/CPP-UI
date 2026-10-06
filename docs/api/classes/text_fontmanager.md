# FontManager 類別 (Class)

`FontManager` 是 CPPUI 跨平台系統字型管理與度量快取中樞，負責系統預設字型解析（UI 字型、繁中字型、等寬字型）、系統字型庫家族檢驗與原生字型度量快取。

---

## 語法宣告 (Syntax)

```cpp
#include "ui/text/FontManager.hpp"

namespace cppui {
namespace text {

class FontManager;

} // namespace text
} // namespace cppui
```

---

## 靜態方法 (Static Methods)

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `Get()` | `FontManager&` | 取得進程唯一的單例 `FontManager` 實例。 |

---

## 成員方法 (Methods)

| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `GetDefaultFont(float size = 14.0f)` | `FontDescriptor` | 取得作業系統標準預設 UI 字型規格（Windows: `"Segoe UI"`）。 |
| `GetDefaultChineseFont(float size = 14.0f)` | `FontDescriptor` | 取得系統預設繁體中文 UI 字型（Windows: `"Microsoft JhengHei UI"` / `"Microsoft JhengHei"`）。 |
| `GetDefaultMonospaceFont(float size = 14.0f)` | `FontDescriptor` | 取得系統預設等寬字型（Windows: `"Cascadia Code"` / `"Consolas"`）。 |
| `HasFontFamily(compat::string_view familyName)` | `bool` | 查詢系統字型庫中是否存在特定字型家族名稱。 |
| `GetFontMetrics(const FontDescriptor& font)` | `FontMetrics` | 查詢特定字型描述之度量數據（基線、行高、升部、降部）。 |

---

## 程式碼範例 (Example)

```cpp
#include "ui/text/FontManager.hpp"
#include "compat/compat.hpp"

using namespace cppui::text;

void CheckSystemFonts() {
    FontManager& fm = FontManager::Get();

    // 取得預設字型
    FontDescriptor uiFont = fm.GetDefaultChineseFont(16.0f);
    FontMetrics metrics = fm.GetFontMetrics(uiFont);

    compat::println("字型家族: {}", uiFont.GetFamily());
    compat::println("推薦行高: {:.2f} px (升部: {:.2f}, 降部: {:.2f})",
                    metrics.GetLineHeight(), metrics.ascent, metrics.descent);

    // 檢驗字型是否存在
    bool hasArial = fm.HasFontFamily("Arial");
    compat::println("系統是否具備 Arial: {}", hasArial ? "YES" : "NO");
}
```
