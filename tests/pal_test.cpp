/**
 * @file pal_test.cpp
 * @brief 平台抽象層 (PAL) 原始事件封裝、修飾鍵運算與軟體光柵化單元測試
 */

#include <cassert>
#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"
#include "../src/rendering/backend/SoftwareRenderContext.hpp"

int main() {
    using namespace cppui::pal;
    using namespace cppui::core;
    using namespace cppui::rendering;

    compat::println("[CPPUI PAL & Backend Test] Starting verification...");

    // 1. 測試 RawInputEvent 修飾鍵位元運算
    KeyModifiers mods = KeyModifiers::Control | KeyModifiers::Shift;
    assert(HasModifier(mods, KeyModifiers::Control));
    assert(HasModifier(mods, KeyModifiers::Shift));
    assert(!HasModifier(mods, KeyModifiers::Alt));
    compat::println("[PASS] KeyModifiers bitwise operations");

    // 2. 測試 RawInputEvent 滑鼠事件封裝 (邏輯座標與物理座標)
    Point logicalPt(100.0f, 200.0f);
    Point physicalPt(150.0f, 300.0f); // 假設 150% 縮放
    RawInputEvent mouseMove = RawInputEvent::MakeMouseMove(logicalPt, physicalPt, mods, 1000);
    assert(mouseMove.type == RawEventType::MouseMove);
    assert(mouseMove.mouse.position.x == 100.0f);
    assert(mouseMove.mouse.position.y == 200.0f);
    assert(mouseMove.mouse.physicalPosition.x == 150.0f);
    assert(mouseMove.mouse.physicalPosition.y == 300.0f);
    assert(mouseMove.timestampMs == 1000);
    compat::println("[PASS] RawInputEvent::MakeMouseMove (Logical & Physical Coordinates)");

    RawInputEvent mouseDown = RawInputEvent::MakeMouseDown(logicalPt, physicalPt, RawMouseButton::Left, 2, mods, 1050);
    assert(mouseDown.type == RawEventType::MouseDown);
    assert(mouseDown.mouse.button == RawMouseButton::Left);
    assert(mouseDown.mouse.physicalPosition.x == 150.0f);
    assert(mouseDown.mouse.clickCount == 2);
    compat::println("[PASS] RawInputEvent::MakeMouseDown (Double Click & Physical Pos)");

    // 3. 測試 SoftwareRenderTarget 幾何光柵化與清屏
    SoftwareRenderTarget target;
    target.Resize(200, 100);
    assert(target.GetWidth() == 200);
    assert(target.GetHeight() == 100);

    // 清屏為藍色
    target.Clear(Colors::Blue);
    const uint32_t* pixels = target.GetPixelBuffer();
    // 檢查第一個像素的藍色分量 (BGRA: B=255, G=0, R=0, A=255 -> 0xFFFF0000 或 0xFF0000FF)
    assert((pixels[0] & 0xFF) == 255); // Blue channel

    // 填充紅色矩形
    Rect fillRc(10, 10, 50, 50);
    target.FillRect(fillRc, Colors::Red);
    // 檢查 (10, 10) 像素是否為紅色 (BGRA: R=255 -> 0x00FF0000)
    uint32_t redPixel = pixels[10 * 200 + 10];
    assert(((redPixel >> 16) & 0xFF) == 255);
    (void)redPixel;
    compat::println("[PASS] SoftwareRenderTarget Clear & FillRect");

    compat::println("[CPPUI PAL & Backend Test] All tests passed successfully!");
    return 0;
}
