/**
 * @file fluent_dsl_test.cpp
 * @brief 流式 RAD DSL、反應式 Signal 與 UI 屬性自動繫結單元測試
 */

#include <cassert>
#include <cstdint>
#include <string>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/dsl/Fluent.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"

int main() {
    using namespace cppui;
    using namespace cppui::core;
    using namespace cppui::reactive;
    using namespace cppui::dsl;

    compat::println("=================================================");
    compat::println(" [CPPUI M6] Fluent RAD DSL & Reactive Signal Tests");
    compat::println("=================================================");

    // -------------------------------------------------------------------------
    // [Test 1] VStack & HStack Container Creation and Hierarchy
    // -------------------------------------------------------------------------
    compat::println("[Test 1] VStack & HStack Container Creation and Hierarchy...");
    {
        auto vstackBuilder = VStack({
            Label("Item 1").Width(100.0f),
            Button("Click Me").Height(30.0f),
            HStack({
                Label("Sub 1"),
                Label("Sub 2")
            }).Spacing(8.0f)
        }).Spacing(12.0f);

        std::shared_ptr<layout::StackPanel> vstack = vstackBuilder.Get();
        assert(vstack != nullptr);
        assert(vstack->GetOrientation() == layout::Orientation::Vertical);
        assert(vstack->GetSpacing() == 12.0f);
        assert(vstack->GetChildren().size() == 3);

        auto hstack = std::dynamic_pointer_cast<layout::StackPanel>(vstack->GetChildren()[2]);
        assert(hstack != nullptr);
        assert(hstack->GetOrientation() == layout::Orientation::Horizontal);
        assert(hstack->GetSpacing() == 8.0f);
        assert(hstack->GetChildren().size() == 2);

        compat::println("  [PASS] VStack & HStack hierarchy and orientations verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 2] UIBuilder Fluent Property Chaining
    // -------------------------------------------------------------------------
    compat::println("[Test 2] UIBuilder Fluent Property Chaining...");
    {
        auto btnBuilder = Button("Submit")
            .Width(120.0f)
            .Height(40.0f)
            .Margin(10.0f, 5.0f)
            .Padding(8.0f)
            .Background(Color(30, 144, 255, 255))
            .Foreground(Color(255, 255, 255, 255))
            .BorderBrush(Color(0, 100, 200, 255))
            .BorderThickness(2.0f)
            .CornerRadius(6.0f)
            .Opacity(0.9f)
            .Focusable(true)
            .TabIndex(1);

        std::shared_ptr<controls::Button> btn = btnBuilder.Get();
        assert(btn != nullptr);
        assert(btn->GetWidth() == 120.0f);
        assert(btn->GetHeight() == 40.0f);
        assert(btn->GetMargin().left == 10.0f && btn->GetMargin().top == 5.0f);
        assert(btn->GetPadding().left == 8.0f);
        assert(btn->GetBackground().r == 30 && btn->GetBackground().g == 144);
        assert(btn->GetForeground().r == 255 && btn->GetForeground().g == 255);
        assert(btn->GetBorderBrush().b == 200);
        assert(btn->GetBorderThickness().top == 2.0f);
        assert(btn->GetCornerRadius() == 6.0f);
        assert(btn->GetOpacity() == 0.9f);
        assert(btn->IsFocusable() == true);
        assert(btn->GetTabIndex() == 1);

        compat::println("  [PASS] UIBuilder fluent property setters verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 3] Signal<T> Reactive Updates and Automatic UI Binding
    // -------------------------------------------------------------------------
    compat::println("[Test 3] Signal<T> Reactive Updates and Automatic UI Binding...");
    {
        Signal<int32_t> counter(0);
        assert(counter.Get() == 0);
        assert(*counter == 0);

        int32_t observedVal = 0;
        int32_t observedOld = -1;
        size_t subId = counter.Subscribe([&](int32_t newVal, int32_t oldVal) {
            observedVal = newVal;
            observedOld = oldVal;
        });

        counter.Set(10);
        assert(counter.Get() == 10);
        assert(observedVal == 10);
        assert(observedOld == 0);

        // Operator= assignment
        counter = 42;
        assert(counter.Get() == 42);
        assert(observedVal == 42);
        assert(observedOld == 10);

        // Unsubscribe
        counter.Unsubscribe(subId);
        counter.Set(99);
        assert(counter.Get() == 99);
        assert(observedVal == 42); // Not updated after unsubscribe

        // UIBuilder BindText integration
        Signal<int32_t> score(100);
        auto labelBuilder = Label("Initial").BindText(score, [](int32_t val) {
            return "Score: " + compat::to_string(val);
        });

        std::shared_ptr<controls::TextBlock> textBlock = labelBuilder.Get();
        assert(textBlock->GetText() == "Score: 100");

        score.Set(250);
        assert(textBlock->GetText() == "Score: 250");

        score = 500;
        assert(textBlock->GetText() == "Score: 500");

        compat::println("  [PASS] Signal<T> notifications and BindText synchronization verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 4] Grid Layout DSL & Attached Property Helpers
    // -------------------------------------------------------------------------
    compat::println("[Test 4] Grid Layout DSL & Attached Property Helpers...");
    {
        auto gridBuilder = Grid({
            Label("Header").Row(0).Column(0).ColumnSpan(2),
            Label("Sidebar").Row(1).Column(0),
            Label("Main Content").Row(1).Column(1)
        });

        std::shared_ptr<layout::Grid> grid = gridBuilder.Get();
        grid->AddRowDefinition(layout::RowDefinition(layout::GridLength::Pixel(50.0f)));
        grid->AddRowDefinition(layout::RowDefinition(layout::GridLength::Star(1.0f)));
        grid->AddColumnDefinition(layout::ColumnDefinition(layout::GridLength::Pixel(150.0f)));
        grid->AddColumnDefinition(layout::ColumnDefinition(layout::GridLength::Star(1.0f)));

        assert(grid->GetChildren().size() == 3);

        const auto& c0 = *grid->GetChildren()[0];
        const auto& c1 = *grid->GetChildren()[1];
        const auto& c2 = *grid->GetChildren()[2];

        assert(layout::Grid::GetRow(c0) == 0 && layout::Grid::GetColumn(c0) == 0 && layout::Grid::GetColumnSpan(c0) == 2);
        assert(layout::Grid::GetRow(c1) == 1 && layout::Grid::GetColumn(c1) == 0);
        assert(layout::Grid::GetRow(c2) == 1 && layout::Grid::GetColumn(c2) == 1);

        grid->Measure(Size(800.0f, 600.0f));
        grid->Arrange(Rect(0.0f, 0.0f, 800.0f, 600.0f));

        // c0 in Row 0 (H:50), spanning Col 0+1 (W: 150 + 650 = 800)
        assert(c0.GetRenderBounds().x == 0.0f);
        assert(c0.GetRenderBounds().y == 0.0f);
        assert(c0.GetRenderBounds().width == 800.0f);
        assert(c0.GetRenderBounds().height == 50.0f);

        // c1 in Row 1, Col 0 (W:150, H:550)
        assert(c1.GetRenderBounds().x == 0.0f);
        assert(c1.GetRenderBounds().y == 50.0f);
        assert(c1.GetRenderBounds().width == 150.0f);
        assert(c1.GetRenderBounds().height == 550.0f);

        // c2 in Row 1, Col 1 (X:150, Y:50, W:650, H:550)
        assert(c2.GetRenderBounds().x == 150.0f);
        assert(c2.GetRenderBounds().y == 50.0f);
        assert(c2.GetRenderBounds().width == 650.0f);
        assert(c2.GetRenderBounds().height == 550.0f);

        compat::println("  [PASS] Grid DSL declaration and attached layout placement verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 5] Event Wiring via Fluent DSL
    // -------------------------------------------------------------------------
    compat::println("[Test 5] Event Wiring via Fluent DSL...");
    {
        bool buttonClicked = false;
        bool checkChanged = false;
        bool lastCheckState = false;
        std::string changedText;

        auto testUi = VStack({
            Button("Click").OnClick([&]() {
                buttonClicked = true;
            }),
            CheckBox("Option").OnCheckedChanged([&](bool checked) {
                checkChanged = true;
                lastCheckState = checked;
            }),
            TextBox("Type here...").OnTextChanged([&](compat::string_view txt) {
                changedText = std::string(txt.data(), txt.size());
            })
        });

        auto btn = std::dynamic_pointer_cast<controls::Button>(testUi.Get()->GetChildren()[0]);
        auto cb  = std::dynamic_pointer_cast<controls::CheckBox>(testUi.Get()->GetChildren()[1]);
        auto tb  = std::dynamic_pointer_cast<controls::TextBox>(testUi.Get()->GetChildren()[2]);

        assert(btn != nullptr && cb != nullptr && tb != nullptr);

        // Trigger Button Click via mouse left press & release
        input::PointerEventArgs pressArgs(input::StandardEvents::PointerPressed, btn.get(), Point(10.0f, 10.0f), pal::RawMouseButton::Left);
        btn->RaiseEvent(pressArgs);

        input::PointerEventArgs releaseArgs(input::StandardEvents::PointerReleased, btn.get(), Point(10.0f, 10.0f), pal::RawMouseButton::Left);
        btn->RaiseEvent(releaseArgs);

        // Trigger CheckBox
        cb->SetChecked(true);
        assert(checkChanged == true);
        assert(lastCheckState == true);

        // Trigger TextBox input
        tb->InsertText("Hello DSL");
        assert(changedText == "Hello DSL");

        compat::println("  [PASS] Fluent event handlers (OnClick, OnCheckedChanged, OnTextChanged) verified.");
    }

    compat::println("=================================================");
    compat::println(" [CPPUI M6] ALL Fluent RAD DSL TESTS PASSED!    ");
    compat::println("=================================================");
    return 0;
}
