#include <cassert>
#include <vector>
#include "compat/compat.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/PlatformWindowGeneric.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"
#include "../src/rendering/backend/SoftwareRenderContext.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include "ui/text/FontManager.hpp"

int main() {
    using namespace cppui;
    using namespace cppui::pal;
    using namespace cppui::core;
    using namespace cppui::rendering;
    using namespace cppui::text;

    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Cross-Platform Generic PAL & Backend Test  ");
    compat::println("==========================================================");

    // 1. 測試 PlatformWindowFactory::CreateGeneric 實例化
    WindowDesc desc;
    desc.title = "Generic PAL Test Window";
    desc.width = 640;
    desc.height = 480;

    auto window = PlatformWindowFactory::CreateGeneric(desc);
    assert(window != nullptr);
    assert(!window->IsClosed());
    assert(window->GetSize().width == 640.0f);
    assert(window->GetSize().height == 480.0f);
    assert(window->GetDpiScale() == 1.0f);
    compat::println("[PASS] PlatformWindowFactory::CreateGeneric creation & dimensions");

    // 2. 測試尺寸與 DPI 動態調整
    auto* genericWin = dynamic_cast<PlatformWindowGeneric*>(window.get());
    assert(genericWin != nullptr);

    bool resizeFired = false;
    window->SetEventCallback([&](const RawInputEvent& e) {
        if (e.type == RawEventType::WindowResize) {
            resizeFired = true;
            assert(e.window.width == 800);
            assert(e.window.height == 600);
        }
    });

    window->SetSize(800, 600);
    window->PollEvents();
    assert(resizeFired);
    assert(window->GetSize().width == 800.0f);
    assert(window->GetSize().height == 600.0f);
    compat::println("[PASS] SetSize & WindowResize event callback dispatch");

    // 3. 測試事件注入 (InjectEvent) 與輪詢 (PollEvents)
    bool mouseFired = false;
    bool keyFired = false;

    window->SetEventCallback([&](const RawInputEvent& e) {
        if (e.type == RawEventType::MouseDown) {
            mouseFired = true;
            assert(e.mouse.position.x == 120.0f);
            assert(e.mouse.position.y == 240.0f);
            assert(e.mouse.button == RawMouseButton::Left);
        } else if (e.type == RawEventType::KeyDown) {
            keyFired = true;
            assert(e.key.virtualKey == 65); // 'A'
        }
    });

    Point pt(120.0f, 240.0f);
    genericWin->InjectEvent(RawInputEvent::MakeMouseDown(pt, pt, RawMouseButton::Left, 1, KeyModifiers::None, 100));
    genericWin->InjectEvent(RawInputEvent::MakeKeyDown(65, 0, KeyModifiers::None, 110));

    window->PollEvents();
    assert(mouseFired);
    assert(keyFired);
    compat::println("[PASS] InjectEvent & PollEvents mouse and key dispatch");

    // 4. 測試 PresentFramebuffer 與像素寫入
    std::vector<uint32_t> testPixels(800 * 600, 0xFF112233); // 測試特定像素值
    window->PresentFramebuffer(testPixels.data(), 800, 600);

    auto fbPixels = genericWin->GetFramebufferPixels();
    assert(fbPixels.size() == 800 * 600);
    assert(fbPixels[0] == 0xFF112233);
    assert(fbPixels[800 * 600 - 1] == 0xFF112233);
    compat::println("[PASS] PresentFramebuffer & GetFramebufferPixels pixel retention");

    // 5. 測試純 C++ 多行文字排版備援與換行
    FontDescriptor font("Segoe UI", 16.0f);
    compat::string_view multilineText = "Hello\nWorld CPPUI Cross-Platform";
    TextLayout layout = TextLayoutEngine::Get().CreateLayout(
        multilineText, font, 200.0f, 1000.0f,
        TextAlignment::Left, TextWrapping::WordWrap, TextTrimming::None
    );

    assert(layout.GetLineCount() >= 2);
    assert(layout.GetSize().height > 16.0f);
    compat::println("[PASS] Cross-platform multiline text layout calculation (LineCount: {})", layout.GetLineCount());

    // 6. 測試命中測試 (HitTestPoint / HitTestTextPosition)
    TextHitTestResult hit = layout.HitTestPoint(Point(10.0f, 5.0f));
    assert(hit.cursorRect.width > 0.0f);
    assert(hit.cursorRect.height > 0.0f);

    Rect cursor = layout.HitTestTextPosition(0);
    assert(cursor.width > 0.0f);
    assert(cursor.height > 0.0f);
    compat::println("[PASS] HitTestPoint & HitTestTextPosition fallback geometry");

    // 7. 測試內建純 C++ 軟體字型光柵化器 (8x8 fallback rasterizer)
    SoftwareRenderTarget rt;
    rt.Resize(100, 50);
    rt.Clear(Colors::Black);

    rt.DrawText("ABC", Point(5, 5), "Default", 8.0f, 400, 0, Colors::White);
    const uint32_t* buf = rt.GetPixelBuffer();
    bool foundWhitePixel = false;
    for (size_t i = 0; i < 100 * 50; ++i) {
        if ((buf[i] & 0x00FFFFFF) == 0x00FFFFFF) {
            foundWhitePixel = true;
            break;
        }
    }
    assert(foundWhitePixel);
    compat::println("[PASS] SoftwareRenderTarget 8x8 fallback font glyph rasterization");

    // 8. 視窗關閉生命週期
    window->Close();
    assert(window->IsClosed());
    compat::println("[PASS] PlatformWindowGeneric Close lifecycle");

    compat::println("[CPPUI Test] All cross-platform generic PAL tests passed successfully!");
    return 0;
}
