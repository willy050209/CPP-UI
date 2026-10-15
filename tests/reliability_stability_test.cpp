/**
 * @file reliability_stability_test.cpp
 * @brief 單元測試：可靠性與跨平台穩定性防禦機制驗證
 * 涵蓋 EventRoute 生命週期保護、Light-Dismiss 迭代器防護、LayoutManager 震盪防護與 NaN/Inf 幾何消毒
 */

#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/input/EventRoute.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/ILightDismissable.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/pal/IClipboard.hpp"
#include "ui/dialogs/DialogCommon.hpp"

namespace cppui {
namespace test {

// 測試用動態銷毀元件
class SelfDestructElement : public core::UIElement {
public:
    std::shared_ptr<core::UIElement> child;
    bool handlerExecuted = false;

    SelfDestructElement() = default;
};

// 測試用相互關閉的浮層
class MockPopup : public core::UIElement, public input::ILightDismissable {
public:
    explicit MockPopup(compat::string_view name) : m_name(name) {}

    bool StaysOpen() const noexcept override { return false; }
    core::UIElement* PlacementTarget() const noexcept override { return nullptr; }

    void SetIsOpen(bool open) override {
        m_isOpen = open;
        if (!open) {
            input::InputManager::Get().UnregisterOpenPopup(this);
            // 級聯關閉另一個浮層 (若指定)
            if (m_siblingToClose && m_siblingToClose->m_isOpen) {
                m_siblingToClose->SetIsOpen(false);
            }
        }
    }

    void SetSiblingToClose(MockPopup* sibling) {
        m_siblingToClose = sibling;
    }

    bool IsOpen() const noexcept { return m_isOpen; }
    std::string GetName() const { return m_name; }

private:
    std::string m_name;
    bool        m_isOpen = true;
    MockPopup*  m_siblingToClose = nullptr;
};

// 測試用震盪排版元件 (在 ArrangeOverride 中強制標記 InvalidateMeasure)
class OscillatingElement : public core::UIElement {
public:
    uint32_t arrangeCount = 0;
    bool shouldOscillate = true;

    core::Size MeasureOverride(core::Size availableSize) override {
        return core::Size(std::min(availableSize.width, 100.0f), std::min(availableSize.height, 100.0f));
    }

