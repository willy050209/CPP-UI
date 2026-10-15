/**
 * @file pal_x11_test.cpp
 * @brief Linux X11 實體視窗後端 (PlatformWindowX11) 生命週期、繪製與契約邊界單元測試
 */

#include <cassert>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstdlib>
#include "compat/Compat.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"

#if defined(CPPUI_HAS_X11) && CPPUI_HAS_X11
#include "../src/pal/x11/PlatformWindowX11.hpp"
#endif

int main() {
    using namespace cppui;
    using namespace cppui::pal;
    using namespace cppui::core;

    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Linux X11 Native Window (PlatformWindowX11) ");
    compat::println("==========================================================");

    // 1. 測試參數契約邊界防護 (Fail-Fast: 寬高為 0 必須擲出 std::invalid_argument)
    bool zeroWidthCaught = false;
    try {
        WindowDesc invalidDesc;
        invalidDesc.width = 0;
        invalidDesc.height = 400;
        auto unused = PlatformWindowFactory::CreateX11(invalidDesc);
        (void)unused;
    } catch (const std::invalid_argument&) {
        zeroWidthCaught = true;
    }
    assert(zeroWidthCaught);
    compat::println("[PASS] Fail-fast: zero width caught via std::invalid_argument");

    bool zeroHeightCaught = false;
    try {
        WindowDesc invalidDesc;
        invalidDesc.width = 600;
        invalidDesc.height = 0;
        auto unused = PlatformWindowFactory::CreateX11(invalidDesc);
        (void)unused;
    } catch (const std::invalid_argument&) {
        zeroHeightCaught = true;
    }
    assert(zeroHeightCaught);
    compat::println("[PASS] Fail-fast: zero height caught via std::invalid_argument");

#if defined(CPPUI_HAS_X11) && CPPUI_HAS_X11
    const char* displayEnv = std::getenv("DISPLAY");
    if (displayEnv && displayEnv[0] != '\0') {
        compat::println("[INFO] DISPLAY is set to: ", displayEnv, ", testing live X11 window...");

        // 2. 測試建立 X11 實體視窗
        WindowDesc desc;
        desc.title = "CPPUI X11 PAL Unit Test";
        desc.width = 640;
        desc.height = 480;
        desc.visible = false; // 測試階段以不可見方式執行，避免干擾畫面

        auto window = PlatformWindowFactory::CreateX11(desc);
        assert(window != nullptr);
        assert(!window->IsClosed());
        assert(window->GetSize().width == 640.0f);
        assert(window->GetSize().height == 480.0f);
        assert(window->GetDpiScale() >= 1.0f);
        assert(window->GetNativeHandle() != nullptr);
        compat::println("[PASS] PlatformWindowX11 creation, dimensions & NativeHandle");

        // 3. 測試動態調整視窗尺寸與標題
        window->SetSize(800, 600);
        assert(window->GetSize().width == 800.0f);
        assert(window->GetSize().height == 600.0f);

        window->SetTitle("CPPUI X11 Unit Test - Resized");
        compat::println("[PASS] PlatformWindowX11 SetSize & SetTitle");

        // 4. 測試雙緩衝像素繪製 (PresentFramebuffer)
        std::vector<uint32_t> testPixels(800 * 600, 0xFF336699);
        window->PresentFramebuffer(testPixels, 800, 600);
        compat::println("[PASS] PlatformWindowX11 PresentFramebuffer 800x600");

        // 5. 測試事件輪詢與喚醒機制
        window->WakeUp();
        window->PollEvents();
        compat::println("[PASS] PlatformWindowX11 WakeUp & PollEvents");

        // 6. 測試視窗關閉生命週期
        window->Close();
        assert(window->IsClosed());
        compat::println("[PASS] PlatformWindowX11 Close lifecycle");

        // 7. 測試工廠自動決策：具備 DISPLAY 時 PlatformWindowFactory::Create 應成功返回
        auto autoWindow = PlatformWindowFactory::Create(desc);
        assert(autoWindow != nullptr);
        assert(!autoWindow->IsClosed());
        autoWindow->Close();
        compat::println("[PASS] PlatformWindowFactory::Create auto-detection with DISPLAY");
    } else {
        compat::println("[INFO] DISPLAY is not set. Testing fallback to std::runtime_error for CreateX11...");
        bool runtimeErrorCaught = false;
        try {
            WindowDesc desc;
            desc.width = 400;
            desc.height = 300;
            auto unused = PlatformWindowFactory::CreateX11(desc);
            (void)unused;
        } catch (const std::runtime_error&) {
            runtimeErrorCaught = true;
        }
        assert(runtimeErrorCaught);
        compat::println("[PASS] PlatformWindowFactory::CreateX11 threw std::runtime_error without DISPLAY");
    }
#else
    compat::println("[INFO] CPPUI_HAS_X11 is not defined in this build.");
#endif

    compat::println("All X11 Platform Window unit tests passed successfully!");
    return 0;
}
