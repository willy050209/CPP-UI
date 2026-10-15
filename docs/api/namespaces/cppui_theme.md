# cppui::theme 命名空間 (Namespace)

`cppui::theme` 命名空間代表 **集中式設計規格與主題樣式系統 (Theme & Styling System)**，提供全域統一的色彩代幣 (Color Tokens)、排版與字級規範 (Typography Tokens)、幾何尺寸與圓角 (Metric Tokens) 以及執行期即時主題切換與事件廣播中樞。

---

## 1. 簡介 (Description)

在傳統桌面應用開發中，介面樣式常面臨三大痛點：
1. **顏色與尺寸硬編碼 (Hardcoding)**：控制項直接綁定固定的十六進位色碼或像素尺寸，導致暗色模式 (Dark Mode) 或高對比輔助模式的實作極其繁瑣且易遺漏。
2. **缺乏一致的設計規範 (Design Tokens)**：按鈕圓角、邊框粗細、間距階層與文字字級分散在各個畫面，缺乏統一的設計語言。
3. **執行期動態切換困難**：當使用者在系統偏好設定切換亮/暗主題時，往往需要重啟應用程式或撰寫複雜的手動更新代碼。

`cppui::theme` 徹底解決上述問題：
* **純粹無副作用的設計代幣**：以強型別結構體 [`ColorTokens`](../classes/theme_tokens.md#1-色彩代幣-colortokens)、[`TypographyTokens`](../classes/theme_tokens.md#2-排版代幣-typographytokens)、[`MetricTokens`](../classes/theme_tokens.md#3-幾何與間距代幣-metrictokens) 明確定義設計規格。
* **原廠多模式主題支援**：內建現代暗色模式 ([`Theme::Dark()`](../classes/theme_theme.md#靜態工廠方法))、清新亮色模式 ([`Theme::Light()`](../classes/theme_theme.md#靜態工廠方法)) 與無障礙專用之高對比模式 ([`Theme::HighContrast()`](../classes/theme_theme.md#靜態工廠方法))。
* **單例動態中樞與廣播機制**：透過 [`ThemeManager`](../classes/theme_thememanager.md) 提供執行期即時主題切換與觀察者事件廣播。
* **階層覆寫與安全回退原則**：控制項遵循「顯式指定優先；未設定則繼承當前主題」原則，兼顧客製靈活性與主題一致性。

---

## 2. 類別與型別索引 (Classes & Types)

| 型別 (Type) | 類別形態 | 簡介與職責 |
| :--- | :--- | :--- |
| [`Theme`](../classes/theme_theme.md) | 實體類別 | 集中式主題實體，封裝色彩、排版與幾何代幣，並支援鏈式客製設定（`WithAccent`, `WithFontFamily`, `WithCornerRadius`）。 |
| [`ThemeManager`](../classes/theme_thememanager.md) | 單例中樞類別 | 全域主題管理器，提供進程唯一的主題快取、執行期模式即時切換與主題變更事件分發。 |
| [`ColorTokens`](../classes/theme_tokens.md#1-色彩代幣-colortokens) | 結構體 | 涵蓋視窗、表面容器、控制項各狀態色、品牌強調色、排版字色與語意反饋的色彩代幣。 |
| [`TypographyTokens`](../classes/theme_tokens.md#2-排版代幣-typographytokens) | 結構體 | 集中式字型家族、等寬家族與階層式字級尺度（Caption 11pt 至 Header 24pt）及字重。 |
| [`MetricTokens`](../classes/theme_tokens.md#3-幾何與間距代幣-metrictokens) | 結構體 | 集中式邊框粗細、圓角半徑（Small 3px 至 Pill 999px）、控制項內距與排版間距階層。 |
| `ThemeMode` | 列舉 | 主題運作模式列舉（`Dark = 0`, `Light = 1`, `HighContrast = 2`, `Custom = 3`）。 |

---

## 3. 範例程式碼 (Example)

```cpp
#include "ui/theme/ThemeManager.hpp"
#include "compat/Compat.hpp"

void SetupApplicationTheme() {
    using namespace cppui::theme;

    // 1. 取得全域主題中樞
    ThemeManager& tm = ThemeManager::Get();

    // 2. 切換為亮色模式
    tm.SetThemeMode(ThemeMode::Light);

    // 3. 以客製鏈式語法语法打造企業主題
    Theme brandTheme = Theme::Dark()
        .WithAccent(
            cppui::core::Color(0, 200, 150, 255), // 主要翡翠綠
            cppui::core::Color(20, 220, 170, 255),
            cppui::core::Color(0, 170, 130, 255)
        )
        .WithFontFamily("Segoe UI")
        .WithCornerRadius(8.0f);

    // 套用企業客製主題
    tm.SetTheme(brandTheme);

    // 4. 註冊動態主題切換監聽器
    size_t handlerId = tm.AddThemeChangedHandler([](const Theme& activeTheme) {
        compat::println("檢測到全域主題變更，當前模式: {}", static_cast<uint32_t>(activeTheme.GetMode()));
    });
}
```

---

## 4. 請參閱 (See Also)

* [`Theme` 類別詳細手冊](../classes/theme_theme.md)
* [`ThemeManager` 單例中樞手冊](../classes/theme_thememanager.md)
* [`ThemeTokens` 設計規格代幣矩陣](../classes/theme_tokens.md)
* [主題與樣式系統開發手冊](../guides/theme_and_styling_system.md)
