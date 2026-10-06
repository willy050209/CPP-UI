#include <iostream>
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

    std::cout << "========================================================" << std::endl;
    std::cout << " CPPUI Milestone 1: Platform Window & Render Backend Demo" << std::endl;
    std::cout << "========================================================" << std::endl;

    // 1. 建立平台原生視窗
    WindowDesc desc;
    desc.title = "CPPUI M1 - Window & Render Backend Demo";
    desc.width = 800;
    desc.height = 600;
    desc.resizable = true;

    auto window = PlatformWindowFactory::Create(desc);
    if (!window) {
        std::cerr << "Failed to create platform window!" << std::endl;
        return -1;
    }

    // 2. 建立渲染後端 Context
    auto renderContext = std::make_unique<SkiaRenderContext>();
    Size fbSize = window->GetFramebufferSize();
    if (!renderContext->Init(window->GetNativeHandle(), static_cast<uint32_t>(fbSize.width), static_cast<uint32_t>(fbSize.height))) {
        std::cerr << "Failed to initialize render context!" << std::endl;
        return -1;
    }

    // 狀態追蹤
    Point mousePos(400.0f, 300.0f);
    bool isMouseDown = false;
    uint32_t currentWidth = static_cast<uint32_t>(fbSize.width);
    uint32_t currentHeight = static_cast<uint32_t>(fbSize.height);

    // 3. 設定事件監聽回呼
    window->SetEventCallback([&](const RawInputEvent& e) {
        switch (e.type) {
            case RawEventType::MouseMove:
                mousePos = e.mouse.position;
                break;
            case RawEventType::MouseDown:
                isMouseDown = true;
                std::cout << "[Event] MouseDown at (" << e.mouse.position.x << ", " << e.mouse.position.y << ")" << std::endl;
                break;
            case RawEventType::MouseUp:
                isMouseDown = false;
                std::cout << "[Event] MouseUp at (" << e.mouse.position.x << ", " << e.mouse.position.y << ")" << std::endl;
                break;
            case RawEventType::WindowResize:
                currentWidth = e.window.width;
                currentHeight = e.window.height;
                renderContext->Resize(currentWidth, currentHeight);
                std::cout << "[Event] Window resized to " << currentWidth << "x" << currentHeight
                          << " (DPI scale: " << e.window.dpiScale << ")" << std::endl;
                break;
            case RawEventType::KeyDown:
                if (e.key.virtualKey == 27) { // VK_ESCAPE
                    std::cout << "[Event] ESC pressed -> Closing window" << std::endl;
                    window->Close();
                }
                break;
            case RawEventType::WindowClose:
                std::cout << "[Event] Window close requested" << std::endl;
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
        if (std::chrono::duration<float>(now - lastFpsTime).count() >= 1.0f) {
            float fps = static_cast<float>(frameCount) / std::chrono::duration<float>(now - lastFpsTime).count();
            std::cout << "[FPS] Current frame rate: " << fps << std::endl;
            frameCount = 0;
            lastFpsTime = now;
        }

        // 開始渲染
        IRenderTarget* target = renderContext->BeginFrame();
        if (target) {
            // 背景清屏 (深灰藍)
            target->Clear(Color(25, 28, 36, 255));

            // 繪製頂部狀態列色塊 (SteelBlue)
            target->FillRect(Rect(0, 0, static_cast<float>(currentWidth), 40.0f), Colors::SteelBlue);

            // 繪製動態跳動色塊 (以 Sin 函數驅動)
            float animOffset = std::sin(elapsedSeconds * 3.0f) * 100.0f;
            Rect animRect(100.0f, 150.0f + animOffset, 120.0f, 80.0f);
            target->FillRect(animRect, Color(220, 80, 50, 255));

            // 繪製中心展示面板 (白色/卡片樣式)
            float cardX = (static_cast<float>(currentWidth) - 300.0f) * 0.5f;
            float cardY = (static_cast<float>(currentHeight) - 200.0f) * 0.5f;
            target->FillRect(Rect(cardX, cardY, 300.0f, 200.0f), Color(45, 52, 64, 255));

            // 繪製跟隨滑鼠的游標反饋矩形
            float cursorSize = isMouseDown ? 36.0f : 24.0f;
            Color cursorColor = isMouseDown ? Color(255, 60, 60, 255) : Color(60, 220, 100, 255);
            Rect cursorRect(mousePos.x - cursorSize * 0.5f, mousePos.y - cursorSize * 0.5f, cursorSize, cursorSize);
            target->FillRect(cursorRect, cursorColor);
        }

        renderContext->EndFrame();

        // 連續請求下一幀重繪以維持流暢動畫
        loop.RequestRedraw();
    });

    std::cout << "Starting RunLoop. Press ESC or close the window to exit." << std::endl;
    loop.Run();

    renderContext->Destroy();
    std::cout << "RunLoop exited cleanly. Demo finished." << std::endl;
    return 0;
}
