/**
 * @file skia_backend_test.cpp
 * @brief Skia 渲染後端生命週期、雙引擎回退、幾何繪製與幀重播單元測試
 */

#include <cassert>
#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"
#include "../src/rendering/backend/SkiaRenderContext.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "PixelParityTestUtils.hpp"

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
    (void)initOk;
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

    // 5. 跨後端像素與幾何一致性契約測試 (Pixel Parity & Contract Test)
    compat::println("[Test 5] Dual-Backend Cross-Rendering Parity (PSNR / SSIM)...");
    {
        constexpr uint32_t kParityW = 200;
        constexpr uint32_t kParityH = 200;

        // 初始化軟體渲染目標
        SoftwareRenderTarget swTarget;
        swTarget.Resize(kParityW, kParityH);
        swTarget.Clear(Colors::Black);

        // 調整 SkiaContext 至相同大小
        ctx.Resize(kParityW, kParityH);
        IRenderTarget* skiaFrameTarget = ctx.BeginFrame();
        assert(skiaFrameTarget != nullptr);
        skiaFrameTarget->Clear(Colors::Black);

        // 構建包含幾何、圓角、線條與裁切之 DisplayList
        DisplayList parityDl;
        DrawingContext parityDc(parityDl);
        parityDc.PushClip(Rect(10.0f, 10.0f, 180.0f, 180.0f));
        parityDc.DrawRectangle(Rect(20.0f, 20.0f, 80.0f, 60.0f), Color(100, 149, 237, 255));
        parityDc.DrawRoundedRectangle(Rect(110.0f, 20.0f, 60.0f, 60.0f), 10.0f, Color(255, 140, 0, 255));
        parityDc.DrawLine(Point(20.0f, 100.0f), Point(180.0f, 160.0f), Color(50, 205, 50, 255), 3.0f);
        parityDc.PopClip();

        // 雙後端重播同一份 DisplayList
        DisplayListRenderer dlRenderer;
        RenderStats skiaStats = dlRenderer.Render(parityDl, skiaFrameTarget);
        RenderStats swStats   = dlRenderer.Render(parityDl, &swTarget);

        assert(skiaStats.totalOps == swStats.totalOps);
        assert(skiaStats.renderedOps == swStats.renderedOps);
        assert(skiaStats.culledOps == swStats.culledOps);

        ctx.EndFrame();

        // 提取雙後端輸出像素緩衝區
        compat::span<const uint32_t> swSpan = swTarget.GetPixelSpan();
        compat::span<const uint32_t> skiaSpan(skiaFrameTarget->GetPixelBuffer(), kParityW * kParityH);

        double minPsnr = ctx.IsSkiaActive() ? 30.0 : 100.0;
        double minSsim = ctx.IsSkiaActive() ? 0.95 : 1.0;

        cppui::test::ParityResult parity = cppui::test::PixelParityTestUtils::VerifyParity(
            swSpan, skiaSpan, kParityW, kParityH, minPsnr, minSsim
        );

        compat::println("  [PASS] Op Count Parity: total={}, rendered={}, culled={}",
                        skiaStats.totalOps, skiaStats.renderedOps, skiaStats.culledOps);
        compat::println("  [PASS] Pixel Parity Metric: MSE={:.4f}, PSNR={:.2f} dB (>= {:.1f}), SSIM={:.4f} (>= {:.2f})",
                        parity.mse, parity.psnr, minPsnr, parity.ssim, minSsim);
        assert(parity.passed);
    }

    // 6. 釋放環境
    ctx.Destroy();
    compat::println("[PASS] SkiaRenderContext Destroy");

    compat::println("[CPPUI Test] All Skia backend tests passed successfully!");
    return 0;
}
