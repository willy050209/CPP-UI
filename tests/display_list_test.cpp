/**
 * @file display_list_test.cpp
 * @brief LinearArena 記憶體對齊、DisplayList 指令錄製、遍歷與渲染器單元測試
 */

#include <cassert>
#include <chrono>
#include <cstdint>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/rendering/LinearArena.hpp"
#include "ui/rendering/DrawOp.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "ui/text/Font.hpp"
#include "../src/rendering/backend/SoftwareRenderContext.hpp"

int main() {
    using namespace cppui::core;
    using namespace cppui::rendering;

    compat::println("[CPPUI DisplayList & Arena Test] Starting verification...");

    // 1. 測試 LinearArena 8 字節對齊與分配
    LinearArena arena(1024);
    void* p1 = arena.Allocate(3, 8);
    void* p2 = arena.Allocate(5, 8);
    assert(reinterpret_cast<uintptr_t>(p1) % 8 == 0);
    assert(reinterpret_cast<uintptr_t>(p2) % 8 == 0);
    assert(static_cast<uint8_t*>(p2) >= static_cast<uint8_t*>(p1) + 3);
    (void)p1;
    (void)p2;
    compat::println("[PASS] LinearArena 8-byte alignment verification");

    // 2. 測試 DrawingContext 錄製與 DisplayList 訪客遍歷
    DisplayList dl;
    DrawingContext dc(dl);

    dc.DrawRectangle(Rect(10, 10, 100, 50), Colors::Red);
    dc.PushClip(Rect(20, 20, 40, 40));
    dc.DrawRoundedRectangle(Rect(25, 25, 30, 30), 4.0f, Colors::Blue);
    dc.PopClip();
    dc.DrawLine(Point(0, 0), Point(50, 50), Colors::Green, 2.0f);

    assert(dl.GetOpCount() == 5);
    compat::println("[PASS] DrawingContext recorded 5 operations (total bytes: {})", dl.GetTotalBytes());

    size_t inspectedOps = 0;
    dl.ForEach([&](const DrawOpHeader& header, const void*) {
        inspectedOps++;
        assert(header.size >= sizeof(DrawOpHeader));
        (void)header;
    });
    assert(inspectedOps == 5);
    compat::println("[PASS] DisplayList ForEach traversal verified");

    // 3. 測試 DisplayListRenderer 剪裁與渲染分派
    SoftwareRenderTarget target;
    target.Resize(200, 200);
    target.Clear(Colors::Black);

    DisplayListRenderer renderer;
    RenderStats stats = renderer.Render(dl, &target);
    assert(stats.totalOps == 5);
    assert(stats.renderedOps == 3); // DrawRectangle, DrawRoundedRectangle, DrawLine
    compat::println("[PASS] DisplayListRenderer executed rendering with clip stack");

    // 4. 測試幀末 O(1) 重設
    dl.Clear();
    assert(dl.IsEmpty());
    assert(dl.GetOpCount() == 0);
    assert(dl.GetTotalBytes() == 0);
    compat::println("[PASS] DisplayList O(1) Clear / Reset verified");

    // 5. 測試 DrawText 與 DrawTextLayout 於局部剪裁區域之嚴格限制 (邊界外無像素污染)
    {
        DisplayList textDl;
        DrawingContext textDc(textDl);

        SoftwareRenderTarget textTarget;
        textTarget.Resize(200, 200);
        textTarget.Clear(Colors::Black);

        // 僅允許 (50, 50, 100, 100) 內部可繪製
        textDc.PushClip(Rect(50.0f, 50.0f, 100.0f, 100.0f));
        textDc.DrawText("ClippedTop", Point(10.0f, 10.0f), cppui::text::FontDescriptor("Arial", 14.0f), Colors::White);
        textDc.DrawText("ClippedBottom", Point(10.0f, 170.0f), cppui::text::FontDescriptor("Arial", 14.0f), Colors::White);
        textDc.DrawText("Inside", Point(60.0f, 60.0f), cppui::text::FontDescriptor("Arial", 14.0f), Colors::White);
        textDc.PopClip();

        DisplayListRenderer textRenderer;
        textRenderer.Render(textDl, &textTarget);

        const uint32_t* pixels = textTarget.GetPixelBuffer();
        uint32_t blackVal = Colors::Black.ToArgb();

        bool outsideUnmodified = true;
        for (int32_t y = 0; y < 50; ++y) {
            for (int32_t x = 0; x < 200; ++x) {
                if (pixels[static_cast<size_t>(y) * 200 + x] != blackVal) {
                    outsideUnmodified = false;
                    break;
                }
            }
        }
        for (int32_t y = 150; y < 200; ++y) {
            for (int32_t x = 0; x < 200; ++x) {
                if (pixels[static_cast<size_t>(y) * 200 + x] != blackVal) {
                    outsideUnmodified = false;
                    break;
                }
            }
        }
        assert(outsideUnmodified && "Pixels outside clip rect should not be modified by DrawText!");
        compat::println("[PASS] DrawText clip boundary clipping verified (no text bleed outside clip rect)");
    }

    // 6. 極限效能基準測試 (Benchmark): 單幀錄製 50,000 個幾何指令
    constexpr size_t BENCHMARK_OP_COUNT = 50000;
    compat::println("[BENCHMARK] Starting recording benchmark with {} operations...", BENCHMARK_OP_COUNT);

    auto benchStart = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < BENCHMARK_OP_COUNT; ++i) {
        float fi = static_cast<float>(i % 100);
        dc.DrawRectangle(Rect(fi, fi, 20.0f, 20.0f), Colors::SteelBlue);
    }
    auto benchEnd = std::chrono::high_resolution_clock::now();

    double elapsedMs = std::chrono::duration<double, std::milli>(benchEnd - benchStart).count();
    compat::println("[BENCHMARK] 50,000 Ops recorded in {:.3f} ms (Target < 1.5 ms)", elapsedMs);
    compat::println("[BENCHMARK] DisplayList total memory used: {:.2f} KB, Op Count: {}",
                    static_cast<double>(dl.GetTotalBytes()) / 1024.0, dl.GetOpCount());

    assert(dl.GetOpCount() == BENCHMARK_OP_COUNT);
    assert(elapsedMs < 50.0); // 寬鬆斷言防平行負載抖動，實測通常遠低於 1.5ms

    dl.Clear();
    compat::println("[CPPUI DisplayList & Arena Test] All tests passed successfully!");
    return 0;
}
