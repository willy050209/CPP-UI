# Theme 類別 (cppui::theme::Theme)

`Theme` 是集中式設計規格與主題樣式的實體類別，封裝色彩代幣 ([`ColorTokens`](theme_tokens.md#1-色彩代幣-colortokens))、排版代幣 ([`TypographyTokens`](theme_tokens.md#2-排版代幣-typographytokens)) 與幾何尺寸代幣 ([`MetricTokens`](theme_tokens.md#3-幾何與間距代幣-metrictokens))。

---

## 1. 元資料 (Metadata)

* **命名空間**：[`cppui::theme`](../namespaces/cppui_theme.md)
* **標頭檔**：`#include <ui/theme/Theme.hpp>`
* **模組**：`CPPUI Theme`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 2. 語法宣告 (Syntax)

```cpp
namespace cppui {
namespace theme {

class Theme {
public:
    COMPAT_NODISCARD static Theme Dark();
    COMPAT_NODISCARD static Theme Light();
    COMPAT_NODISCARD static Theme HighContrast();

    Theme();

    COMPAT_NODISCARD ThemeMode GetMode() const noexcept;
    void SetMode(ThemeMode mode) noexcept;

    COMPAT_NODISCARD const ColorTokens& Colors() const noexcept;
    COMPAT_NODISCARD ColorTokens& Colors() noexcept;

    COMPAT_NODISCARD const TypographyTokens& Typography() const noexcept;
    COMPAT_NODISCARD TypographyTokens& Typography() noexcept;

    COMPAT_NODISCARD const MetricTokens& Metrics() const noexcept;
    COMPAT_NODISCARD MetricTokens& Metrics() noexcept;

    // 鏈式客製設定
    Theme& WithAccent(core::Color primary, core::Color hover, core::Color pressed);
    Theme& WithFontFamily(compat::string_view family);
    Theme& WithCornerRadius(float mediumRadius);
};

} // namespace theme
} // namespace cppui
```

---

## 3. 靜態工廠方法 (Static Factory Methods)

### `Dark()`
建立預設暗色主題實例（現代專業暗色調，深灰底色搭配鮮明強調色）。
```cpp
COMPAT_NODISCARD static Theme Dark();
```

### `Light()`
建立預設亮色主題實例（清新明亮、低眩光表面搭配高對比深色文字）。
```cpp
COMPAT_NODISCARD static Theme Light();
```

### `HighContrast()`
建立高對比輔助主題實例（純黑底色、純白文字與高彩度青黃邊界，專為視覺無障礙設計）。
```cpp
COMPAT_NODISCARD static Theme HighContrast();
```

---

## 4. 鏈式客製方法 (Fluent Chaining Methods)

### `WithAccent(core::Color primary, core::Color hover, core::Color pressed)`
覆寫全域主要品牌強調色彩（包括主色、懸停色與按下色）。
* **傳回值**：自身引用 `Theme&`，支援連續鏈式調用。

### `WithFontFamily(compat::string_view family)`
覆寫全域預設 UI 字型家族名稱。
* **參數**：`family` 為字型名稱視圖（例如 `"Segoe UI"`, `"Inter"`, `"Noto Sans TC"`）。
* **例外**：若 `family` 為空字串，立即擲出 `std::invalid_argument`。
* **傳回值**：自身引用 `Theme&`。

### `WithCornerRadius(float mediumRadius)`
依據中型圓角半徑基準，等比例自動計算設定 `cornerRadiusSmall` (`mediumRadius * 0.5f`)、`cornerRadiusMedium` (`mediumRadius`) 與 `cornerRadiusLarge` (`mediumRadius * 1.5f`)。
* **傳回值**：自身引用 `Theme&`。

---

## 5. 範例程式碼 (Example)

```cpp
#include "ui/theme/Theme.hpp"
#include "compat/Compat.hpp"

void CreateCustomBrandTheme() {
    using namespace cppui::theme;
    using namespace cppui::core;

    // 基於預設暗色主題派生，鏈式指定企業品牌規格
    Theme theme = Theme::Dark()
        .WithAccent(Color(0, 160, 255, 255), Color(30, 180, 255, 255), Color(0, 130, 220, 255))
        .WithFontFamily("Segoe UI")
        .WithCornerRadius(6.0f);

    // 精細微調特定語意代幣
    theme.Colors().success = Color(35, 180, 80, 255);
    theme.Metrics().paddingButton = Thickness(20.0f, 10.0f, 20.0f, 10.0f);

    compat::println("主題建立成功，按鈕內距: {:.1f}x{:.1f}, 預設字型: {}",
                    theme.Metrics().paddingButton.left, theme.Metrics().paddingButton.top,
                    theme.Typography().fontFamily);
}
```

---

## 6. 請參閱 (See Also)

* [`ThemeManager` 全域主題管理器](theme_thememanager.md)
* [`ThemeTokens` 設計規格代幣矩陣](theme_tokens.md)
* [主題與樣式系統開發手冊](../guides/theme_and_styling_system.md)
