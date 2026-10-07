#include <cassert>
#include "compat/compat.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"
#include "../src/rendering/backend/SkiaRenderContext.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"

int main() {
    using namespace cppui::core;
    using namespace cppui::rendering;

    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Skia Rendering Backend & Dual-Engine Test   ");
    compat::println("==========================================================");

    // 1. 初始化 SkiaRenderContext
    SkiaRenderContext ctx;
    bool initOk = ctx.Init(nullptr, 400, 300);
    assert(initOk);
    compat::println("[PASS] SkiaRenderContext Init (Skia active: {})", ctx.IsSkiaActive());

    // 2. 測試 BeginFrame 與 RenderTarget 介面
    IRenderTarget* target = ctx.BeginFrame();
    assert(target != nullptr);
    assert(target->GetWidth() == 400);
    assert(target->GetHeight() == 300);

    target->Clear(Color(0, 0, 128, 255));
    target->FillRect(Rect(20.0f, 20.0f, 100.0f, 60.0f), Color(255, 215, 0, 255));

    ctx.EndFrame();
    compat::println("[PASS] BeginFrame, Clear, FillRect & EndFrame");

    // 3. 測試 DisplayList 重播至 Skia/Fallback RenderTarget
    DisplayList dl;
    DrawingContext dc(dl);
    dc.PushClip(Rect(0, 0, 400, 300));
    dc.DrawRectangle(Rect(10, 10, 50, 50), Colors::Red);
    dc.DrawRoundedRectangle(Rect(70, 10, 50, 50), 8.0f, Colors::Green);
    dc.DrawLine(Point(0, 0), Point(100, 100), Colors::White, 2.0f);
    dc.PopClip();

    target = ctx.BeginFrame();
    assert(target != nullptr);
    DisplayListRenderer renderer;
    RenderStats stats = renderer.Render(dl, target);
    assert(stats.renderedOps >= 3);
    ctx.EndFrame();
    compat::println("[PASS] DisplayListRenderer playback on RenderTarget (RenderedOps: {})", stats.renderedOps);

    // 4. 測試 Resize 幾何調整
    ctx.Resize(600, 450);
    target = ctx.BeginFrame();
    assert(target != nullptr);
    assert(target->GetWidth() == 600);
    assert(target->GetHeight() == 450);
    ctx.EndFrame();
    compat::println("[PASS] Resize to 600x450");

    // 5. 釋放環境
    ctx.Destroy();
    compat::println("[PASS] SkiaRenderContext Destroy");

    compat::println("[CPPUI Test] All Skia backend tests passed successfully!");
    return 0;
}
