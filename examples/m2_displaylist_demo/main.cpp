#include <chrono>
#include <cmath>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/pal/RunLoop.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "../../src/rendering/backend/SkiaRenderContext.hpp"

int main() {
    using namespace cppui::core;
    using namespace cppui::pal;
    using namespace cppui::rendering;

    compat::println("================================================================");
    compat::println(" CPPUI Milestone 2: DisplayList & Canvas Command Recording Demo");
    compat::println("================================================================");

    // 1. 建立視窗
    WindowDesc desc;
    desc.title = "CPPUI M2 - DisplayList Command Stream Demo";
    desc.width = 960;
    desc.height = 640;
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

    // 3. 建立 DisplayList、DrawingContext 與 Renderer
    DisplayList displayList;
    DrawingContext dc(displayList);
    DisplayListRenderer renderer;

    // 狀態追蹤
    float dpiScale = window->GetDpiScale();
    Point mousePos(400.0f * dpiScale, 300.0f * dpiScale);
    bool isMouseDown = false;
    uint32_t currentWidth = static_cast<uint32_t>(fbSize.width);
    uint32_t currentHeight = static_cast<uint32_t>(fbSize.height);

    window->SetEventCallback([&](const RawInputEvent& e) {
        switch (e.type) {
            case RawEventType::MouseMove:
                mousePos = e.mouse.physicalPosition;
                break;
            case RawEventType::MouseDown:
                isMouseDown = true;
                mousePos = e.mouse.physicalPosition;
                break;
            case RawEventType::MouseUp:
                isMouseDown = false;
                mousePos = e.mouse.physicalPosition;
                break;
            case RawEventType::WindowResize:
                currentWidth = e.window.width;
                currentHeight = e.window.height;
                dpiScale = e.window.dpiScale;
                renderContext->Resize(currentWidth, currentHeight);
                compat::println("[Event] Resized to {}x{} (DPI scale: {:.2f})", currentWidth, currentHeight, dpiScale);
                break;
            case RawEventType::KeyDown:
                if (e.key.virtualKey == 27) { // VK_ESCAPE
                    compat::println("[Event] ESC pressed -> Closing window");
                    window->Close();
                }
                break;
            case RawEventType::WindowClose:
                compat::println("[Event] Window close requested");
                window->Close();
                break;
            default:
                break;
        }
    });

    // 4. 建立事件循環
    RunLoop loop;
    loop.AttachWindow(window);

    auto startTime = std::chrono::steady_clock::now();
    uint64_t frameCount = 0;
    auto lastFpsTime = startTime;

    loop.SetRenderCallback([&]() {
        auto now = std::chrono::steady_clock::now();
        float elapsedSeconds = std::chrono::duration<float>(now - startTime).count();

        // 計算 FPS
        frameCount++;
        float fpsElapsed = std::chrono::duration<float>(now - lastFpsTime).count();
        if (fpsElapsed >= 1.0f) {
            float fps = static_cast<float>(frameCount) / fpsElapsed;
            compat::println("[FPS] {:.2f} | DisplayList Ops: {} | Memory: {:.1f} KB",
                            fps, displayList.GetOpCount(), static_cast<double>(displayList.GetTotalBytes()) / 1024.0);
            frameCount = 0;
            lastFpsTime = now;
        }

        // -------------------------------------------------------------
        // 階段一：純量命令錄製 (Recording Phase) - 完全不觸碰底層 GPU
        // -------------------------------------------------------------
        displayList.Clear();

        // 1. 全螢幕深色背景 (Dark Slate)
        dc.DrawRectangle(Rect(0, 0, static_cast<float>(currentWidth), static_cast<float>(currentHeight)),
                         Color(18, 20, 26, 255));

        // 2. 頂部導覽列 (帶底部線段分割)
        float navHeight = 48.0f * dpiScale;
        dc.DrawRectangle(Rect(0, 0, static_cast<float>(currentWidth), navHeight), Color(28, 32, 42, 255));
        dc.DrawLine(Point(0, navHeight), Point(static_cast<float>(currentWidth), navHeight),
                    Color(45, 52, 68, 255), 1.0f * dpiScale);

        // 3. 左側側邊攔圓角卡片
        float sidebarWidth = 220.0f * dpiScale;
        dc.DrawRoundedRectangle(Rect(16.0f * dpiScale, navHeight + 16.0f * dpiScale,
                                     sidebarWidth, static_cast<float>(currentHeight) - navHeight - 32.0f * dpiScale),
                                12.0f * dpiScale, Color(24, 28, 38, 255));

        // 4. 主內容區圓角面板
        float mainX = sidebarWidth + 32.0f * dpiScale;
        float mainY = navHeight + 16.0f * dpiScale;
        float mainW = static_cast<float>(currentWidth) - mainX - 16.0f * dpiScale;
        float mainH = static_cast<float>(currentHeight) - mainY - 32.0f * dpiScale;
        dc.DrawRoundedRectangle(Rect(mainX, mainY, mainW, mainH), 12.0f * dpiScale, Color(30, 35, 48, 255));

        // 5. 局部剪裁視窗示範 (Clip Region + 內部動態動畫)
        float clipBoxX = mainX + 24.0f * dpiScale;
        float clipBoxY = mainY + 24.0f * dpiScale;
        float clipBoxW = 280.0f * dpiScale;
        float clipBoxH = 180.0f * dpiScale;

        // 繪製剪裁視窗底色與邊框
        dc.DrawRectangle(Rect(clipBoxX, clipBoxY, clipBoxW, clipBoxH), Color(12, 14, 18, 255));

        {
            // 使用 RAII AutoClip 將繪圖約束在剪裁視窗內部
            DrawingContext::AutoClip clipGuard(dc, Rect(clipBoxX, clipBoxY, clipBoxW, clipBoxH));

            // 在剪裁區內部繪製動態往復跳動的彩色方塊 (超出邊界處將自動被 Culler / Clip 剔除)
            float animOffset = std::sin(elapsedSeconds * 4.0f) * (180.0f * dpiScale);
            Rect dynamicRect(clipBoxX + (clipBoxW * 0.5f) - (40.0f * dpiScale) + animOffset,
                             clipBoxY + 30.0f * dpiScale,
                             80.0f * dpiScale, 60.0f * dpiScale);
            dc.DrawRoundedRectangle(dynamicRect, 8.0f * dpiScale, Color(230, 75, 60, 255));

            // 繪製輔助水平指示線
            dc.DrawLine(Point(clipBoxX - 50.0f, clipBoxY + 120.0f * dpiScale),
                        Point(clipBoxX + clipBoxW + 50.0f, clipBoxY + 120.0f * dpiScale),
                        Colors::Green, 2.0f * dpiScale);
        }

        // 6. 半透明透明度疊加層 (Opacity Stack 示範)
        {
            DrawingContext::AutoOpacity opacityGuard(dc, 0.65f);
            float hudX = mainX + 330.0f * dpiScale;
            float hudY = mainY + 24.0f * dpiScale;
            dc.DrawRoundedRectangle(Rect(hudX, hudY, 240.0f * dpiScale, 180.0f * dpiScale),
                                    10.0f * dpiScale, Color(70, 130, 180, 255));
        }

        // 7. 跟隨滑鼠的游標反饋矩形 (100% 精準對齊滑鼠中心)
        float cursorSize = (isMouseDown ? 36.0f : 24.0f) * dpiScale;
        Color cursorColor = isMouseDown ? Color(255, 60, 60, 255) : Color(60, 220, 100, 255);
        dc.DrawRoundedRectangle(Rect(mousePos.x - cursorSize * 0.5f,
                                     mousePos.y - cursorSize * 0.5f,
                                     cursorSize, cursorSize),
                                4.0f * dpiScale, cursorColor);

        // -------------------------------------------------------------
        // 階段二：重播與 GPU 呈現 (Playback Phase)
        // -------------------------------------------------------------
        IRenderTarget* target = renderContext->BeginFrame();
        if (target) {
            renderer.Render(displayList, target);
        }
        renderContext->EndFrame();

        loop.RequestRedraw();
    });

    compat::println("Starting RunLoop. Press ESC or close the window to exit.");
    loop.Run();

    renderContext->Destroy();
    compat::println("RunLoop exited cleanly. Demo finished.");
    return 0;
}