    void ArrangeOverride(core::Size finalSize) override {
        (void)finalSize;
        arrangeCount++;
        if (shouldOscillate) {
            InvalidateMeasure();
        }
    }
};

static void TestEventRouteUafProtection() {
    compat::println("[Test] 1. EventRoute LifetimeToken UAF Protection...");

    // 案例 A：同一元件內部多個回呼，第一次回呼直接銷毀自身
    {
        auto target = std::make_shared<SelfDestructElement>();
        bool secondHandlerCalled = false;

        target->OnPointerPressed([&target](input::PointerEventArgs& e) {
            (void)e;
            target.reset(); // 銷毀自身，觸發 Invalidate()
        });

        target->OnPointerPressed([&secondHandlerCalled](input::PointerEventArgs& e) {
            (void)e;
            secondHandlerCalled = true;
        });

        core::UIElement* rawTarget = target.get();
        input::PointerEventArgs args(
            input::StandardEvents::PointerPressed,
            rawTarget,
            core::Point(10.0f, 10.0f),
            pal::RawMouseButton::Left,
            pal::KeyModifiers::None
        );

        input::EventRoute route(rawTarget);
        route.Dispatch(input::StandardEvents::PreviewPointerPressed, input::StandardEvents::PointerPressed, args);

        assert(!secondHandlerCalled);
        compat::println("  -> PASSED (Case A): Self-destructing element safely skipped second handler.");
    }

    // 案例 B：父子結構中，父元件於隧道階段銷毀子節點，後續隧道派發安全略過已銷毀子節點
    {
        auto root = std::make_shared<SelfDestructElement>();
        auto child = std::make_shared<SelfDestructElement>();
        root->AddVisualChild(child);
        core::UIElement* rawChild = child.get();

        bool childHandlerExecuted = false;

        root->AddHandler(input::StandardEvents::PreviewPointerPressed, [&root](input::RoutedEventArgs& e) {
            (void)e;
            root->ClearVisualChildren(); // 銷毀唯一的 child 節點
        });

        child->AddHandler(input::StandardEvents::PreviewPointerPressed, [&childHandlerExecuted](input::RoutedEventArgs& e) {
            (void)e;
            childHandlerExecuted = true;
        });

        // 釋放局部 child shared_ptr，使 root 成為唯一持有者
        child.reset();

        input::PointerEventArgs args(
            input::StandardEvents::PointerPressed,
            rawChild,
            core::Point(10.0f, 10.0f),
            pal::RawMouseButton::Left,
            pal::KeyModifiers::None
        );

        input::EventRoute route(rawChild);
        route.Dispatch(input::StandardEvents::PreviewPointerPressed, input::StandardEvents::PointerPressed, args);

        assert(!childHandlerExecuted);
        compat::println("  -> PASSED (Case B): Child destroyed during tunneling was safely skipped.");
    }
}

static void TestLightDismissCascadeProtection() {
    compat::println("[Test] 2. LightDismiss Iterator Invalidation Protection...");

    auto& im = input::InputManager::Get();
    im.ResetState();

    auto popup1 = std::make_shared<MockPopup>("Popup1");
    auto popup2 = std::make_shared<MockPopup>("Popup2");

    // 設定 popup1 關閉時會連帶關閉 popup2
    popup1->SetSiblingToClose(popup2.get());

    im.RegisterOpenPopup(popup1.get());
    im.RegisterOpenPopup(popup2.get());

    assert(im.GetOpenPopups().size() == 2);

    // 模擬滑鼠在外部點擊觸發 Light-Dismiss
    core::UIElement root;
    pal::RawMouseEventData mouseData;
    mouseData.position = core::Point(500.0f, 500.0f);
    mouseData.button = pal::RawMouseButton::Left;
    im.ProcessMouseDown(root, mouseData, pal::KeyModifiers::None);

    // 驗證兩個浮層皆已安全關閉，且無迭代器崩潰
    assert(!popup1->IsOpen());
    assert(!popup2->IsOpen());
    assert(im.GetOpenPopups().empty());
    compat::println("  -> PASSED: Cascading popup dismissal executed without iterator invalidation.");
}

static void TestLayoutManagerOscillationDefense() {
    compat::println("[Test] 3. LayoutManager Oscillation Cycle Defense...");

    auto& lm = layout::LayoutManager::Get();
    lm.ResetStats();
    lm.SetMaxIterations(10); // 設定較小的上限以便測試

    auto root = std::make_shared<OscillatingElement>();
    root->shouldOscillate = true;

    // 觸發排版更新，驗證迴圈是否在達到上限後安全中斷
    lm.UpdateLayout(*root, core::Size(300.0f, 300.0f));

    auto stats = lm.GetStats();
    assert(stats.totalOscillations == 1);
    assert(stats.totalArranges == 10);
    assert(stats.totalMeasures == 10);

    // 恢復預設上限
    lm.SetMaxIterations(layout::LayoutManager::kDefaultMaxLayoutIterations);
    compat::println("  -> PASSED: Oscillation terminated safely at max iteration threshold.");
}

static void TestUIElementNanInfinitySanitization() {
    compat::println("[Test] 4. UIElement NaN & Infinity Sanitization...");

    core::UIElement elem;
    elem.SetWidth(150.0f);
    elem.SetHeight(80.0f);

    const float nanVal = std::numeric_limits<float>::quiet_NaN();
    const float infVal = std::numeric_limits<float>::infinity();

    // 測試傳入 NaN 尺寸
    elem.Measure(core::Size(nanVal, nanVal));
    assert(!std::isnan(elem.GetDesiredSize().width));
    assert(!std::isnan(elem.GetDesiredSize().height));
    assert(elem.GetDesiredSize().width == 150.0f);
    assert(elem.GetDesiredSize().height == 80.0f);

    // 測試傳入 Infinity 尺寸
    elem.Measure(core::Size(infVal, infVal));
    assert(!std::isinf(elem.GetDesiredSize().width));
    assert(!std::isinf(elem.GetDesiredSize().height));
    assert(elem.GetDesiredSize().width == 150.0f);

    // 測試 Arrange 傳入 NaN / Infinity
    elem.Arrange(core::Rect(nanVal, nanVal, infVal, infVal));
    assert(!std::isnan(elem.GetRenderBounds().x));
    assert(!std::isnan(elem.GetRenderBounds().y));
    assert(!std::isinf(elem.GetRenderBounds().width));
    assert(!std::isinf(elem.GetRenderBounds().height));

    compat::println("  -> PASSED: Geometry bounds safely sanitized against NaN and Infinity.");
}

static void TestSafeDirectoryIterationLogic() {
    compat::println("[Test] 5. Safe Filesystem Non-throwing Inspection...");

    std::error_code ec;
    auto options = compat::filesystem::directory_options::skip_permission_denied;
    // 遍歷當前工作目錄
    compat::filesystem::directory_iterator it(".", options, ec);
    compat::filesystem::directory_iterator endIt;

    size_t count = 0;
    while (!ec && it != endIt) {
        std::error_code statusEc;
        const bool isDir = it->is_directory(statusEc);
        if (!statusEc && isDir) {
            count++;
        }
        it.increment(ec);
    }

    assert(!ec);
    assert(count > 0);
    compat::println("  -> PASSED: Non-throwing filesystem iteration succeeded with {} directories.", count);
}

} // namespace test
} // namespace cppui

int main() {
    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Reliability & Cross-Platform Stability Test ");
    compat::println("==========================================================");

    cppui::test::TestEventRouteUafProtection();
    cppui::test::TestLightDismissCascadeProtection();
    cppui::test::TestLayoutManagerOscillationDefense();
    cppui::test::TestUIElementNanInfinitySanitization();
    cppui::test::TestSafeDirectoryIterationLogic();

    compat::println("==========================================================");
    compat::println(" ALL RELIABILITY & STABILITY TESTS PASSED (100% SUCCESS)  ");
    compat::println("==========================================================");
    return 0;
}
