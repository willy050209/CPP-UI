/**
 * @file coordinate_transform_test.cpp
 * @brief 高 DPI 物理座標與邏輯座標雙向轉換及防禦機制單元測試
 */

#include <cassert>
#include <cmath>
#include <cstdint>
#include "compat/compat.hpp"
#include "ui/core/CoordinateTransform.hpp"

namespace {

inline bool ApproxEqual(float a, float b, float eps = 1e-4f) noexcept {
    return std::fabs(a - b) <= eps;
}

inline bool ApproxEqual(cppui::core::Point a, cppui::core::Point b, float eps = 1e-4f) noexcept {
    return ApproxEqual(a.x, b.x, eps) && ApproxEqual(a.y, b.y, eps);
}

inline bool ApproxEqual(cppui::core::Size a, cppui::core::Size b, float eps = 1e-4f) noexcept {
    return ApproxEqual(a.width, b.width, eps) && ApproxEqual(a.height, b.height, eps);
}

inline bool ApproxEqual(cppui::core::Rect a, cppui::core::Rect b, float eps = 1e-4f) noexcept {
    return ApproxEqual(a.x, b.x, eps) && ApproxEqual(a.y, b.y, eps) &&
           ApproxEqual(a.width, b.width, eps) && ApproxEqual(a.height, b.height, eps);
}

inline bool ApproxEqual(cppui::core::Thickness a, cppui::core::Thickness b, float eps = 1e-4f) noexcept {
    return ApproxEqual(a.left, b.left, eps) && ApproxEqual(a.top, b.top, eps) &&
           ApproxEqual(a.right, b.right, eps) && ApproxEqual(a.bottom, b.bottom, eps);
}

} // anonymous namespace

