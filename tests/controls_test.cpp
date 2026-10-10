/**
 * @file controls_test.cpp
 * @brief 控制項基礎架構、ContentControl、Button、CheckBox、TextBox 與 Grid 排版單元測試
 */

#include <cassert>
#include <cstdint>
#include <string>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/ScrollViewer.hpp"
#include "ui/controls/PictureBox.hpp"
#include "ui/controls/RadioButton.hpp"
#include "ui/controls/ProgressBar.hpp"
#include "ui/controls/Slider.hpp"
#include "ui/controls/ListBox.hpp"
#include "ui/controls/Separator.hpp"
#include "ui/controls/Label.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "../src/rendering/backend/SoftwareRenderContext.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/EventRoute.hpp"

int main() {
    using namespace cppui;
    using namespace cppui::core;
    using namespace cppui::controls;
    using namespace cppui::layout;
    using namespace cppui::input;
    using namespace cppui::app;

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

        int32_t clickCount = 0;
        btn->OnClick([&](Button& b) {
            assert(&b == btn.get());
            (void)b;
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
        int32_t eventCount = 0;

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

    // -------------------------------------------------------------------------
    // [Test 7] Fail-Fast Argument Null Validation
    // -------------------------------------------------------------------------
    compat::println("[Test 7] Fail-Fast Argument Null Validation...");
    {
        // 1. ContentControl::SetContent(nullptr) 應擲出 std::invalid_argument
        bool ccNullCaught = false;
        try {
            ContentControl cc;
            std::shared_ptr<UIElement> nullElem;
            cc.SetContent(nullElem);
        } catch (const std::invalid_argument&) {
            ccNullCaught = true;
        }
        assert(ccNullCaught);

        // 2. Border::SetChild(nullptr) 應擲出 std::invalid_argument
        bool borderNullCaught = false;
        try {
            Border b;
            b.SetChild(nullptr);
        } catch (const std::invalid_argument&) {
            borderNullCaught = true;
        }
        assert(borderNullCaught);

        // 3. Panel::AddChild(nullptr) 應擲出 std::invalid_argument
        bool panelAddNullCaught = false;
        try {
            Panel p;
            p.AddChild(nullptr);
        } catch (const std::invalid_argument&) {
            panelAddNullCaught = true;
        }
        assert(panelAddNullCaught);

        // 4. Panel::RemoveChild(nullptr) 應擲出 std::invalid_argument
        bool panelRemoveNullCaught = false;
        try {
            Panel p;
            p.RemoveChild(nullptr);
        } catch (const std::invalid_argument&) {
            panelRemoveNullCaught = true;
        }
        assert(panelRemoveNullCaught);

        // 5. Button::SetCommand(nullptr) 應擲出 std::invalid_argument
        bool btnCommandNullCaught = false;
        try {
            Button btn("Cmd");
            btn.SetCommand(nullptr);
        } catch (const std::invalid_argument&) {
            btnCommandNullCaught = true;
        }
        assert(btnCommandNullCaught);

        compat::println("  [PASS] Fail-Fast contracts verified for Control, Border, Panel, and Button.");
    }

    // -------------------------------------------------------------------------
    // [Test 8] Button Content Centering & TextBlock Default Wrapping
    // -------------------------------------------------------------------------
    compat::println("[Test 8] Button Content Centering & TextBlock Default Wrapping...");
    {
        TextBlock tb;
        assert(tb.GetTextWrapping() == text::TextWrapping::NoWrap);

        auto btn = std::make_shared<Button>("OK");
        btn->SetPadding(Thickness(10.0f));
        btn->SetBorderThickness(Thickness(0.0f));
        btn->SetWidth(100.0f);
        btn->SetHeight(40.0f);

        auto contentElement = std::make_shared<UIElement>();
        contentElement->SetWidth(40.0f);
        contentElement->SetHeight(20.0f);
        btn->SetContent(contentElement);

        btn->Measure(Size(100.0f, 40.0f));
        btn->Arrange(Rect(0.0f, 0.0f, 100.0f, 40.0f));

        // Inner width = 100 - 20 = 80; content = 40. Centering offset = (80 - 40) / 2 = 20.
        // Left offset = 10 (padding) + 20 = 30.
        // Inner height = 40 - 20 = 20; content = 20. Centering offset = (20 - 20) / 2 = 0.
        // Top offset = 10 (padding) + 0 = 10.
        assert(contentElement->GetRenderBounds().x == 30.0f);
        assert(contentElement->GetRenderBounds().y == 10.0f);
        assert(contentElement->GetRenderBounds().width == 40.0f);
        assert(contentElement->GetRenderBounds().height == 20.0f);

        compat::println("  [PASS] Button content centering and TextBlock NoWrap verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 9] Button Content Alignments (Left / Right / Stretch) & Height Protection
    // -------------------------------------------------------------------------
    compat::println("[Test 9] Button Content Alignments & Tight Height Protection...");
    {
        auto btn = std::make_shared<Button>("AlignTest");
        btn->SetPadding(Thickness(10.0f));
        btn->SetBorderThickness(Thickness(0.0f));
        btn->SetWidth(100.0f);
        btn->SetHeight(40.0f);

        auto contentElement = std::make_shared<UIElement>();
        contentElement->SetWidth(40.0f);
        contentElement->SetHeight(20.0f);
        btn->SetContent(contentElement);

        // 1. Horizontal Left
        btn->SetHorizontalContentAlignment(layout::HorizontalAlignment::Left);
        btn->Measure(Size(100.0f, 40.0f));
        btn->Arrange(Rect(0.0f, 0.0f, 100.0f, 40.0f));
        assert(contentElement->GetRenderBounds().x == 10.0f); // Left padding = 10

        // 2. Horizontal Right
        btn->SetHorizontalContentAlignment(layout::HorizontalAlignment::Right);
        btn->InvalidateArrange();
        btn->Arrange(Rect(0.0f, 0.0f, 100.0f, 40.0f));
        // availW = 80, contentW = 40. Right offset = 10 + 80 - 40 = 50.
        assert(contentElement->GetRenderBounds().x == 50.0f);

        // 3. Horizontal Stretch
        contentElement->SetWidth(-1.0f);
        btn->SetHorizontalContentAlignment(layout::HorizontalAlignment::Stretch);
        btn->InvalidateMeasure();
        btn->InvalidateArrange();
        btn->Measure(Size(100.0f, 40.0f));
        btn->Arrange(Rect(0.0f, 0.0f, 100.0f, 40.0f));
        assert(contentElement->GetRenderBounds().x == 10.0f);
        assert(contentElement->GetRenderBounds().width == 80.0f);
        contentElement->SetWidth(40.0f);

        // 4. Tight Height Protection
        // Button height 24px, padding 10px each side (20px total), inner = 4px.
        // But contentDesired is 16px! Height protection should allow 16px and center vertically.
        btn->SetHeight(24.0f);
        contentElement->SetHeight(16.0f);
        btn->SetVerticalContentAlignment(layout::VerticalAlignment::Center);
        btn->InvalidateMeasure();
        btn->InvalidateArrange();
        btn->Measure(Size(100.0f, 24.0f));
        btn->Arrange(Rect(0.0f, 0.0f, 100.0f, 24.0f));
        assert(contentElement->GetRenderBounds().height == 16.0f);
        assert(contentElement->GetRenderBounds().y == 4.0f); // (24 - 16) / 2 = 4

        compat::println("  [PASS] Button content alignment and height protection verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 10] ScrollViewer Layout, Wheel Event, Offsets, Clamping, Line/Page
    // -------------------------------------------------------------------------
    compat::println("[Test 10] ScrollViewer Layout & Scrolling Behavior...");
    {
        auto scrollViewer = std::make_shared<ScrollViewer>();
        scrollViewer->SetWidth(200.0f);
        scrollViewer->SetHeight(100.0f);

        // Content is 300px tall
        auto contentElement = std::make_shared<UIElement>();
        contentElement->SetWidth(190.0f);
        contentElement->SetHeight(300.0f);
        scrollViewer->SetContent(contentElement);

        scrollViewer->Measure(Size(200.0f, 100.0f));
        scrollViewer->Arrange(Rect(0.0f, 0.0f, 200.0f, 100.0f));

        assert(scrollViewer->GetExtentHeight() == 300.0f);
        assert(scrollViewer->GetViewportHeight() == 100.0f);
        assert(scrollViewer->GetVerticalOffset() == 0.0f);

        // Content offset should start at 0
        assert(contentElement->GetRenderBounds().y == 0.0f);

        // 1. LineDown and LineUp
        scrollViewer->SetScrollStep(30.0f);
        scrollViewer->LineDown();
        scrollViewer->Arrange(Rect(0.0f, 0.0f, 200.0f, 100.0f));
        assert(scrollViewer->GetVerticalOffset() == 30.0f);
        assert(contentElement->GetRenderBounds().y == -30.0f);

        scrollViewer->LineUp();
        scrollViewer->Arrange(Rect(0.0f, 0.0f, 200.0f, 100.0f));
        assert(scrollViewer->GetVerticalOffset() == 0.0f);

        // 2. PageDown and PageUp
        scrollViewer->PageDown();
        assert(scrollViewer->GetVerticalOffset() == 100.0f);

        // 3. ScrollToEnd and Clamping
        scrollViewer->ScrollToEnd();
        // maxOffset = 300 - 100 = 200.
        assert(scrollViewer->GetVerticalOffset() == 200.0f);

        scrollViewer->SetVerticalOffset(9999.0f);
        assert(scrollViewer->GetVerticalOffset() == 200.0f);

        scrollViewer->SetVerticalOffset(-50.0f);
        assert(scrollViewer->GetVerticalOffset() == 0.0f);

        // 4. Mouse wheel event simulation
        scrollViewer->ScrollToHome();
        PointerEventArgs wheelArgs(
            StandardEvents::PointerWheel,
            scrollViewer.get(),
            Point(50.0f, 50.0f),
            pal::RawMouseButton::None,
            pal::KeyModifiers::None,
            0,
            0,
            -120 // Scroll down 1 notch
        );
        EventRoute route(scrollViewer.get());
        route.DispatchBubble(StandardEvents::PointerWheel, wheelArgs);
        assert(wheelArgs.IsHandled());
        assert(scrollViewer->GetVerticalOffset() == 30.0f);

        compat::println("  [PASS] ScrollViewer layout, wheel dispatch, and clamping verified.");

        // 5. ScrollViewer Render Clipping Verification (未捲出的文字不應繪製至可視視區外部)
        {
            auto sv = std::make_shared<ScrollViewer>();
            sv->SetWidth(200.0f);
            sv->SetHeight(100.0f);

            auto stack = std::make_shared<layout::StackPanel>(layout::Orientation::Vertical);
            for (size_t i = 0; i < 10; ++i) {
                auto tb = std::make_shared<TextBlock>("Long Item Text Line");
                tb->SetHeight(30.0f);
                stack->AddChild(tb);
            }
            sv->SetContent(stack);

            sv->Measure(Size(200.0f, 100.0f));
            sv->Arrange(Rect(0.0f, 0.0f, 200.0f, 100.0f));

            rendering::DisplayList svDl;
            rendering::DrawingContext svDc(svDl);
            sv->Render(svDc);

            rendering::SoftwareRenderTarget svTarget;
            svTarget.Resize(200, 300);
            svTarget.Clear(Colors::Black);

            rendering::DisplayListRenderer svRenderer;
            svRenderer.Render(svDl, &svTarget);

            const uint32_t* svPixels = svTarget.GetPixelBuffer();
            uint32_t blackVal = Colors::Black.ToArgb();

            // 驗證 ScrollViewer 底部 (y >= 100) 絕對沒有未捲出之文字像素溢出
            bool outsideZeroBleed = true;
            for (int32_t y = 100; y < 300; ++y) {
                for (int32_t x = 0; x < 200; ++x) {
                    if (svPixels[static_cast<size_t>(y) * 200 + x] != blackVal) {
                        outsideZeroBleed = false;
                        break;
                    }
                }
            }
            assert(outsideZeroBleed && "Text outside ScrollViewer viewport must be clipped!");
            compat::println("  [PASS] ScrollViewer text clipping verified (no un-scrolled text drawn outside viewport).");
        }
    }

    // -------------------------------------------------------------------------
    // [Test 7] Control Identity, Hash Code and std::hash Specialization
    // -------------------------------------------------------------------------
    compat::println("[Test 7] Control Identity, Hash Code and std::hash...");
    {
        auto btn1 = std::make_shared<Button>("Button 1");
        auto btn2 = std::make_shared<Button>("Button 2");
        auto btn3 = std::make_shared<Button>("Button 1"); // 相同文字不同實例

        // 1. GetHashCode 確定性驗證
        size_t hash1 = btn1->GetHashCode();
        assert(hash1 == btn1->GetHashCode());

        // 2. 屬性變更不影響雜湊值 (避免破壞 hash table)
        btn1->SetText("New Button Title");
        btn1->SetSize(200.0f, 50.0f);
        assert(btn1->GetHashCode() == hash1);

        // 3. 實例等價性運算子
        assert(*btn1 == *btn1);
        assert(*btn1 != *btn2);
        assert(*btn1 != *btn3); // 即使文字或屬性相同，不同實例即不相等
        assert(!(*btn1 == *btn2));

        // 4. std::hash<Control> 函式物件特化驗證
        std::hash<Control> hasher;
        assert(hasher(*btn1) == hash1);
        assert(hasher(*btn2) == btn2->GetHashCode());
        (void)hash1;
        (void)hasher;

        // 5. 標準容器整合測試: std::unordered_set<std::reference_wrapper<Control>>
        std::unordered_set<std::reference_wrapper<Control>> ctrlSet;
        ctrlSet.insert(std::ref(*btn1));
        ctrlSet.insert(std::ref(*btn2));
        assert(ctrlSet.size() == 2);
        assert(ctrlSet.find(std::ref(*btn1)) != ctrlSet.end());
        assert(ctrlSet.find(std::ref(*btn3)) == ctrlSet.end());

        // 6. 標準容器整合測試: std::unordered_map<std::reference_wrapper<Control>, std::string>
        std::unordered_map<std::reference_wrapper<Control>, std::string> ctrlMap;
        ctrlMap.emplace(std::ref(*btn1), "Action1");
        ctrlMap.emplace(std::ref(*btn2), "Action2");
        assert(ctrlMap.at(std::ref(*btn1)) == "Action1");
        assert(ctrlMap.at(std::ref(*btn2)) == "Action2");

        compat::println("  [PASS] Control GetHashCode, equality operators, and std::hash verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 8] Button Expanded Methods & DialogResult
    // -------------------------------------------------------------------------
    compat::println("[Test 8] Button Expanded Methods & DialogResult...");
    {
        Button btn("OK");
        assert(!btn.IsDefault());
        assert(!btn.IsCancel());
        assert(btn.GetDialogResult() == DialogResult::None);

        btn.SetIsDefault(true);
        assert(btn.IsDefault());
        btn.SetIsCancel(true);
        assert(btn.IsCancel());
        btn.SetDialogResult(DialogResult::OK);
        assert(btn.GetDialogResult() == DialogResult::OK);

        bool clicked = false;
        btn.OnClick([&clicked]() { clicked = true; });
        btn.PerformClick();
        assert(clicked);

        compat::println("  [PASS] Button PerformClick, IsDefault, IsCancel, DialogResult verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 9] CheckBox ThreeState & Indeterminate Mode
    // -------------------------------------------------------------------------
    compat::println("[Test 9] CheckBox ThreeState & Indeterminate Mode...");
    {
        CheckBox cb("Option");
        assert(!cb.IsThreeState());
        assert(cb.GetCheckState() == CheckBox::CheckState::Unchecked);
        assert(!cb.IsChecked());

        cb.SetIsThreeState(true);
        assert(cb.IsThreeState());

        // Toggle cycle: Unchecked -> Checked -> Indeterminate -> Unchecked
        cb.Toggle();
        assert(cb.GetCheckState() == CheckBox::CheckState::Checked);
        assert(cb.IsChecked());

        cb.Toggle();
        assert(cb.GetCheckState() == CheckBox::CheckState::Indeterminate);
        assert(!cb.IsChecked());

        cb.Toggle();
        assert(cb.GetCheckState() == CheckBox::CheckState::Unchecked);
        assert(!cb.IsChecked());

        // Direct SetCheckState
        cb.SetCheckState(CheckBox::CheckState::Indeterminate);
        assert(cb.GetCheckState() == CheckBox::CheckState::Indeterminate);

        bool indeterminateFired = false;
        cb.OnIndeterminate([&indeterminateFired]() { indeterminateFired = true; });
        cb.SetCheckState(CheckBox::CheckState::Checked);
        cb.SetCheckState(CheckBox::CheckState::Indeterminate);
        assert(indeterminateFired);

        compat::println("  [PASS] CheckBox ThreeState cycle and Indeterminate event verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 10] TextBox ReadOnly, MaxLength, Selection & Password
    // -------------------------------------------------------------------------
    compat::println("[Test 10] TextBox ReadOnly, MaxLength, Selection & Password...");
    {
        TextBox tb;
        tb.SetText("Hello");
        assert(tb.GetText() == "Hello");

        // ReadOnly
        assert(!tb.IsReadOnly());
        tb.SetReadOnly(true);
        assert(tb.IsReadOnly());
        tb.InsertText(" World");
        assert(tb.GetText() == "Hello"); // insertion ignored when ReadOnly

        tb.SetReadOnly(false);
        tb.InsertText(" World");
        assert(tb.GetText() == "Hello World");

        // MaxLength
        assert(tb.GetMaxLength() == 0);
        tb.SetMaxLength(5);
        assert(tb.GetMaxLength() == 5);
        assert(tb.GetText() == "Hello"); // truncated to 5

        tb.InsertText(" Extra");
        assert(tb.GetText() == "Hello"); // cannot exceed 5

        // Clear
        tb.SetMaxLength(0);
        tb.Clear();
        assert(tb.GetText().empty());

        // Selection
        tb.SetText("Modern C++ GUI");
        tb.Select(0, 6);
        assert(tb.GetSelectionStart() == 0);
        assert(tb.GetSelectionLength() == 6);
        assert(tb.GetSelectedText() == "Modern");

        tb.SelectAll();
        assert(tb.GetSelectionStart() == 0);
        assert(tb.GetSelectionLength() == tb.GetText().size());
        assert(tb.GetSelectedText() == "Modern C++ GUI");

        // Password mode
        assert(!tb.IsPassword());
        tb.SetIsPassword(true);
        assert(tb.IsPassword());
        assert(tb.GetPasswordChar() == '*');
        tb.SetPasswordChar('#');
        assert(tb.GetPasswordChar() == '#');

        compat::println("  [PASS] TextBox ReadOnly, MaxLength, Selection, Password verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 11] ScrollViewer Navigation & ScrollBarVisibility
    // -------------------------------------------------------------------------
    compat::println("[Test 11] ScrollViewer Navigation & ScrollBarVisibility...");
    {
        ScrollViewer sv;
        assert(sv.GetHorizontalScrollBarVisibility() == ScrollViewer::ScrollBarVisibility::Disabled);
        assert(sv.GetVerticalScrollBarVisibility() == ScrollViewer::ScrollBarVisibility::Auto);

        sv.SetHorizontalScrollBarVisibility(ScrollViewer::ScrollBarVisibility::Auto);
        assert(sv.GetHorizontalScrollBarVisibility() == ScrollViewer::ScrollBarVisibility::Auto);

        sv.SetVerticalScrollBarVisibility(ScrollViewer::ScrollBarVisibility::Visible);
        assert(sv.GetVerticalScrollBarVisibility() == ScrollViewer::ScrollBarVisibility::Visible);

        // Child larger than viewport
        auto child = std::make_shared<Control>();
        child->SetSize(800.0f, 600.0f);
        sv.SetContent(child);
        sv.Measure(Size(400.0f, 300.0f));
        sv.Arrange(Rect(0, 0, 400.0f, 300.0f));

        sv.ScrollToBottom();
        assert(sv.GetVerticalOffset() == 300.0f);

        sv.ScrollToTop();
        assert(sv.GetVerticalOffset() == 0.0f);

        sv.ScrollToRightEnd();
        assert(sv.GetHorizontalOffset() == 400.0f);

        sv.ScrollToLeftEnd();
        assert(sv.GetHorizontalOffset() == 0.0f);

        sv.LineRight();
        assert(sv.GetHorizontalOffset() == 32.0f);

        sv.LineLeft();
        assert(sv.GetHorizontalOffset() == 0.0f);

        compat::println("  [PASS] ScrollViewer ScrollBarVisibility & quick navigation verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 12] RadioButton Mutual Exclusion & Grouping
    // -------------------------------------------------------------------------
    compat::println("[Test 12] RadioButton Mutual Exclusion & Grouping...");
    {
        auto panel = std::make_shared<StackPanel>();
        auto rb1 = std::make_shared<RadioButton>("Option 1");
        auto rb2 = std::make_shared<RadioButton>("Option 2");
        auto rb3 = std::make_shared<RadioButton>("Option 3");

        panel->AddChild(rb1);
        panel->AddChild(rb2);
        panel->AddChild(rb3);

        assert(!rb1->IsChecked());
        assert(!rb2->IsChecked());
        assert(!rb3->IsChecked());

        rb1->SetChecked(true);
        assert(rb1->IsChecked());
        assert(!rb2->IsChecked());
        assert(!rb3->IsChecked());

        rb2->SetChecked(true);
        assert(!rb1->IsChecked());
        assert(rb2->IsChecked());
        assert(!rb3->IsChecked());

        // Named groups
        auto rbA1 = std::make_shared<RadioButton>("Group A 1");
        auto rbA2 = std::make_shared<RadioButton>("Group A 2");
        auto rbB1 = std::make_shared<RadioButton>("Group B 1");

        rbA1->SetGroupName("GroupA");
        rbA2->SetGroupName("GroupA");
        rbB1->SetGroupName("GroupB");

        panel->AddChild(rbA1);
        panel->AddChild(rbA2);
        panel->AddChild(rbB1);

        rbA1->SetChecked(true);
        rbB1->SetChecked(true);
        assert(rbA1->IsChecked());
        assert(rbB1->IsChecked());

        rbA2->SetChecked(true);
        assert(!rbA1->IsChecked());
        assert(rbA2->IsChecked());
        assert(rbB1->IsChecked()); // Group B unaffected

        compat::println("  [PASS] RadioButton mutual exclusion and group isolation verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 13] ProgressBar Value Clamping & Step Calculation
    // -------------------------------------------------------------------------
    compat::println("[Test 13] ProgressBar Value Clamping & Step Calculation...");
    {
        ProgressBar pb;
        assert(pb.GetMinimum() == 0.0f);
        assert(pb.GetMaximum() == 100.0f);
        assert(pb.GetValue() == 0.0f);

        pb.SetValue(50.0f);
        assert(pb.GetValue() == 50.0f);

        // Clamp upper and lower bounds
        pb.SetValue(150.0f);
        assert(pb.GetValue() == 100.0f);
        pb.SetValue(-20.0f);
        assert(pb.GetValue() == 0.0f);

        // Step & Increment
        pb.SetStep(25.0f);
        assert(pb.GetStep() == 25.0f);
        pb.PerformStep();
        assert(pb.GetValue() == 25.0f);
        pb.Increment(30.0f);
        assert(pb.GetValue() == 55.0f);

        // Orientation & Indeterminate
        pb.SetOrientation(Orientation::Vertical);
        assert(pb.GetOrientation() == Orientation::Vertical);
        pb.SetIndeterminate(true);
        assert(pb.IsIndeterminate());

        compat::println("  [PASS] ProgressBar value clamping, stepping, and orientation verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 14] Slider Value Clamping, Changes & ValueChanged Event
    // -------------------------------------------------------------------------
    compat::println("[Test 14] Slider Value Clamping, Changes & ValueChanged Event...");
    {
        Slider sl(0.0f, 100.0f, 20.0f);
        assert(sl.GetMinimum() == 0.0f);
        assert(sl.GetMaximum() == 100.0f);
        assert(sl.GetValue() == 20.0f);

        float lastValue = 0.0f;
        sl.OnValueChanged([&lastValue](float val) { lastValue = val; });

        sl.SetValue(75.0f);
        assert(sl.GetValue() == 75.0f);
        assert(lastValue == 75.0f);

        // Clamping
        sl.SetValue(200.0f);
        assert(sl.GetValue() == 100.0f);

        // SmallChange / LargeChange
        sl.SetSmallChange(2.0f);
        sl.SetLargeChange(15.0f);
        assert(sl.GetSmallChange() == 2.0f);
        assert(sl.GetLargeChange() == 15.0f);

        compat::println("  [PASS] Slider value clamping, changes, and ValueChanged event verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 15] ListBox Item Management & Selection
    // -------------------------------------------------------------------------
    compat::println("[Test 15] ListBox Item Management & Selection...");
    {
        ListBox lb;
        assert(lb.GetItemCount() == 0);
        assert(lb.GetSelectedIndex() == -1);
        assert(lb.GetSelectedItem().empty());

        lb.AddItem("Apple");
        lb.AddItem("Banana");
        lb.AddItem("Cherry");
        assert(lb.GetItemCount() == 3);
        assert(lb.GetItem(0) == "Apple");
        assert(lb.GetItem(1) == "Banana");
        assert(lb.GetItem(2) == "Cherry");

        // Selection
        int32_t changedIdx = -999;
        lb.OnSelectedIndexChanged([&changedIdx](int32_t idx) { changedIdx = idx; });

        lb.SetSelectedIndex(1);
        assert(lb.GetSelectedIndex() == 1);
        assert(lb.GetSelectedItem() == "Banana");
        assert(changedIdx == 1);

        lb.SetSelectedItem("Cherry");
        assert(lb.GetSelectedIndex() == 2);
        assert(lb.GetSelectedItem() == "Cherry");
        assert(changedIdx == 2);

        // InsertItem & index shift
        lb.InsertItem(1, "Blueberry");
        assert(lb.GetItemCount() == 4);
        assert(lb.GetItem(1) == "Blueberry");
        assert(lb.GetSelectedIndex() == 3); // shifted from 2 to 3

        // RemoveItem
        lb.RemoveItem(0); // removes Apple
        assert(lb.GetItemCount() == 3);
        assert(lb.GetSelectedIndex() == 2);

        // ClearItems
        lb.ClearItems();
        assert(lb.GetItemCount() == 0);
        assert(lb.GetSelectedIndex() == -1);

        // Fail-fast exceptions
        bool threwOutOfRange = false;
        try {
            (void)lb.GetItem(10);
        } catch (const std::out_of_range&) {
            threwOutOfRange = true;
        }
        assert(threwOutOfRange);

        bool threwInvalidArg = false;
        try {
            lb.SetItemHeight(-5.0f);
        } catch (const std::invalid_argument&) {
            threwInvalidArg = true;
        }
        assert(threwInvalidArg);

        compat::println("  [PASS] ListBox item operations, selection, and exception handling verified.");
    }

    // -------------------------------------------------------------------------
    // [Test 16] Separator, Label & PictureBox Extended Properties
    // -------------------------------------------------------------------------
    compat::println("[Test 16] Separator, Label & PictureBox Extended Properties...");
    {
        // Separator
        Separator sep(Orientation::Vertical);
        assert(sep.GetOrientation() == Orientation::Vertical);
        assert(sep.GetThickness() == 1.0f);
        sep.SetThickness(3.0f);
        assert(sep.GetThickness() == 3.0f);

        bool sepThrew = false;
        try {
            sep.SetThickness(-1.0f);
        } catch (const std::invalid_argument&) {
            sepThrew = true;
        }
        assert(sepThrew);

        // Label
        auto targetBox = std::make_shared<TextBox>();
        Label lbl("Username:");
        assert(lbl.GetText() == "Username:");
        assert(lbl.GetTarget() == nullptr);
        lbl.SetTarget(targetBox.get());
        assert(lbl.GetTarget() == targetBox.get());

        bool labelNullThrew = false;
        try {
            std::shared_ptr<UIElement> nullElem = nullptr;
            Label invalidLabel(nullElem);
        } catch (const std::invalid_argument&) {
            labelNullThrew = true;
        }
        assert(labelNullThrew);

        // PictureBox
        PictureBox pic;
        assert(pic.GetImageLocation().empty());
        pic.SetImageLocation("assets/test.png");
        assert(pic.GetImageLocation() == "assets/test.png");

        bool picNullThrew = false;
        try {
            pic.Load("");
        } catch (const std::invalid_argument&) {
            picNullThrew = true;
        }
        assert(picNullThrew);

        compat::println("  [PASS] Separator, Label, PictureBox properties and fail-fast validation verified.");
    }

    compat::println("=================================================");
    compat::println(" ALL M6 CONTROLS & GRID TESTS PASSED!            ");
    compat::println("=================================================");
    return 0;
}
