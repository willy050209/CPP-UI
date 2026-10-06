#include <iostream>
#include <cassert>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"

int main() {
    using namespace cppui::core;

    std::cout << "[CPPUI Smoke Test] Starting environment verification..." << std::endl;

    // 1. 驗證 compat::string_view
    compat::string_view frameworkName = "CPPUI";
    assert(frameworkName.length() == 5);
    std::cout << "[PASS] compat::string_view: " << frameworkName << std::endl;

    // 2. 驗證 compat::optional
    compat::optional<int> optVal = 42;
    assert(optVal.has_value());
    assert(*optVal == 42);
    std::cout << "[PASS] compat::optional: " << *optVal << std::endl;

    // 3. 驗證 compat::expected
    compat::expected<int, const char*> result = 100;
    assert(result.has_value());
    assert(*result == 100);
    std::cout << "[PASS] compat::expected: " << *result << std::endl;

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
    std::cout << "[PASS] Geometry (Point, Size, Rect, Contains)" << std::endl;

    // 5. 驗證 Color
    Color col = Colors::SteelBlue;
    assert(col.r == 70 && col.g == 130 && col.b == 180 && col.a == 255);
    std::cout << "[PASS] Color (SteelBlue RGBA)" << std::endl;

    std::cout << "[CPPUI Smoke Test] All checks passed successfully!" << std::endl;
    return 0;
}
