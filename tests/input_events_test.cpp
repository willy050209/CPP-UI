/**
 * @file input_events_test.cpp
 * @brief 輸入路由、冒泡與穿透事件、焦點管理與原始輸入管線單元測試
 */

#include <string>
#include <vector>
#include <cassert>
#include <cstdint>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/Visual.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/input/InputTypes.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/FocusEventArgs.hpp"
#include "ui/input/EventRoute.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/InputManager.hpp"

namespace {
class TestElement : public cppui::core::UIElement {
public:
    std::string name;
    explicit TestElement(std::string n) : name(std::move(n)) {}
};
} // anonymous namespace

int main() {
    using namespace cppui;
    using namespace cppui::core;
    using namespace cppui::layout;
    using namespace cppui::input;

    compat::println("=================================================");
    compat::println(" [CPPUI M5] Input & Routed Events System Tests   ");
    compat::println("=================================================");

    // -------------------------------------------------------------------------
    // [Test 1] Tunneling & Bubbling Strict Order
    // -------------------------------------------------------------------------
    compat::println("[Test 1] Tunneling & Bubbling Strict Order...");
    {
        auto root = std::make_shared<TestElement>("Root");
        auto child = std::make_shared<TestElement>("Child");
        auto leaf = std::make_shared<TestElement>("Leaf");

        root->AddVisualChild(child);
        child->AddVisualChild(leaf);

        std::vector<std::string> log;

        auto attachLog = [&](const std::shared_ptr<TestElement>& elem) {
            elem->AddHandler(StandardEvents::PreviewPointerPressed, [&log, n = elem->name](RoutedEventArgs&) {
                log.push_back(n + ":Preview");
            });
            elem->AddHandler(StandardEvents::PointerPressed, [&log, n = elem->name](RoutedEventArgs&) {
                log.push_back(n + ":Bubble");
            });
        };

        attachLog(root);
        attachLog(child);
        attachLog(leaf);

        PointerEventArgs args(StandardEvents::PointerPressed, leaf.get(), Point(10.0f, 10.0f));
        EventRoute route(leaf.get());
        route.Dispatch(StandardEvents::PreviewPointerPressed, StandardEvents::PointerPressed, args);

        std::vector<std::string> expected = {
            "Root:Preview", "Child:Preview", "Leaf:Preview",
            "Leaf:Bubble", "Child:Bubble", "Root:Bubble"
        };

        assert(log.size() == expected.size());
        for (size_t i = 0; i < expected.size(); ++i) {
            assert(log[i] == expected[i]);
        }
        compat::println("  [PASS] Tunneling (Root->Leaf) then Bubbling (Leaf->Root) order verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 2] Handled Short-Circuiting
    // -------------------------------------------------------------------------
    compat::println("[Test 2] Handled Short-Circuiting Propagation...");
    {
        auto root = std::make_shared<TestElement>("Root");
        auto child = std::make_shared<TestElement>("Child");
        auto leaf = std::make_shared<TestElement>("Leaf");

        root->AddVisualChild(child);
        child->AddVisualChild(leaf);

        std::vector<std::string> log;

        root->AddHandler(StandardEvents::PreviewPointerPressed, [&](RoutedEventArgs&) {
            log.push_back("Root:Preview");
        });
        child->AddHandler(StandardEvents::PreviewPointerPressed, [&](RoutedEventArgs& e) {
            log.push_back("Child:Preview (Handles!)");
            e.SetHandled(true);
        });
        leaf->AddHandler(StandardEvents::PreviewPointerPressed, [&](RoutedEventArgs&) {
            log.push_back("Leaf:Preview");
        });
        leaf->AddHandler(StandardEvents::PointerPressed, [&](RoutedEventArgs&) {
            log.push_back("Leaf:Bubble");
        });
        child->AddHandler(StandardEvents::PointerPressed, [&](RoutedEventArgs&) {
            log.push_back("Child:Bubble");
        });
        root->AddHandler(StandardEvents::PointerPressed, [&](RoutedEventArgs&) {
            log.push_back("Root:Bubble");
        });

        PointerEventArgs args(StandardEvents::PointerPressed, leaf.get(), Point(10.0f, 10.0f));
        EventRoute route(leaf.get());
        route.Dispatch(StandardEvents::PreviewPointerPressed, StandardEvents::PointerPressed, args);

        assert(log.size() == 2);
        assert(log[0] == "Root:Preview");
        assert(log[1] == "Child:Preview (Handles!)");
        assert(args.IsHandled());
        compat::println("  [PASS] Handled = true stops regular downstream and upstream handlers.");
    }

    // -------------------------------------------------------------------------
    // [Test 3] HandledEventsToo Registration
    // -------------------------------------------------------------------------
    compat::println("[Test 3] HandledEventsToo Registration...");
    {
        auto root = std::make_shared<TestElement>("Root");
        auto leaf = std::make_shared<TestElement>("Leaf");
        root->AddVisualChild(leaf);

        bool regularCalled = false;
        bool handledTooCalled = false;

        leaf->AddHandler(StandardEvents::PointerPressed, [](RoutedEventArgs& e) {
            e.SetHandled(true);
        });

        root->AddHandler(StandardEvents::PointerPressed, [&](RoutedEventArgs&) {
            regularCalled = true;
        }, false);

        root->AddHandler(StandardEvents::PointerPressed, [&](RoutedEventArgs&) {
            handledTooCalled = true;
        }, true); // handledEventsToo = true

        PointerEventArgs args(StandardEvents::PointerPressed, leaf.get(), Point(5.0f, 5.0f));
        EventRoute route(leaf.get());
        route.Dispatch(StandardEvents::PreviewPointerPressed, StandardEvents::PointerPressed, args);

        assert(!regularCalled);
        assert(handledTooCalled);
        compat::println("  [PASS] handledEventsToo = true correctly receives handled events.");
    }

    // -------------------------------------------------------------------------
    // [Test 4] Pointer Enter & Exit Direct Routing
    // -------------------------------------------------------------------------
    compat::println("[Test 4] Pointer Enter & Exit Direct Routing...");
    {
        InputManager::Get().ResetState();

        auto root = std::make_shared<Panel>();
        root->SetVisualBounds(Rect(0.0f, 0.0f, 400.0f, 400.0f));

        auto btnA = std::make_shared<TestElement>("BtnA");
        btnA->SetVisualOffset(Point(10.0f, 10.0f));
        btnA->SetVisualBounds(Rect(0.0f, 0.0f, 100.0f, 50.0f));

        auto btnB = std::make_shared<TestElement>("BtnB");
        btnB->SetVisualOffset(Point(150.0f, 10.0f));
        btnB->SetVisualBounds(Rect(0.0f, 0.0f, 100.0f, 50.0f));

        root->AddChild(btnA);
        root->AddChild(btnB);

        std::vector<std::string> hoverLog;
        btnA->OnPointerEntered([&](PointerEventArgs&) { hoverLog.push_back("BtnA:Enter"); });
        btnA->OnPointerExited([&](PointerEventArgs&) { hoverLog.push_back("BtnA:Exit"); });
        btnB->OnPointerEntered([&](PointerEventArgs&) { hoverLog.push_back("BtnB:Enter"); });
        btnB->OnPointerExited([&](PointerEventArgs&) { hoverLog.push_back("BtnB:Exit"); });

        // 1. 滑入 BtnA 內部 (20, 20)
        pal::RawMouseEventData m1;
        m1.position = Point(20.0f, 20.0f);
        InputManager::Get().ProcessMouseMove(*root, m1, pal::KeyModifiers::None);
        assert(hoverLog.size() == 1 && hoverLog[0] == "BtnA:Enter");
        assert(InputManager::Get().GetHoveredElement() == btnA.get());

        // 2. 移動至 BtnB 內部 (160, 20)
        pal::RawMouseEventData m2;
        m2.position = Point(160.0f, 20.0f);
        InputManager::Get().ProcessMouseMove(*root, m2, pal::KeyModifiers::None);
        assert(hoverLog.size() == 3);
        assert(hoverLog[1] == "BtnA:Exit");
        assert(hoverLog[2] == "BtnB:Enter");
        assert(InputManager::Get().GetHoveredElement() == btnB.get());

        // 3. 移出外部 (350, 350)
        pal::RawMouseEventData m3;
        m3.position = Point(350.0f, 350.0f);
        InputManager::Get().ProcessMouseMove(*root, m3, pal::KeyModifiers::None);
        assert(hoverLog.size() == 4);
        assert(hoverLog[3] == "BtnB:Exit");
        assert(InputManager::Get().GetHoveredElement() == root.get());

        InputManager::Get().ResetState();
        compat::println("  [PASS] Hover state tracking accurately generates Enter/Exit pair.");
    }

    // -------------------------------------------------------------------------
    // [Test 5] Pointer Capture Semantics
    // -------------------------------------------------------------------------
    compat::println("[Test 5] Pointer Capture Semantics...");
    {
        InputManager::Get().ResetState();

        auto root = std::make_shared<Panel>();
        root->SetVisualBounds(Rect(0.0f, 0.0f, 400.0f, 400.0f));

        auto btn = std::make_shared<TestElement>("Btn");
        btn->SetVisualOffset(Point(10.0f, 10.0f));
        btn->SetVisualBounds(Rect(0.0f, 0.0f, 100.0f, 50.0f));
        root->AddChild(btn);

        bool movedReceived = false;
        bool releasedReceived = false;

        btn->OnPointerMoved([&](PointerEventArgs&) { movedReceived = true; });
        btn->OnPointerReleased([&](PointerEventArgs&) { releasedReceived = true; });

        // 按下時捕獲指標
        InputManager::Get().CapturePointer(btn.get());
        assert(InputManager::Get().GetCapturedElement() == btn.get());

        // 滑鼠移到視窗最遠角落 (390, 390)，依然應由 btn 收到 Moved
        pal::RawMouseEventData mMove;
        mMove.position = Point(390.0f, 390.0f);
        InputManager::Get().ProcessMouseMove(*root, mMove, pal::KeyModifiers::None);
        assert(movedReceived);

        // 滑鼠放開，btn 收到 Released
        pal::RawMouseEventData mUp;
        mUp.position = Point(390.0f, 390.0f);
        InputManager::Get().ProcessMouseUp(*root, mUp, pal::KeyModifiers::None);
        assert(releasedReceived);

        InputManager::Get().ReleasePointerCapture();
        assert(InputManager::Get().GetCapturedElement() == nullptr);

        compat::println("  [PASS] PointerCapture holds event dispatch even when mouse leaves element.");
    }

    // -------------------------------------------------------------------------
    // [Test 6] Focus Management & Focus Events
    // -------------------------------------------------------------------------
    compat::println("[Test 6] Focus Management & Focus Events...");
    {
        FocusManager::Get().ClearFocus();

        auto elem1 = std::make_shared<TestElement>("Elem1");
        auto elem2 = std::make_shared<TestElement>("Elem2");
        elem1->SetFocusable(true);
        elem2->SetFocusable(true);

        std::vector<std::string> focusLog;
        elem1->OnGotFocus([&](FocusEventArgs&) { focusLog.push_back("Elem1:Got"); });
        elem1->OnLostFocus([&](FocusEventArgs&) { focusLog.push_back("Elem1:Lost"); });
        elem2->OnGotFocus([&](FocusEventArgs&) { focusLog.push_back("Elem2:Got"); });
        elem2->OnLostFocus([&](FocusEventArgs&) { focusLog.push_back("Elem2:Lost"); });

        bool s1 = FocusManager::Get().SetFocus(elem1.get());
        assert(s1 && elem1->IsFocused() && !elem2->IsFocused());
        (void)s1;
        assert(FocusManager::Get().GetFocusedElement() == elem1.get());
        assert(focusLog.size() == 1 && focusLog[0] == "Elem1:Got");

        bool s2 = FocusManager::Get().SetFocus(elem2.get());
        assert(s2 && !elem1->IsFocused() && elem2->IsFocused());
        (void)s2;
        assert(FocusManager::Get().GetFocusedElement() == elem2.get());
        assert(focusLog.size() == 3);
        assert(focusLog[1] == "Elem1:Lost");
        assert(focusLog[2] == "Elem2:Got");

        FocusManager::Get().ClearFocus();
        assert(!elem1->IsFocused() && !elem2->IsFocused());
        assert(FocusManager::Get().GetFocusedElement() == nullptr);
        assert(focusLog.size() == 4 && focusLog[3] == "Elem2:Lost");

        compat::println("  [PASS] GotFocus and LostFocus events dispatched in lockstep.");
    }

    // -------------------------------------------------------------------------
    // [Test 7] Tab & Shift+Tab Navigation Algorithm
    // -------------------------------------------------------------------------
    compat::println("[Test 7] Tab & Shift+Tab Navigation Algorithm...");
    {
        auto root = std::make_shared<Panel>();
        auto f1 = std::make_shared<TestElement>("F1");
        auto f2 = std::make_shared<TestElement>("F2");
        auto f3 = std::make_shared<TestElement>("F3");

        f1->SetFocusable(true);
        f2->SetFocusable(true);
        f3->SetFocusable(true);

        f1->SetTabIndex(10);
        f2->SetTabIndex(5);
        f3->SetTabIndex(20);

        root->AddChild(f1);
        root->AddChild(f2);
        root->AddChild(f3);

        // 預期 TabIndex 排序為: F2 (5) -> F1 (10) -> F3 (20)
        FocusManager::Get().ClearFocus();

        // 第一次 Tab：應進入 F2
        FocusManager::Get().MoveFocus(root.get(), true);
        assert(FocusManager::Get().GetFocusedElement() == f2.get());

        // 第二次 Tab：應進入 F1
        FocusManager::Get().MoveFocus(root.get(), true);
        assert(FocusManager::Get().GetFocusedElement() == f1.get());

        // 第三次 Tab：應進入 F3
        FocusManager::Get().MoveFocus(root.get(), true);
        assert(FocusManager::Get().GetFocusedElement() == f3.get());

        // 第四次 Tab：循環回 F2
        FocusManager::Get().MoveFocus(root.get(), true);
        assert(FocusManager::Get().GetFocusedElement() == f2.get());

        // Shift+Tab (逆向)：回到 F3
        FocusManager::Get().MoveFocus(root.get(), false);
        assert(FocusManager::Get().GetFocusedElement() == f3.get());

        // 清理焦點，防止跨區塊物件解構時存取
        FocusManager::Get().ClearFocus();
        compat::println("  [PASS] TabIndex order and wrap-around navigation verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 8] RawInputEvent Pipeline Dispatch
    // -------------------------------------------------------------------------
    compat::println("[Test 8] RawInputEvent Pipeline Dispatch...");
    {
        InputManager::Get().ResetState();
        FocusManager::Get().ClearFocus();

        auto root = std::make_shared<Panel>();
        root->SetVisualBounds(Rect(0.0f, 0.0f, 600.0f, 400.0f));

        auto inputTarget = std::make_shared<TestElement>("InputTarget");
        inputTarget->SetVisualOffset(Point(50.0f, 50.0f));
        inputTarget->SetVisualBounds(Rect(0.0f, 0.0f, 200.0f, 80.0f));
        inputTarget->SetFocusable(true);
        root->AddChild(inputTarget);

        bool clickPressed = false;
        bool keyDownReceived = false;
        std::string typedText;

        inputTarget->OnPointerPressed([&](PointerEventArgs& e) {
            clickPressed = true;
            assert(e.IsLeftButton());
            (void)e;
        });

        inputTarget->OnKeyDown([&](KeyEventArgs& e) {
            keyDownReceived = true;
            assert(e.GetVirtualKey() == 0x41); // 'A'
            (void)e;
        });

        inputTarget->OnTextInput([&](TextInputEventArgs& e) {
            typedText = std::string(e.GetText().data(), e.GetText().size());
        });

        // 1. 注入 MouseDown (70, 70) -> 點擊 inputTarget 並自動獲焦
        pal::RawInputEvent evDown = pal::RawInputEvent::MakeMouseDown(
            Point(70.0f, 70.0f),
            pal::RawMouseButton::Left,
            1,
            pal::KeyModifiers::None
        );
        InputManager::Get().ProcessRawEvent(*root, evDown);

        assert(clickPressed);
        assert(FocusManager::Get().GetFocusedElement() == inputTarget.get());

        // 2. 注入 KeyDown ('A')
        pal::RawInputEvent evKey;
        evKey.type = pal::RawEventType::KeyDown;
        evKey.key.virtualKey = 0x41;
        InputManager::Get().ProcessRawEvent(*root, evKey);
        assert(keyDownReceived);

        // 3. 注入 TextInput ('中')
        pal::RawInputEvent evText;
        evText.type = pal::RawEventType::TextInput;
        evText.text.codepoint = 0x4E2D; // '中'
        InputManager::Get().ProcessRawEvent(*root, evText);
        assert(typedText == "中");

        FocusManager::Get().ClearFocus();
        InputManager::Get().ResetState();
        compat::println("  [PASS] RawInputEvent end-to-end pipeline correctly dispatched.");
    }

    compat::println("=================================================");
    compat::println(" ALL M5 INPUT & ROUTED EVENTS TESTS PASSED!      ");
    compat::println("=================================================");
    return 0;
}