int main() {
    using namespace cppui::core;

    compat::println("=================================================");
    compat::println(" [CPPUI] CoordinateTransform Verification Tests ");
    compat::println("=================================================");

    // -------------------------------------------------------------------------
    // [Test 1] Construction & DPI Scale Mutation
    // -------------------------------------------------------------------------
    compat::println("[Test 1] Construction & DPI Scale Mutation...");
    {
        CoordinateTransform ctDefault;
        assert(ApproxEqual(ctDefault.GetDpiScale(), 1.0f));

        CoordinateTransform ct150(1.5f);
        assert(ApproxEqual(ct150.GetDpiScale(), 1.5f));

        ctDefault.SetDpiScale(2.0f);
        assert(ApproxEqual(ctDefault.GetDpiScale(), 2.0f));

        compat::println("  [PASS] Default scale and setter verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 2] 100% DPI (1.0x) Identity Transformation
    // -------------------------------------------------------------------------
    compat::println("[Test 2] 100% DPI (1.0x) Identity Transformation...");
    {
        CoordinateTransform ct(1.0f);

        Point pt(120.0f, 450.0f);
        assert(ApproxEqual(ct.ToLogical(pt), pt));
        assert(ApproxEqual(ct.ToPhysical(pt), pt));

        Size sz(800.0f, 600.0f);
        assert(ApproxEqual(ct.ToLogical(sz), sz));
        assert(ApproxEqual(ct.ToPhysical(sz), sz));

        Rect rc(10.0f, 20.0f, 300.0f, 400.0f);
        assert(ApproxEqual(ct.ToLogical(rc), rc));
        assert(ApproxEqual(ct.ToPhysical(rc), rc));

        Thickness th(5.0f, 10.0f, 15.0f, 20.0f);
        assert(ApproxEqual(ct.ToLogical(th), th));
        assert(ApproxEqual(ct.ToPhysical(th), th));

        float val = 42.0f;
        assert(ApproxEqual(ct.ToLogical(val), val));
        assert(ApproxEqual(ct.ToPhysical(val), val));
        (void)val;

        compat::println("  [PASS] 1.0x identity conversion preserved 1:1.");
    }

    // -------------------------------------------------------------------------
    // [Test 3] High-DPI Scaling (1.25x, 1.5x, 2.0x, 3.0x)
    // -------------------------------------------------------------------------
    compat::println("[Test 3] High-DPI Scaling Precision...");
    {
        // 150% DPI 典型情境：視窗按鈕物理 (36, 120) -> 邏輯 (24, 80)
        CoordinateTransform ct150(1.5f);

        Point physicalBtn(36.0f, 120.0f);
        Point logicalBtn = ct150.ToLogical(physicalBtn);
        assert(ApproxEqual(logicalBtn, Point(24.0f, 80.0f)));

        Point backPhysical = ct150.ToPhysical(logicalBtn);
        assert(ApproxEqual(backPhysical, physicalBtn));

        Size physicalSize(150.0f, 300.0f);
        assert(ApproxEqual(ct150.ToLogical(physicalSize), Size(100.0f, 200.0f)));
        assert(ApproxEqual(ct150.ToPhysical(Size(100.0f, 200.0f)), physicalSize));

        Rect physicalRect(15.0f, 30.0f, 150.0f, 75.0f);
        assert(ApproxEqual(ct150.ToLogical(physicalRect), Rect(10.0f, 20.0f, 100.0f, 50.0f)));

        Thickness physicalMargin(15.0f, 30.0f, 45.0f, 60.0f);
        assert(ApproxEqual(ct150.ToLogical(physicalMargin), Thickness(10.0f, 20.0f, 30.0f, 40.0f)));

        // 200% DPI (2.0x) 情境
        CoordinateTransform ct200(2.0f);
        assert(ApproxEqual(ct200.ToLogical(Point(200.0f, 100.0f)), Point(100.0f, 50.0f)));
        assert(ApproxEqual(ct200.ToPhysical(Point(100.0f, 50.0f)), Point(200.0f, 100.0f)));

        // 125% DPI (1.25x) 情境
        CoordinateTransform ct125(1.25f);
        assert(ApproxEqual(ct125.ToLogical(Point(125.0f, 250.0f)), Point(100.0f, 200.0f)));
        assert(ApproxEqual(ct125.ToPhysical(Point(100.0f, 200.0f)), Point(125.0f, 250.0f)));

        compat::println("  [PASS] High-DPI (1.25x, 1.5x, 2.0x) conversions accurate.");
    }

    // -------------------------------------------------------------------------
    // [Test 4] Static Helper Functions
    // -------------------------------------------------------------------------
    compat::println("[Test 4] Static Helper Functions...");
    {
        Point pPhys = CoordinateTransform::LogicalToPhysical(Point(20.0f, 30.0f), 1.5f);
        assert(ApproxEqual(pPhys, Point(30.0f, 45.0f)));

        Point pLog = CoordinateTransform::PhysicalToLogical(Point(30.0f, 45.0f), 1.5f);
        assert(ApproxEqual(pLog, Point(20.0f, 30.0f)));

        Size sPhys = CoordinateTransform::LogicalToPhysical(Size(40.0f, 60.0f), 2.0f);
        assert(ApproxEqual(sPhys, Size(80.0f, 120.0f)));

        Size sLog = CoordinateTransform::PhysicalToLogical(Size(80.0f, 120.0f), 2.0f);
        assert(ApproxEqual(sLog, Size(40.0f, 60.0f)));

        Rect rPhys = CoordinateTransform::LogicalToPhysical(Rect(10.0f, 20.0f, 50.0f, 60.0f), 1.5f);
        assert(ApproxEqual(rPhys, Rect(15.0f, 30.0f, 75.0f, 90.0f)));

        Thickness tPhys = CoordinateTransform::LogicalToPhysical(Thickness(2.0f, 4.0f, 6.0f, 8.0f), 2.5f);
        assert(ApproxEqual(tPhys, Thickness(5.0f, 10.0f, 15.0f, 20.0f)));

        float fLog = CoordinateTransform::PhysicalToLogical(150.0f, 1.5f);
        assert(ApproxEqual(fLog, 100.0f));
        (void)fLog;

        float fPhys = CoordinateTransform::LogicalToPhysical(100.0f, 1.5f);
        assert(ApproxEqual(fPhys, 150.0f));
        (void)fPhys;

        compat::println("  [PASS] Stateless static helper functions verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 5] Round-Trip Conversion Invariant
    // -------------------------------------------------------------------------
    compat::println("[Test 5] Round-Trip Conversion Invariant...");
    {
        const float testScales[] = { 1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f };
        for (float scale : testScales) {
            CoordinateTransform ct(scale);

            Point ptOrig(123.45f, 678.91f);
            Point ptRoundTrip = ct.ToPhysical(ct.ToLogical(ptOrig));
            assert(ApproxEqual(ptRoundTrip, ptOrig, 1e-3f));

            Rect rcOrig(12.3f, 45.6f, 78.9f, 101.1f);
            Rect rcRoundTrip = ct.ToPhysical(ct.ToLogical(rcOrig));
            assert(ApproxEqual(rcRoundTrip, rcOrig, 1e-3f));
        }

        compat::println("  [PASS] Round-trip identity invariant holds across all scales.");
    }

    // -------------------------------------------------------------------------
    // [Test 6] Safety Guard against Non-positive Scales
    // -------------------------------------------------------------------------
    compat::println("[Test 6] Safety Guard against Non-positive Scales...");
    {
        // scale <= 0.0f 應自動回退為 1.0f，防止浮點除零崩潰
        CoordinateTransform ctZero(0.0f);
        assert(ApproxEqual(ctZero.GetDpiScale(), 1.0f));

        CoordinateTransform ctNeg(-1.5f);
        assert(ApproxEqual(ctNeg.GetDpiScale(), 1.0f));

        CoordinateTransform ctSetter;
        ctSetter.SetDpiScale(0.0f);
        assert(ApproxEqual(ctSetter.GetDpiScale(), 1.0f));
        ctSetter.SetDpiScale(-5.0f);
        assert(ApproxEqual(ctSetter.GetDpiScale(), 1.0f));

        // 靜態方法非正數縮放回退
        Point ptFallback = CoordinateTransform::PhysicalToLogical(Point(100.0f, 200.0f), 0.0f);
        assert(ApproxEqual(ptFallback, Point(100.0f, 200.0f)));

        compat::println("  [PASS] Division by zero prevented with fallback to 1.0f.");
    }

    compat::println("=================================================");
    compat::println(" ALL COORDINATE TRANSFORM TESTS PASSED!          ");
    compat::println("=================================================");
    return 0;
}
