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

### 1. 預設字型規格查詢
| 方法名稱 | 傳回值 | 說明 |
| :--- | :--- | :--- |
| `GetDefaultFont(float size = 14.0f)` | `FontDescriptor` | 取得系統標準預設 UI 字型規格（預設 Windows: `"Segoe UI"`，可由 `SetDefaultFontFamily` 覆寫）。 |
| `GetDefaultChineseFont(float size = 14.0f)` | `FontDescriptor` | 取得系統預設繁體中文 UI 字型（預設 Windows: `"Microsoft JhengHei UI"`，可由 `SetDefaultChineseFontFamily` 覆寫）。 |
| `GetDefaultMonospaceFont(float size = 14.0f)` | `FontDescriptor` | 取得系統預設等寬字型（預設 Windows: `"Cascadia Code"` / `"Consolas"`，可由 `SetDefaultMonospaceFontFamily` 覆寫）。 |
| `HasFontFamily(compat::string_view familyName)` | `bool` | 查詢系統字型庫或動態註冊庫中是否存在特定字型家族名稱。 |
| `GetFontMetrics(const FontDescriptor& font)` | `FontMetrics` | 查詢特定字型描述之度量數據（基線、行高、升部、降部）。 |

### 2. 動態字型註冊與實體路徑解析 (Dynamic Font Loading & Path Resolution)
| 方法名稱 | 傳回值 | 說明與例外 |
| :--- | :--- | :--- |
| `RegisterFontFile(compat::string_view path)` | `bool` | 載入本機字型檔（支援 `.ttf`, `.otf`, `.ttc`）。Windows 底層調用 `AddFontResourceExW`；跨平臺維護已註冊字族快取與檔案映射。若路徑為空擲出 `std::invalid_argument`。 |
| `RegisterFontFromMemory(compat::span<const uint8_t> data)` | `bool` | 從記憶體二進位緩衝區載入字型。Windows 底層調用 `AddFontMemResourceEx`。若傳入空緩衝區擲出 `std::invalid_argument`。 |
| `GetRegisteredFontFamilies()` | `std::vector<std::string>` | 取得透過本類別成功註冊的所有動態字型家族名稱清單。 |
| `ResolveFontFilePath(compat::string_view familyName = "")` | `std::string` | 解析指定字型家族之實體檔案路徑（以供 FreeType 等原生光柵引擎載入）。在非 Windows (WSL / Linux) 平台依序探測：WSL 掛載之新細明體 (`mingliu.ttc`) $\rightarrow$ 微軟正黑體 (`msjh.ttc`) $\rightarrow$ 思源黑體 (`NotoSansCJK-Regular.ttc`) $\rightarrow$ 文泉驛微米黑 (`wqy-microhei.ttc`)。 |

### 3. 全域預設字型管理 (Default Font Family Override)
| 方法名稱 | 傳回值 | 說明與例外 |
| :--- | :--- | :--- |
| `SetDefaultFontFamily(compat::string_view familyName)` | `void` | 覆寫全域預設 UI 字型家族名稱。若為空擲出 `std::invalid_argument`。 |
| `SetDefaultChineseFontFamily(compat::string_view familyName)` | `void` | 覆寫全域預設繁體中文字型家族名稱。若為空擲出 `std::invalid_argument`。 |
| `SetDefaultMonospaceFontFamily(compat::string_view familyName)` | `void` | 覆寫全域預設等寬字型家族名稱。若為空擲出 `std::invalid_argument`。 |
| `ResetDefaultFonts()` | `void` | 重設全域預設字型為作業系統原始出廠預設值。 |

---

## 程式碼範例 (Example)

```cpp
#include "ui/text/FontManager.hpp"
#include "compat/compat.hpp"

using namespace cppui::text;

void FontManagementDemo() {
    FontManager& fm = FontManager::Get();

    // 1. 動態載入專案自訂字型檔 (如思源黑體)
    bool loaded = fm.RegisterFontFile("assets/fonts/NotoSansTC-Regular.otf");
    if (loaded) {
        compat::println("自訂字型載入成功！");
        // 設定為全域繁體中文預設字型
        fm.SetDefaultChineseFontFamily("Noto Sans TC");
    }

    // 2. 取得當前套用之預設中文字型規格
    FontDescriptor uiFont = fm.GetDefaultChineseFont(16.0f);
    FontMetrics metrics = fm.GetFontMetrics(uiFont);

    compat::println("目前繁中字型: {}, 行高: {:.2f} px",
                    uiFont.GetFamily(), metrics.GetLineHeight());

    // 3. 列出所有動態註冊之字型
    for (const auto& family : fm.GetRegisteredFontFamilies()) {
        compat::println("動態註冊家族: {}", family);
    }
}
```
