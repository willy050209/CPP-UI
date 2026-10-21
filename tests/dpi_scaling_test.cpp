/**
 * @file dpi_scaling_test.cpp
 * @brief High-DPI 渲染縮放比例與排版等比放大單元測試 (125%, 150%, 200%)
 */

#include <cassert>
#include <cmath>
#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/rendering/LinearArena.hpp"
#include "ui/rendering/DrawOp.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "../src/rendering/backend/SoftwareRenderContext.hpp"

namespace {

COMPAT_CONSTEXPR_14 bool ColorsMatch(uint32_t argb1, uint32_t argb2) noexcept {
    return argb1 == argb2;
}

} // anonymous namespace

int main() {
    using namespace cppui::core;
    using namespace cppui::rendering;

    compat::println("==========================================================");
    compat::println(" [CPPUI Test] High-DPI Scaling & Rendering Pipeline Test  ");
    compat::println("==========================================================");

    // -------------------------------------------------------------------------
    // [Test 1] IRenderTarget & SoftwareRenderTarget DPI Scale Configuration
    // -------------------------------------------------------------------------
    compat::println("[Test 1] IRenderTarget & SoftwareRenderTarget DPI Scale Configuration...");
    {
        SoftwareRenderTarget target;
        assert(target.GetDpiScale() == 1.0f);

        target.SetDpiScale(1.5f);
        assert(std::abs(target.GetDpiScale() - 1.5f) < 0.0001f);

        target.SetDpiScale(2.0f);
        assert(std::abs(target.GetDpiScale() - 2.0f) < 0.0001f);

        // 防禦性邊界：非正值自動安全回退為 1.0f
        target.SetDpiScale(0.0f);
        assert(target.GetDpiScale() == 1.0f);

        target.SetDpiScale(-3.5f);
        assert(target.GetDpiScale() == 1.0f);

        compat::println("  [PASS] Target DPI configuration and defensive fallback verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 2] High-DPI 1.5x (150% DPI) Scaling Projection
    // -------------------------------------------------------------------------
    compat::println("[Test 2] High-DPI 1.5x (150% DPI) Scaling Projection...");
    {
        // 物理 Framebuffer 大小為 150x150
        SoftwareRenderTarget target;
        target.Resize(150, 150);
        target.Clear(Colors::Black);
        target.SetDpiScale(1.5f);

        // 繪製 100x100 DIP 的紅色矩形
        DisplayList dl;
        DrawingContext dc(dl);
        dc.DrawRectangle(Rect(0, 0, 100, 100), Colors::Red);

        DisplayListRenderer renderer;
        RenderStats stats = renderer.Render(dl, &target);
        assert(stats.renderedOps == 1);

        const uint32_t* pixels = target.GetPixelBuffer();
        uint32_t redArgb = Colors::Red.ToArgb();

        // 驗證左上角 (0, 0)
        assert(ColorsMatch(pixels[0], redArgb));

        // 驗證中心點 (75, 75)
        assert(ColorsMatch(pixels[75 * 150 + 75], redArgb));

        // 驗證 1.0x 範圍之外、但在 1.5x 縮放範圍之內的物理像素 (140, 140)
        // 若未縮放 (100x100 像素)，(140, 140) 會是背景黑色
        // 在 1.5x 縮放後 (150x150 像素)，(140, 140) 必須是紅色！
        assert(ColorsMatch(pixels[140 * 150 + 140], redArgb));
        assert(ColorsMatch(pixels[149 * 150 + 149], redArgb));

        compat::println("  [PASS] 100x100 DIP rectangle correctly projected to 150x150 physical pixels.");
    }

    // -------------------------------------------------------------------------
    // [Test 3] High-DPI 2.0x (200% DPI) Scaling & Precision Boundaries
    // -------------------------------------------------------------------------
    compat::println("[Test 3] High-DPI 2.0x (200% DPI) Scaling & Precision Boundaries...");
    {
        // 物理 Framebuffer 大小為 200x200
        SoftwareRenderTarget target;
        target.Resize(200, 200);
        target.Clear(Colors::Black);
        target.SetDpiScale(2.0f);

        // 在邏輯坐標 (10, 10) 繪製 80x80 DIP 的藍色矩形 (邏輯範圍: x=[10..90], y=[10..90])
        // 經 2.0x 投影後，物理範圍應為 x=[20..180], y=[20..180]
        DisplayList dl;
        DrawingContext dc(dl);
        dc.DrawRectangle(Rect(10, 10, 80, 80), Colors::Blue);

        DisplayListRenderer renderer;
        renderer.Render(dl, &target);

        const uint32_t* pixels = target.GetPixelBuffer();
        uint32_t blueArgb = Colors::Blue.ToArgb();
        uint32_t blackArgb = Colors::Black.ToArgb();

        // 邊界外側檢查 (15, 15) -> 物理應為黑色
        assert(ColorsMatch(pixels[15 * 200 + 15], blackArgb));

        // 邊界內側檢查 (25, 25) -> 物理應為藍色
        assert(ColorsMatch(pixels[25 * 200 + 25], blueArgb));

        // 中心點 (100, 100) -> 物理應為藍色
        assert(ColorsMatch(pixels[100 * 200 + 100], blueArgb));

        // 右下角邊界內側 (175, 175) -> 物理應為藍色
        assert(ColorsMatch(pixels[175 * 200 + 175], blueArgb));

        // 右下角邊界外側 (185, 185) -> 物理應為黑色
        assert(ColorsMatch(pixels[185 * 200 + 185], blackArgb));

        compat::println("  [PASS] 2.0x High-DPI boundary alignment and precision verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 4] High-DPI Rounded Rectangle & Line Scaling
    // -------------------------------------------------------------------------
    compat::println("[Test 4] High-DPI Rounded Rectangle & Line Scaling...");
    {
        SoftwareRenderTarget target;
        target.Resize(200, 200);
        target.Clear(Colors::Black);
        target.SetDpiScale(2.0f);

        DisplayList dl;
        DrawingContext dc(dl);
        // 圓角矩形 50x50 DIP, 半徑 10 DIP
        dc.DrawRoundedRectangle(Rect(5, 5, 50, 50), 10.0f, Colors::Green);
        // 水平直線: y=80, x=[0..50], 寬度=2 DIP
        dc.DrawLine(Point(0, 80), Point(50, 80), Colors::Yellow, 2.0f);

        DisplayListRenderer renderer;
        RenderStats stats = renderer.Render(dl, &target);
        assert(stats.renderedOps == 2);

        const uint32_t* pixels = target.GetPixelBuffer();
        uint32_t greenArgb = Colors::Green.ToArgb();
        uint32_t yellowArgb = Colors::Yellow.ToArgb();

        // 圓角矩形中心點: DIP (30, 30) -> 物理 (60, 60)
        assert(ColorsMatch(pixels[60 * 200 + 60], greenArgb));

        // 水平線: DIP y=80, x=25 -> 物理 y=160, x=50
        assert(ColorsMatch(pixels[160 * 200 + 50], yellowArgb));

        compat::println("  [PASS] Rounded rectangle and line operations correctly scaled.");
    }

    compat::println("All High-DPI Scaling & Rendering tests passed successfully!");
    return 0;
}
