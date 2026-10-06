#include <iostream>
#include <cassert>
#include "compat/compat.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"
#include "../../src/rendering/backend/SoftwareRenderContext.hpp"

int main() {
    using namespace cppui::pal;
    using namespace cppui::core;
    using namespace cppui::rendering;

    std::cout << "[CPPUI PAL & Backend Test] Starting verification..." << std::endl;

    // 1. 測試 RawInputEvent 修飾鍵位元運算
    KeyModifiers mods = KeyModifiers::Control | KeyModifiers::Shift;
    assert(HasModifier(mods, KeyModifiers::Control));
    assert(HasModifier(mods, KeyModifiers::Shift));
    assert(!HasModifier(mods, KeyModifiers::Alt));
    std::cout << "[PASS] KeyModifiers bitwise operations" << std::endl;

    // 2. 測試 RawInputEvent 滑鼠事件封裝
    Point pt(150.0f, 250.0f);
    RawInputEvent mouseMove = RawInputEvent::MakeMouseMove(pt, mods, 1000);
    assert(mouseMove.type == RawEventType::MouseMove);
    assert(mouseMove.mouse.position.x == 150.0f);
    assert(mouseMove.mouse.position.y == 250.0f);
    assert(mouseMove.timestampMs == 1000);
    std::cout << "[PASS] RawInputEvent::MakeMouseMove" << std::endl;

    RawInputEvent mouseDown = RawInputEvent::MakeMouseDown(pt, RawMouseButton::Left, 2, mods, 1050);
    assert(mouseDown.type == RawEventType::MouseDown);
    assert(mouseDown.mouse.button == RawMouseButton::Left);
    assert(mouseDown.mouse.clickCount == 2);
    std::cout << "[PASS] RawInputEvent::MakeMouseDown (Double Click)" << std::endl;

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
    std::cout << "[PASS] SoftwareRenderTarget Clear & FillRect" << std::endl;

    std::cout << "[CPPUI PAL & Backend Test] All tests passed successfully!" << std::endl;
    return 0;
}
