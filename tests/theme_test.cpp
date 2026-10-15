/**
 * @file theme_test.cpp
 * @brief 集中式主題系統 (Design Tokens / ThemeManager) 與控制項繼承單元測試
 */

#include <cassert>
#include <cstdint>
#include <string>
#include <stdexcept>
#include "compat/Compat.hpp"
#include "ui/theme/ThemeTypes.hpp"
#include "ui/theme/ColorTokens.hpp"
#include "ui/theme/TypographyTokens.hpp"
#include "ui/theme/MetricTokens.hpp"
#include "ui/theme/Theme.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/TextBlock.hpp"

int main() {
    using namespace cppui;
    using namespace cppui::theme;
    using namespace cppui::core;
    using namespace cppui::controls;

    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Centralized Theme & Design Tokens Test      ");
    compat::println("==========================================================");

    // -------------------------------------------------------------------------
    // [Test 1] 預設 Dark 與 Light 主題代幣規格驗證
    // -------------------------------------------------------------------------
    compat::println("[Test 1] Theme Default Tokens & Color Modes...");
    {
        Theme dark = Theme::Dark();
        assert(dark.GetMode() == ThemeMode::Dark);
        assert(dark.Colors().windowBackground.r == 24);
        assert(dark.Colors().windowBackground.g == 26);
        assert(dark.Colors().windowBackground.b == 32);
        assert(dark.Colors().accentPrimary.r == 0);
        assert(dark.Colors().accentPrimary.g == 122);
        assert(dark.Colors().accentPrimary.b == 255);
        assert(dark.Metrics().cornerRadiusSmall == 3.0f);
        assert(dark.Typography().fontSizeBody == 14.0f);

        Theme light = Theme::Light();
        assert(light.GetMode() == ThemeMode::Light);
        assert(light.Colors().windowBackground.r == 245);
        assert(light.Colors().windowBackground.g == 246);
        assert(light.Colors().windowBackground.b == 248);
        assert(light.Colors().textPrimary.r == 20);
        assert(light.Colors().textPrimary.g == 24);
        assert(light.Colors().textPrimary.b == 33);

        Theme hc = Theme::HighContrast();
        assert(hc.GetMode() == ThemeMode::HighContrast);
        assert(hc.Colors().windowBackground.r == 0 && hc.Colors().windowBackground.g == 0);
        assert(hc.Colors().accentPrimary.r == 0 && hc.Colors().accentPrimary.g == 255 && hc.Colors().accentPrimary.b == 255);

        compat::println("[PASS] Default Dark/Light/HighContrast token specifications verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 2] 鏈式客製設定與 Fail-Fast 防護
    // -------------------------------------------------------------------------
    compat::println("[Test 2] Fluent Theme Customization & Fail-Fast Validation...");
    {
        Theme custom = Theme::Dark()
            .WithAccent(Color(255, 60, 80, 255), Color(230, 40, 60, 255), Color(200, 20, 40, 255))
            .WithFontFamily("Inter")
            .WithCornerRadius(8.0f);

        assert(custom.Colors().accentPrimary == Color(255, 60, 80, 255));
        assert(custom.Typography().fontFamily == "Inter");
        assert(custom.Metrics().cornerRadiusMedium == 8.0f);
        assert(custom.Metrics().cornerRadiusSmall == 4.0f);
        assert(custom.Metrics().cornerRadiusLarge == 12.0f);

        // Fail-Fast: 空字型名稱必須擲出 std::invalid_argument
        bool caughtEmptyFamily = false;
        try {
            custom.WithFontFamily("");
        } catch (const std::invalid_argument&) {
            caughtEmptyFamily = true;
        }
        assert(caughtEmptyFamily);
        (void)caughtEmptyFamily;
        compat::println("[PASS] Fluent customization and empty family fail-fast verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 3] ThemeManager 單例、狀態切換與事件監聽
    // -------------------------------------------------------------------------
    compat::println("[Test 3] ThemeManager Singleton, Mode Switching & Event Dispatch...");
    {
        ThemeManager& tm = ThemeManager::Get();
        tm.SetThemeMode(ThemeMode::Dark);
        assert(tm.GetCurrentTheme().GetMode() == ThemeMode::Dark);

        // Fail-Fast: 空監聽器必須擲出 std::invalid_argument
        bool caughtNullHandler = false;
        try {
            tm.AddThemeChangedHandler(nullptr);
        } catch (const std::invalid_argument&) {
            caughtNullHandler = true;
        }
        assert(caughtNullHandler);
        (void)caughtNullHandler;

        bool eventFired = false;
        ThemeMode notifiedMode = ThemeMode::Dark;
        size_t subId = tm.AddThemeChangedHandler([&](const Theme& t) {
            eventFired = true;
            notifiedMode = t.GetMode();
        });

        tm.SetThemeMode(ThemeMode::Light);
        assert(eventFired);
        assert(notifiedMode == ThemeMode::Light);
        assert(tm.GetCurrentTheme().GetMode() == ThemeMode::Light);

        // 取消註冊
        eventFired = false;
        tm.RemoveThemeChangedHandler(subId);
        tm.SetThemeMode(ThemeMode::Dark);
        assert(!eventFired); // 不應再收到通知
        compat::println("[PASS] ThemeManager event dispatch & handler lifecycle verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 4] 控制項自動主題繼承與動態切換
    // -------------------------------------------------------------------------
    compat::println("[Test 4] Controls Dynamic Theme Inheritance & Application...");
    {
        ThemeManager& tm = ThemeManager::Get();
        tm.SetThemeMode(ThemeMode::Dark);

        Button btn("Theme Button");
        assert(btn.GetCornerRadius() == tm.GetCurrentTheme().Metrics().cornerRadiusSmall);
        assert(btn.GetFontFamily() == tm.GetCurrentTheme().Typography().fontFamily);
        assert(btn.GetFontSize() == tm.GetCurrentTheme().Typography().fontSizeBody);

        TextBox tb;
        assert(tb.GetPlaceholderColor() == tm.GetCurrentTheme().Colors().textPlaceholder);
        assert(tb.GetFontFamily() == tm.GetCurrentTheme().Typography().fontFamily);

        CheckBox cb("Theme CheckBox");
        assert(cb.GetCornerRadius() == tm.GetCurrentTheme().Metrics().cornerRadiusSmall);

        TextBlock textBlock("Theme TextBlock");
        assert(textBlock.GetFont().GetFamily() == tm.GetCurrentTheme().Typography().fontFamily);

        // 動態切換至 Light 主題
        tm.SetThemeMode(ThemeMode::Light);
        btn.ApplyTheme(tm.GetCurrentTheme());
        tb.ApplyTheme(tm.GetCurrentTheme());
        cb.ApplyTheme(tm.GetCurrentTheme());
        textBlock.ApplyTheme(tm.GetCurrentTheme());

        assert(tb.GetPlaceholderColor() == tm.GetCurrentTheme().Colors().textPlaceholder);

        // 復原為預設暗色
        tm.SetThemeMode(ThemeMode::Dark);
        compat::println("[PASS] Controls theme application verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 5] ThemeManager Pimpl 與深常數性 (Deep Constness) 驗證
    // -------------------------------------------------------------------------
    compat::println("[Test 5] ThemeManager Deep Constness via Pimpl...");
    {
        ThemeManager& tm = ThemeManager::Get();
        tm.SetThemeMode(ThemeMode::Dark);

        const ThemeManager& constTm = tm;
        const Theme& currentTheme = constTm.GetCurrentTheme();
        assert(currentTheme.GetMode() == ThemeMode::Dark);
        compat::println("[PASS] ThemeManager deep constness verified.");
    }

    compat::println("All theme and design token tests passed successfully!");
    return 0;
}
