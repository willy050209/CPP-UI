#include <cassert>
#include <string>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/EventRoute.hpp"

using namespace cppui;
using namespace cppui::core;
using namespace cppui::controls;
using namespace cppui::layout;
using namespace cppui::input;

int main() {
    compat::println("=================================================");
    compat::println(" [CPPUI M6] Controls & Grid Layout Tests        ");
    compat::println("=================================================");

    // -------------------------------------------------------------------------
    // [Test 1] Control Base Properties & Insets
    // -------------------------------------------------------------------------
    compat::println("[Test 1] Control Base Properties & Insets...");
    {
        class TestControl : public Control {
        public:
            Size TestDeflate(Size sz) const noexcept { return DeflateSize(sz); }
            Size TestInflate(Size sz) const noexcept { return InflateSize(sz); }
        };

        TestControl ctrl;
        ctrl.SetPadding(Thickness(10.0f, 5.0f, 10.0f, 5.0f));
        ctrl.SetBorderThickness(Thickness(2.0f));
        ctrl.SetCornerRadius(6.0f);
        ctrl.SetBackground(Color(10, 20, 30, 255));
        ctrl.SetForeground(Color(200, 200, 200, 255));
        ctrl.SetBorderBrush(Color(255, 0, 0, 255));

        assert(ctrl.GetPadding().left == 10.0f);
        assert(ctrl.GetBorderThickness().top == 2.0f);
        assert(ctrl.GetCornerRadius() == 6.0f);
        assert(ctrl.GetBackground().r == 10);
        assert(ctrl.GetForeground().r == 200);

        // H-Inset = 10 + 10 + 2 + 2 = 24, V-Inset = 5 + 5 + 2 + 2 = 14
        Size deflated = ctrl.TestDeflate(Size(100.0f, 50.0f));
        assert(deflated.width == 76.0f);
        assert(deflated.height == 36.0f);

        Size inflated = ctrl.TestInflate(Size(76.0f, 36.0f));
        assert(inflated.width == 100.0f);
        assert(inflated.height == 50.0f);

        // Location 座標屬性測試
        assert(ctrl.GetLocation().x == 0.0f);
        assert(ctrl.GetLocation().y == 0.0f);
        ctrl.SetLocation(Point(120.0f, 65.0f));
        assert(ctrl.GetLocation().x == 120.0f);
        assert(ctrl.GetLocation().y == 65.0f);
        ctrl.SetLocation(200.0f, 150.0f);
        assert(ctrl.GetLocation().x == 200.0f);
        assert(ctrl.GetLocation().y == 150.0f);

        compat::println("  [PASS] Control styling properties, Location and insets verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 2] ContentControl Dynamic Content & Measurement
    // -------------------------------------------------------------------------
    compat::println("[Test 2] ContentControl Dynamic Content & Measurement...");
    {
        ContentControl cc;
        cc.SetPadding(Thickness(10.0f));
        cc.SetContent("按鈕內容文字");

        assert(cc.GetContent() != nullptr);
        assert(cc.GetContentText() == "按鈕內容文字");

        cc.Measure(Size(400.0f, 300.0f));
        Size desired = cc.GetDesiredSize();
        // Desired size 必須大於 Padding (20x20)
        assert(desired.width > 20.0f);
        assert(desired.height > 20.0f);

        // 動態更換為自訂子元件
        auto customChild = std::make_shared<UIElement>();
        customChild->SetWidth(80.0f);
        customChild->SetHeight(40.0f);
        cc.SetContent(customChild);

        assert(cc.GetContent() == customChild);
        cc.Measure(Size(400.0f, 300.0f));
        // Desired = 80 + 20, 40 + 20 = 100 x 60
        assert(cc.GetDesiredSize().width == 100.0f);
        assert(cc.GetDesiredSize().height == 60.0f);

        cc.Arrange(Rect(0.0f, 0.0f, 100.0f, 60.0f));
        assert(customChild->GetRenderBounds().x == 10.0f);
        assert(customChild->GetRenderBounds().y == 10.0f);

        compat::println("  [PASS] ContentControl string and UIElement content verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 3] Button Click State Machine & Keyboard Activation
    // -------------------------------------------------------------------------
    compat::println("[Test 3] Button Click State Machine & Keyboard Activation...");
    {
        auto btn = std::make_shared<Button>("點擊我");
        btn->SetVisualBounds(Rect(0.0f, 0.0f, 120.0f, 40.0f));

        int clickCount = 0;
        btn->OnClick([&](Button& b) {
            assert(&b == btn.get());
            clickCount++;
        });

        // 1. 滑鼠按下於按鈕內部 (50, 20)
        PointerEventArgs pDown(StandardEvents::PointerPressed, btn.get(), Point(50.0f, 20.0f), pal::RawMouseButton::Left);
        EventRoute rDown(btn.get());
        rDown.Dispatch(StandardEvents::PreviewPointerPressed, StandardEvents::PointerPressed, pDown);
        assert(btn->IsPressed());

        // 2. 滑鼠放開於按鈕內部 -> 觸發 Click
        PointerEventArgs pUp(StandardEvents::PointerReleased, btn.get(), Point(50.0f, 20.0f), pal::RawMouseButton::Left);
        EventRoute rUp(btn.get());
        rUp.Dispatch(StandardEvents::PreviewPointerReleased, StandardEvents::PointerReleased, pUp);
        assert(!btn->IsPressed());
        assert(clickCount == 1);

        // 3. 鍵盤空白鍵觸發點擊
        KeyEventArgs kSpace(StandardEvents::KeyDown, btn.get(), 0x20, 0, false, pal::KeyModifiers::None);
        EventRoute rKey(btn.get());
        rKey.Dispatch(StandardEvents::PreviewKeyDown, StandardEvents::KeyDown, kSpace);
        assert(clickCount == 2);

        // 4. 停用按鈕 (Disabled) 阻止點擊
        btn->SetEnabled(false);
        assert(!btn->IsEnabled());
        EventRoute rKey2(btn.get());
        rKey2.Dispatch(StandardEvents::PreviewKeyDown, StandardEvents::KeyDown, kSpace);
        assert(clickCount == 2); // 次數不變

        compat::println("  [PASS] Button click and keyboard activation verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 4] CheckBox Toggle & Checked/Unchecked Events
    // -------------------------------------------------------------------------
    compat::println("[Test 4] CheckBox Toggle & Checked/Unchecked Events...");
    {
        auto cb = std::make_shared<CheckBox>("記住密碼");
        assert(!cb->IsChecked());

        bool lastCheckedState = false;
        int eventCount = 0;

        cb->OnCheckedChanged([&](bool isChecked) {
            lastCheckedState = isChecked;
            eventCount++;
        });

        // 1. 切換至勾選狀態
        cb->Toggle();
        assert(cb->IsChecked());
        assert(lastCheckedState == true);
        assert(eventCount == 1);

        // 2. 再次切換回未勾選狀態
        cb->Toggle();
        assert(!cb->IsChecked());
        assert(lastCheckedState == false);
        assert(eventCount == 2);

        // 3. 測量排版尺寸（包含 18px 方框 + 8px 間距 + 文字寬度）
        cb->Measure(Size(500.0f, 100.0f));
        Size cbDesired = cb->GetDesiredSize();
        assert(cbDesired.width > 26.0f);
        assert(cbDesired.height >= 18.0f);

        compat::println("  [PASS] CheckBox toggle, events, and layout verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 5] TextBox Typing, Caret & TextChanged Event
    // -------------------------------------------------------------------------
    compat::println("[Test 5] TextBox Typing, Caret & TextChanged Event...");
    {
        auto tb = std::make_shared<TextBox>("請輸入名稱...");
        assert(tb->GetText().empty());
        assert(tb->GetPlaceholder() == "請輸入名稱...");

        std::string textLog;
        tb->OnTextChanged([&](compat::string_view txt) {
            textLog = std::string(txt.data(), txt.size());
        });

        // 1. 模擬輸入文字 "CPP"
        TextInputEventArgs evText1(StandardEvents::TextInput, tb.get(), 0, "CPP");
        EventRoute r1(tb.get());
        r1.DispatchBubble(StandardEvents::TextInput, evText1);

        assert(tb->GetText() == "CPP");
        assert(tb->GetCaretIndex() == 3);
        assert(textLog == "CPP");

        // 2. 模擬按鍵退格鍵 Backspace (0x08)
        KeyEventArgs evBack(StandardEvents::KeyDown, tb.get(), 0x08, 0, false, pal::KeyModifiers::None);
        EventRoute r2(tb.get());
        r2.Dispatch(StandardEvents::PreviewKeyDown, StandardEvents::KeyDown, evBack);

        assert(tb->GetText() == "CP");
        assert(tb->GetCaretIndex() == 2);
        assert(textLog == "CP");

        // 3. 模擬向左鍵 (0x25) 移動游標
        KeyEventArgs evLeft(StandardEvents::KeyDown, tb.get(), 0x25, 0, false, pal::KeyModifiers::None);
        EventRoute r3(tb.get());
        r3.Dispatch(StandardEvents::PreviewKeyDown, StandardEvents::KeyDown, evLeft);
        assert(tb->GetCaretIndex() == 1);

        compat::println("  [PASS] TextBox text insertion, backspace, and caret movement verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 6] Grid Pixel, Auto, Star Layout & Attached Properties
    // -------------------------------------------------------------------------
    compat::println("[Test 6] Grid Pixel, Auto, Star Layout & Attached Properties...");
    {
        auto grid = std::make_shared<Grid>();

        // 定義 2 列：Row 0 = Pixel(60), Row 1 = Star(1.0f)
        grid->AddRow(RowDefinition(GridLength::Pixel(60.0f)));
        grid->AddRow(RowDefinition(GridLength::Star(1.0f)));

        // 定義 2 欄：Col 0 = Pixel(100), Col 1 = Star(2.0f)
        grid->AddColumn(ColumnDefinition(GridLength::Pixel(100.0f)));
        grid->AddColumn(ColumnDefinition(GridLength::Star(2.0f)));

        auto cell00 = std::make_shared<UIElement>();
        auto cell01 = std::make_shared<UIElement>();
        auto cell10 = std::make_shared<UIElement>();
        auto cell11 = std::make_shared<UIElement>();

        Grid::SetRow(*cell00, 0); Grid::SetColumn(*cell00, 0);
        Grid::SetRow(*cell01, 0); Grid::SetColumn(*cell01, 1);
        Grid::SetRow(*cell10, 1); Grid::SetColumn(*cell10, 0);
        Grid::SetRow(*cell11, 1); Grid::SetColumn(*cell11, 1);

        grid->AddChild(cell00);
        grid->AddChild(cell01);
        grid->AddChild(cell10);
        grid->AddChild(cell11);

        // 在 400 x 300 畫布上執行 Measure 與 Arrange
        grid->Measure(Size(400.0f, 300.0f));
        grid->Arrange(Rect(0.0f, 0.0f, 400.0f, 300.0f));

        // 預期欄寬：Col 0 = 100, Col 1 = (400 - 100) = 300
        // 預期列高：Row 0 = 60,  Row 1 = (300 - 60) = 240

        // Cell (0, 0)
        assert(cell00->GetRenderBounds().x == 0.0f);
        assert(cell00->GetRenderBounds().y == 0.0f);
        assert(cell00->GetRenderBounds().width == 100.0f);
        assert(cell00->GetRenderBounds().height == 60.0f);

        // Cell (0, 1)
        assert(cell01->GetRenderBounds().x == 100.0f);
        assert(cell01->GetRenderBounds().y == 0.0f);
        assert(cell01->GetRenderBounds().width == 300.0f);
        assert(cell01->GetRenderBounds().height == 60.0f);

        // Cell (1, 1)
        assert(cell11->GetRenderBounds().x == 100.0f);
        assert(cell11->GetRenderBounds().y == 60.0f);
        assert(cell11->GetRenderBounds().width == 300.0f);
        assert(cell11->GetRenderBounds().height == 240.0f);

        compat::println("  [PASS] Grid multi-cell proportional allocation verified.");
    }

    compat::println("=================================================");
    compat::println(" ALL M6 CONTROLS & GRID TESTS PASSED!            ");
    compat::println("=================================================");
    return 0;
}
