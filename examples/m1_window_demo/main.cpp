#include <chrono>
#include <cmath>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/RunLoop.hpp"
#include "../../src/rendering/backend/SkiaRenderContext.hpp"

int main() {
    using namespace cppui::core;
    using namespace cppui::pal;
    using namespace cppui::rendering;

    compat::println("========================================================");
    compat::println(" CPPUI Milestone 1: Platform Window & Render Backend Demo");
    compat::println("========================================================");

    // 1. 建立平台原生視窗
    WindowDesc desc;
    desc.title = "CPPUI M1 - Window & Render Backend Demo";
    desc.width = 800;
    desc.height = 600;
    desc.resizable = true;

    auto window = PlatformWindowFactory::Create(desc);
    if (!window) {
        compat::println(stderr, "Failed to create platform window!");
        return -1;
    }

    // 2. 建立渲染後端 Context
    auto renderContext = std::make_unique<SkiaRenderContext>();
    Size fbSize = window->GetFramebufferSize();
    if (!renderContext->Init(window->GetNativeHandle(), static_cast<uint32_t>(fbSize.width), static_cast<uint32_t>(fbSize.height))) {
        compat::println(stderr, "Failed to initialize render context!");
        return -1;
    }

    // 狀態追蹤
    float dpiScale = window->GetDpiScale();
    Point mouseLogicalPos(400.0f, 300.0f);
    Point mousePhysicalPos(400.0f * dpiScale, 300.0f * dpiScale);
    bool isMouseDown = false;
    uint32_t currentWidth = static_cast<uint32_t>(fbSize.width);
    uint32_t currentHeight = static_cast<uint32_t>(fbSize.height);

    // 3. 設定事件監聽回呼
    window->SetEventCallback([&](const RawInputEvent& e) {
        switch (e.type) {
            case RawEventType::MouseMove:
                mouseLogicalPos = e.mouse.position;
                mousePhysicalPos = e.mouse.physicalPosition;
                break;
            case RawEventType::MouseDown:
                isMouseDown = true;
                mouseLogicalPos = e.mouse.position;
                mousePhysicalPos = e.mouse.physicalPosition;
                compat::println("[Event] MouseDown at Logical ({:.1f}, {:.1f}), Physical ({:.1f}, {:.1f})",
                                mouseLogicalPos.x, mouseLogicalPos.y, mousePhysicalPos.x, mousePhysicalPos.y);
                break;
            case RawEventType::MouseUp:
                isMouseDown = false;
                mouseLogicalPos = e.mouse.position;
                mousePhysicalPos = e.mouse.physicalPosition;
                compat::println("[Event] MouseUp at Logical ({:.1f}, {:.1f}), Physical ({:.1f}, {:.1f})",
                                mouseLogicalPos.x, mouseLogicalPos.y, mousePhysicalPos.x, mousePhysicalPos.y);
                break;
            case RawEventType::WindowResize:
                currentWidth = e.window.width;
                currentHeight = e.window.height;
                dpiScale = e.window.dpiScale;
                renderContext->Resize(currentWidth, currentHeight);
                compat::println("[Event] Window resized to {}x{} (DPI scale: {:.2f})",
                                currentWidth, currentHeight, dpiScale);
                break;
            case RawEventType::KeyDown:
                if (e.key.virtualKey == 27) { // VK_ESCAPE
                    compat::println("[Event] ESC pressed -> Closing window");
                    window->Close();
                }
                break;
            case RawEventType::WindowClose:
                compat::println("[Event] Window close requested");
                break;
            default:
                break;
        }
    });

    // 4. 建立並啟動 RunLoop
    RunLoop loop;
    loop.AttachWindow(window);

    auto startTime = std::chrono::steady_clock::now();
    uint64_t frameCount = 0;
    auto lastFpsTime = startTime;

    loop.SetRenderCallback([&]() {
        auto now = std::chrono::steady_clock::now();
        float elapsedSeconds = std::chrono::duration<float>(now - startTime).count();

        // 幀率計算
        frameCount++;
        float fpsElapsed = std::chrono::duration<float>(now - lastFpsTime).count();
        if (fpsElapsed >= 1.0f) {
            float fps = static_cast<float>(frameCount) / fpsElapsed;
            compat::println("[FPS] Current frame rate: {:.2f}", fps);
            frameCount = 0;
            lastFpsTime = now;
        }

        // 開始渲染 (在物理 Framebuffer 上作圖)
        IRenderTarget* target = renderContext->BeginFrame();
        if (target) {
            // 背景清屏 (深灰藍)
            target->Clear(Color(25, 28, 36, 255));

            // 繪製頂部狀態列色塊 (SteelBlue，高度依 DPI 自適應)
            float headerHeight = 40.0f * dpiScale;
            target->FillRect(Rect(0, 0, static_cast<float>(currentWidth), headerHeight), Colors::SteelBlue);

            // 繪製動態跳動色塊 (以 Sin 函數驅動)
            float animOffset = std::sin(elapsedSeconds * 3.0f) * (100.0f * dpiScale);
            Rect animRect(100.0f * dpiScale, (150.0f * dpiScale) + animOffset, 120.0f * dpiScale, 80.0f * dpiScale);
            target->FillRect(animRect, Color(220, 80, 50, 255));

            // 繪製中心展示面板 (白色/卡片樣式)
            float cardW = 300.0f * dpiScale;
            float cardH = 200.0f * dpiScale;
            float cardX = (static_cast<float>(currentWidth) - cardW) * 0.5f;
            float cardY = (static_cast<float>(currentHeight) - cardH) * 0.5f;
            target->FillRect(Rect(cardX, cardY, cardW, cardH), Color(45, 52, 64, 255));

            // 繪製跟隨滑鼠的游標反饋矩形 (使用物理像素座標，使方塊幾何中心 100% 精準對齊滑鼠)
            float cursorSize = (isMouseDown ? 36.0f : 24.0f) * dpiScale;
            Color cursorColor = isMouseDown ? Color(255, 60, 60, 255) : Color(60, 220, 100, 255);
            Rect cursorRect(mousePhysicalPos.x - cursorSize * 0.5f,
                            mousePhysicalPos.y - cursorSize * 0.5f,
                            cursorSize, cursorSize);
            target->FillRect(cursorRect, cursorColor);
        }

        renderContext->EndFrame();

        // 連續請求下一幀重繪以維持流暢動畫
        loop.RequestRedraw();
    });

    compat::println("Starting RunLoop. Press ESC or close the window to exit.");
    loop.Run();

    renderContext->Destroy();
    compat::println("RunLoop exited cleanly. Demo finished.");
    return 0;
}
