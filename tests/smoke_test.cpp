/**
 * @file smoke_test.cpp
 * @brief 基礎環境冒煙測試（compat 字串視圖、optional、expected、span、views 管道與幾何結構）
 */

#include <cassert>
#include <cstdint>
#include "compat/Compat.hpp"
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

    // 8. 驗證 compat::to_string
    std::string sInt = compat::to_string(42);
    assert(sInt == "42");
    std::string sNeg = compat::to_string(-100);
    assert(sNeg == "-100");
    std::string sI64 = compat::to_string(static_cast<int64_t>(1234567890123LL));
    assert(sI64 == "1234567890123");
    std::string sFlt = compat::to_string(3.5f);
    assert(sFlt == "3.5");
    compat::println("[PASS] compat::to_string (int={}, neg={}, int64={}, float={})", sInt, sNeg, sI64, sFlt);

    // 9. 驗證 compat::parse<T> (強型別零例外解析)
    auto p1 = compat::parse<int32_t>("12345");
    assert(p1.has_value());
    assert(*p1 == 12345);
    auto p2 = compat::parse<int32_t>("-999");
    assert(p2.has_value());
    assert(*p2 == -999);
    auto pFloat = compat::parse<float>("3.5");
    assert(pFloat.has_value());
    assert(*pFloat == 3.5f);
    auto pErr = compat::parse<int32_t>("not_a_number");
    assert(!pErr.has_value());
    auto pEmpty = compat::parse<int32_t>("");
    assert(!pEmpty.has_value());
    compat::println("[PASS] compat::parse<T> (success & fail-fast unexpected validation)");

    // 10. 驗證 compat::from_chars (底層進位制解析)
    const char hexStr[] = "1A3F";
    int32_t hexVal = 0;
    auto fcRes = compat::from_chars(hexStr, hexStr + 4, hexVal, 16);
    assert(fcRes.ec == std::errc{});
    assert(hexVal == 6719);
    compat::println("[PASS] compat::from_chars (hexStr=1A3F -> val={})", hexVal);

    compat::println("[CPPUI Smoke Test] All checks passed successfully!");
    return 0;
}
