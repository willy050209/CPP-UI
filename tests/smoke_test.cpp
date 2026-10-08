/**
 * @file smoke_test.cpp
 * @brief 基礎環境冒煙測試（compat 字串視圖、optional、expected、span、views 管道與幾何結構）
 */

#include <cassert>
#include <cstdint>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"

int main() {
    using namespace cppui::core;

    compat::println("[CPPUI Smoke Test] Starting environment verification...");

    // 1. 驗證 compat::string_view
    compat::string_view frameworkName = "CPPUI";
    assert(frameworkName.length() == 5);
    compat::println("[PASS] compat::string_view: {}", frameworkName);

    // 2. 驗證 compat::optional
    compat::optional<int32_t> optVal = 42;
    assert(optVal.has_value());
    assert(*optVal == 42);
    compat::println("[PASS] compat::optional: {}", *optVal);

    // 3. 驗證 compat::expected
    compat::expected<int32_t, const char*> result = 100;
    assert(result.has_value());
    assert(*result == 100);
    compat::println("[PASS] compat::expected: {}", *result);

    // 4. 驗證 Geometry 幾何結構
    Point pt(10.0f, 20.0f);
    Size sz(100.0f, 50.0f);
    Rect rc(pt, sz);

    assert(rc.Left() == 10.0f);
    assert(rc.Top() == 20.0f);
    assert(rc.Right() == 110.0f);
    assert(rc.Bottom() == 70.0f);
    assert(rc.Contains(Point(50.0f, 30.0f)));
    assert(!rc.Contains(Point(0.0f, 0.0f)));
    compat::println("[PASS] Geometry (Point, Size, Rect, Contains)");

    // 5. 驗證 Color
    Color col = Colors::SteelBlue;
    assert(col.r == 70 && col.g == 130 && col.b == 180 && col.a == 255);
    compat::println("[PASS] Color (SteelBlue RGBA)");

    // 6. 驗證 compat::span
    int32_t arr[] = { 10, 20, 30, 40, 50 };
    compat::span<int32_t> sp(arr);
    assert(sp.size() == 5);
    assert(sp[0] == 10);
    assert(sp.back() == 50);
    compat::span<int32_t> sub = sp.subspan(1, 3);
    assert(sub.size() == 3);
    assert(sub[0] == 20);
    compat::println("[PASS] compat::span (size={}, subspan={})", sp.size(), sub.size());

    // 7. 驗證 compat::views 惰性管道
    int32_t sum = 0;
    auto pipe = compat::views::iota(1, 10)
        | compat::views::filter([](int32_t n) { return n % 2 == 0; })
        | compat::views::take(3);
    compat::ranges::for_each(pipe, [&](int32_t v) {
        sum += v;
    });
    assert(sum == (2 + 4 + 6));
    compat::println("[PASS] compat::views pipeline (iota | filter | take)");

    compat::println("[CPPUI Smoke Test] All checks passed successfully!");
    return 0;
}
