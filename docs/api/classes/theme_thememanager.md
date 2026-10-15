# ThemeManager 類別 (cppui::theme::ThemeManager)

`ThemeManager` 是 CPPUI 進程唯一的全域主題中樞 (Singleton Hub)，負責當前主題實體維護、執行期主題動態切換、以及全域主題變更事件廣播。

---

## 1. 元資料 (Metadata)

* **命名空間**：[`cppui::theme`](../namespaces/cppui_theme.md)
* **標頭檔**：`#include <ui/theme/ThemeManager.hpp>`
* **模組**：`CPPUI Theme`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 2. 語法宣告 (Syntax)

```cpp
namespace cppui {
namespace theme {

class ThemeManager {
public:
    using ThemeChangedHandler = std::function<void(const Theme&)>;

    static ThemeManager& Get();

    COMPAT_NODISCARD const Theme& GetCurrentTheme() const noexcept;
    void SetTheme(const Theme& theme);
    void SetThemeMode(ThemeMode mode);

    size_t AddThemeChangedHandler(ThemeChangedHandler handler);
    void RemoveThemeChangedHandler(size_t handlerId);
    void NotifyThemeChanged();

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
};

} // namespace theme
} // namespace cppui
```

---

## 3. 成員方法說明 (Methods)

### `Get()`
取得進程唯一的單例 `ThemeManager` 實例引用。
```cpp
static ThemeManager& Get();
```

### `GetCurrentTheme()`
取得當前全域啟用的主題實例常數引用。
```cpp
COMPAT_NODISCARD const Theme& GetCurrentTheme() const noexcept;
```

### `SetTheme(const Theme& theme)`
設定並套用全域主題實例，並自動向所有已註冊的觀察者廣播變更通知。
```cpp
void SetTheme(const Theme& theme);
```

### `SetThemeMode(ThemeMode mode)`
依據傳入的列舉模式快速建立並切換原廠主題（`Dark`, `Light`, `HighContrast`）。
```cpp
void SetThemeMode(ThemeMode mode);
```

### `AddThemeChangedHandler(ThemeChangedHandler handler)`
註冊主題變更通知回呼函式。
```cpp
size_t AddThemeChangedHandler(ThemeChangedHandler handler);
```
* **參數**：`handler` 為回呼函式（簽名為 `void(const Theme&)`）。
* **傳回值**：該註冊項目之唯一識別碼 (`size_t ID`)，可用於後續取消註冊。
* **例外**：若傳入之 `handler` 為空，立即擲出 `std::invalid_argument`。

### `RemoveThemeChangedHandler(size_t handlerId)`
依據註冊時取得的識別碼移除回呼函式，防止已銷毀物件遭非預期調用。
```cpp
void RemoveThemeChangedHandler(size_t handlerId);
```

### `NotifyThemeChanged()`
主動向所有註冊之監聽器廣播當前主題變更通知。
```cpp
void NotifyThemeChanged();
```

---

## 4. 視窗與控制項自動連動架構

`Form` 視窗元件在建構時會自動呼叫 `AddThemeChangedHandler` 進行註冊：
1. 當 `ThemeManager::SetTheme` 或 `SetThemeMode` 被呼叫時，`ThemeManager` 自動廣播通知。
2. 作用中的所有 `Form` 收到通知後，立即更新視窗背景色。
3. `Form` 遞迴對其視覺子樹呼叫 `control->ApplyTheme(theme)`，同步更新所有未明確覆寫樣式的子控制項。
4. 自動發出 `Invalidate()` 請求，觸發下一幀重繪，實現無縫的即時動態換膚。
5. `Form` 在解構時會自動呼叫 `RemoveThemeChangedHandler`，確保記憶體安全與零懸空指標。

---

## 5. 範例程式碼 (Example)

```cpp
#include "ui/theme/ThemeManager.hpp"
#include "compat/Compat.hpp"

void ThemeSwitchingDemo() {
    using namespace cppui::theme;

    ThemeManager& tm = ThemeManager::Get();

    // 註冊變更回呼
    size_t id = tm.AddThemeChangedHandler([](const Theme& theme) {
        compat::println("[ThemeChanged] 主題已切換，主文字顏色 RGB: ({}, {}, {})",
                        theme.Colors().textPrimary.r,
                        theme.Colors().textPrimary.g,
                        theme.Colors().textPrimary.b);
    });

    // 執行期快速切換至亮色模式
    tm.SetThemeMode(ThemeMode::Light);

    // 執行期快速切換至高對比模式
    tm.SetThemeMode(ThemeMode::HighContrast);

    // 取消註冊
    tm.RemoveThemeChangedHandler(id);
}
```

---

## 6. 請參閱 (See Also)

* [`Theme` 主題實體手冊](theme_theme.md)
* [`ThemeTokens` 設計規格代幣矩陣](theme_tokens.md)
* [`Form` 視窗生命週期手冊](app_form.md)
* [主題與樣式系統開發手冊](../guides/theme_and_styling_system.md)
