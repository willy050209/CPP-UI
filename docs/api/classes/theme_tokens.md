# 設計規格代幣 (Design Tokens: Color, Typography, Metric)

CPPUI 設計規格代幣定義於 `cppui::theme` 命名空間，採用現代設計系統 (Design Systems) 的 Token 原則，將色彩、排版與幾何間距徹底語意化，為所有內建控制項與使用者自訂介面提供集中式單一事實來源。

---

## 1. 色彩代幣 (`ColorTokens`)

定義於標頭檔 `#include <ui/theme/ColorTokens.hpp>`。

```cpp
namespace cppui {
namespace theme {

struct ColorTokens {
    // --- 視窗與表面 (Window & Surface) ---
    core::Color windowBackground{ 24, 26, 32, 255 };
    core::Color surfaceBackground{ 32, 36, 46, 255 };
    core::Color surfaceBorder{ 50, 56, 70, 255 };
    core::Color cardBackground{ 40, 45, 58, 255 };

    // --- 互動控制項狀態 (Controls: Button, TextBox, CheckBox) ---
    core::Color controlBackground{ 45, 52, 70, 255 };
    core::Color controlHoverBackground{ 60, 75, 105, 255 };
    core::Color controlPressedBackground{ 32, 40, 58, 255 };
    core::Color controlDisabledBackground{ 35, 38, 45, 255 };

    core::Color controlBorder{ 70, 80, 100, 255 };
    core::Color controlHoverBorder{ 95, 110, 135, 255 };
    core::Color controlFocusedBorder{ 0, 160, 255, 255 };
    core::Color controlDisabledBorder{ 50, 55, 65, 255 };

    // --- 品牌與強調節點 (Accent & Brand) ---
    core::Color accentPrimary{ 0, 122, 255, 255 };
    core::Color accentHover{ 20, 140, 255, 255 };
    core::Color accentPressed{ 0, 100, 220, 255 };
    core::Color accentDisabled{ 0, 122, 255, 100 };
    core::Color accentText{ 255, 255, 255, 255 };

    // --- 文字與排版 (Typography & Foregrounds) ---
    core::Color textPrimary{ 255, 255, 255, 255 };
    core::Color textSecondary{ 160, 168, 184, 255 };
    core::Color textDisabled{ 90, 96, 110, 255 };
    core::Color textPlaceholder{ 120, 130, 150, 255 };
    core::Color textOnAccent{ 255, 255, 255, 255 };

    // --- 語意狀態色彩 (Semantic Feedback) ---
    core::Color success{ 40, 167, 69, 255 };
    core::Color warning{ 255, 193, 7, 255 };
    core::Color error{ 220, 53, 69, 255 };
    core::Color info{ 23, 162, 184, 255 };
};

} // namespace theme
} // namespace cppui
```

### 色彩分層職責說明
* **Window & Surface**：頂層視窗畫布背景、對話方塊表面容器底色、邊框色與卡片 (Card) 區塊底色。
* **Control States**：按鈕、文字方塊與核取方塊之常態、滑鼠懸停 (Hover)、點擊按下 (Pressed) 與禁用 (Disabled) 之背景與邊框色。
* **Accent & Brand**：焦點強調、主行動按鈕 (Primary CTA)、高亮元素之核心顏色與專用前景色。
* **Typography**：主內文文字、次要輔助文字、禁用灰階文字、輸入預留位置 (Placeholder) 文字。
* **Semantic Feedback**：表單驗證成功 (`success`)、警示提示 (`warning`)、錯誤與校驗失敗 (`error`)、一般通知資訊 (`info`)。

---

## 2. 排版代幣 (`TypographyTokens`)

定義於標頭檔 `#include <ui/theme/TypographyTokens.hpp>`。

```cpp
namespace cppui {
namespace theme {

struct TypographyTokens {
    std::string fontFamily = "Segoe UI";
    std::string fontFamilyMonospace = "Consolas";

    // 字級大小比例尺 (pt / DIP)
    float fontSizeCaption = 11.0f;
    float fontSizeBody = 14.0f;
    float fontSizeSubtitle = 16.0f;
    float fontSizeTitle = 20.0f;
    float fontSizeHeader = 24.0f;

    // 字重常數
    text::FontWeight fontWeightRegular = text::FontWeight::Regular;
    text::FontWeight fontWeightMedium = text::FontWeight::Medium;
    text::FontWeight fontWeightSemiBold = text::FontWeight::SemiBold;
    text::FontWeight fontWeightBold = text::FontWeight::Bold;
};

} // namespace theme
} // namespace cppui
```

### 階層字級尺度說明
* **`fontSizeCaption` (11.0f)**：圖表說明、狀態列文字、微型標籤。
* **`fontSizeBody` (14.0f)**：應用程式預設內文字級、按鈕標題、輸入框文字。
* **`fontSizeSubtitle` (16.0f)**：區塊副標題、卡片抬頭。
* **`fontSizeTitle` (20.0f)**：頁面主要標題、對話框大標題。
* **`fontSizeHeader` (24.0f)**：應用程式頂部標頭、儀表板核心數值看板。

---

## 3. 幾何與間距代幣 (`MetricTokens`)

定義於標頭檔 `#include <ui/theme/MetricTokens.hpp>`。

```cpp
namespace cppui {
namespace theme {

struct MetricTokens {
    // 圓角尺度 (Corner Radii)
    float cornerRadiusSmall = 3.0f;
    float cornerRadiusMedium = 6.0f;
    float cornerRadiusLarge = 8.0f;
    float cornerRadiusPill = 999.0f;

    // 邊框粗細 (Border Thickness)
    float borderThicknessDefault = 1.0f;
    float borderThicknessFocused = 2.0f;

    // 控制項常用內距 (Paddings)
    core::Thickness paddingButton{ 16.0f, 8.0f, 16.0f, 8.0f };
    core::Thickness paddingInput{ 10.0f, 6.0f, 10.0f, 6.0f };

    // 排版間距階層 (Spacing Scale)
    float spacingXs = 4.0f;
    float spacingSm = 8.0f;
    float spacingMd = 16.0f;
    float spacingLg = 24.0f;
    float spacingXl = 32.0f;
};

} // namespace theme
} // namespace cppui
```

### 幾何尺度說明
* **Corner Radii**：微型圓角 (3px，用於標籤或微型開關)、中型圓角 (6px，預設按鈕與輸入框)、大型圓角 (8px，卡片與面板)、膠囊膠囊型圓角 (999px，Pill 按鈕與標籤)。
* **Border Thickness**：預設靜態邊框 (1px) 與焦點加粗邊框 (2px)。
* **Paddings**：標準按鈕內距 (左右 16px、上下 8px) 與文字輸入框內距 (左右 10px、上下 6px)。
* **Spacing Scale**：排版間距倍數系統，遵循 4px/8px 基準網格系統 (`spacingXs` 4px 至 `spacingXl` 32px)。

---

## 4. 請參閱 (See Also)

* [`Theme` 實體類別手冊](theme_theme.md)
* [`ThemeManager` 單例中樞手冊](theme_thememanager.md)
* [主題與樣式系統開發手冊](../guides/theme_and_styling_system.md)
