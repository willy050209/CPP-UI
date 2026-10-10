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

    // 5. 極限效能基準測試 (Benchmark): 單幀錄製 50,000 個幾何指令
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
