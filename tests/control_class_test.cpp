/**
 * @file control_class_test.cpp
 * @brief 補齊之 Control 基礎類別全功能規格對標單元測試
 *        涵蓋 Name/Tag、幾何邊界與客戶區、狀態與焦點、親緣樹與 Z-Order、
 *        佈局生命週期、文字協定、字型環境繼承、GDI+ 繪圖互操作與 Fluent DSL。
 */

#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/app/Form.hpp"
#include "ui/dsl/Fluent.hpp"
#include "ui/graphics/Graphics.hpp"
#include "ui/graphics/Colors.hpp"
#include "ui/graphics/Pens.hpp"
#include "ui/graphics/Brushes.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

// 自訂測試使用者資料結構
struct TestUserData {
    int32_t     userId = 0;
    std::string roleName;
};

// 自訂 GDI+ 繪圖控制項
class CustomCanvasControl : public cppui::controls::Control {
public:
    bool wasPaintCalled = false;

    void OnPaint(cppui::graphics::Graphics& g) override {
        wasPaintCalled = true;
        g.DrawLine(cppui::graphics::Pens::Red(), 0.0f, 0.0f, 50.0f, 50.0f);
        g.FillRectangle(cppui::graphics::Brushes::Blue(), 10.0f, 10.0f, 30.0f, 30.0f);
    }
};

// =============================================================================
// 1. 識別與使用者自訂資料 (Name & Tag)
// =============================================================================
static void TestNameAndTag() {
    compat::println("[Test] 測試 1: 識別與使用者自訂資料 (Name & Tag)...");

    cppui::controls::Control ctrl;
    assert(ctrl.GetName().empty());
    assert(ctrl.GetTag() == nullptr);

    ctrl.SetName("btnSubmit");
    assert(ctrl.GetName() == "btnSubmit");

    auto data = std::make_shared<TestUserData>();
    data->userId = 1001;
    data->roleName = "Administrator";

    ctrl.SetTag(data);
    assert(ctrl.GetTag() != nullptr);

    auto retrieved = ctrl.GetTag<TestUserData>();
    assert(retrieved != nullptr);
    assert(retrieved->userId == 1001);
    assert(retrieved->roleName == "Administrator");

    compat::println("  ✔ Name 與 Tag 泛型轉型驗證通過！");
}

// =============================================================================
// 2. 幾何尺寸、邊界、客戶區與座標轉換
// =============================================================================
static void TestBoundsAndCoordinates() {
    compat::println("[Test] 測試 2: 幾何尺寸、邊界、客戶區與座標轉換...");

    cppui::controls::Control ctrl;
    ctrl.SetBounds(15.0f, 25.0f, 300.0f, 150.0f);

    assert(ctrl.GetLeft() == 15.0f);
    assert(ctrl.GetTop() == 25.0f);
    assert(ctrl.GetRight() == 315.0f);
    assert(ctrl.GetBottom() == 175.0f);

    cppui::core::Size sz = ctrl.GetSize();
    assert(sz.width == 300.0f && sz.height == 150.0f);
    (void)sz;

    cppui::core::Rect bounds = ctrl.GetBounds();
    assert(bounds.x == 15.0f && bounds.y == 25.0f && bounds.width == 300.0f && bounds.height == 150.0f);
    (void)bounds;

    // 測試內距與邊框對客戶區之扣除
    ctrl.SetPadding(10.0f);
    ctrl.SetBorderThickness(2.0f);

    // 橫向扣除 (10*2 + 2*2) = 24, 縱向扣除 24
    cppui::core::Size clientSz = ctrl.GetClientSize();
    assert(clientSz.width == 276.0f);
    assert(clientSz.height == 126.0f);
    (void)clientSz;

    cppui::core::Rect clientRect = ctrl.GetClientRectangle();
    assert(clientRect.x == 0.0f && clientRect.y == 0.0f);
    assert(clientRect.width == 276.0f && clientRect.height == 126.0f);
    (void)clientRect;

    // 測試矩形轉換
    cppui::core::Rect scrRect = ctrl.RectangleToScreen(clientRect);
    assert(scrRect.width == 276.0f && scrRect.height == 126.0f);

    cppui::core::Rect backToClient = ctrl.RectangleToClient(scrRect);
    assert(backToClient.x == clientRect.x && backToClient.y == clientRect.y);
    (void)backToClient;

    // 測試 Anchor 與 Dock
    ctrl.SetAnchor(cppui::controls::AnchorStyles::Bottom | cppui::controls::AnchorStyles::Right);
    assert((ctrl.GetAnchor() & cppui::controls::AnchorStyles::Bottom) == cppui::controls::AnchorStyles::Bottom);
    assert((ctrl.GetAnchor() & cppui::controls::AnchorStyles::Right) == cppui::controls::AnchorStyles::Right);
    assert((ctrl.GetAnchor() & cppui::controls::AnchorStyles::Top) == cppui::controls::AnchorStyles::None);

    ctrl.SetDock(cppui::controls::DockStyle::Fill);
    assert(ctrl.GetDock() == cppui::controls::DockStyle::Fill);

    // 測試 Cursor
    ctrl.SetCursor(cppui::controls::CursorType::Hand);
    assert(ctrl.GetCursor() == cppui::controls::CursorType::Hand);

    compat::println("  ✔ 邊界、幾何計算、客戶區與 Cursor/Anchor/Dock 驗證通過！");
}

// =============================================================================
// 3. 可用性、可見性與狀態切換
// =============================================================================
static void TestStateAndVisibility() {
    compat::println("[Test] 測試 3: 可用性、可見性與狀態切換...");

    cppui::controls::Control ctrl;
    assert(ctrl.IsEnabled());
    assert(ctrl.IsVisible());

    bool enabledChangedTriggered = false;
    ctrl.OnEnabledChanged([&enabledChangedTriggered](bool e) {
        if (!e) enabledChangedTriggered = true;
    });

    ctrl.SetEnabled(false);
    assert(!ctrl.IsEnabled());
    assert(enabledChangedTriggered);

    // 禁用時嘗試獲焦應失敗
    assert(!ctrl.Focus());

    bool visibleChangedTriggered = false;
    ctrl.OnVisibleChanged([&visibleChangedTriggered](bool v) {
        if (!v) visibleChangedTriggered = true;
    });

    ctrl.Hide();
    assert(!ctrl.IsVisible());
    assert(visibleChangedTriggered);

    ctrl.Show();
    assert(ctrl.IsVisible());

    compat::println("  ✔ 可用性、可見性切換與事件觸發驗證通過！");
}

// =============================================================================
// 4. 親緣樹導航、Z-Order 與 Form 搜尋
// =============================================================================
static void TestHierarchyAndZOrder() {
    compat::println("[Test] 測試 4: 親緣樹導航、Z-Order 與 Form 搜尋...");

    auto form = std::make_shared<cppui::app::Form>();
    auto panel = std::make_shared<cppui::controls::Control>();
    auto child1 = std::make_shared<cppui::controls::Control>();
    auto child2 = std::make_shared<cppui::controls::Control>();

    child1->SetName("Child1");
    child2->SetName("Child2");

    panel->AddVisualChild(child1);
    panel->AddVisualChild(child2);
    form->SetContent(panel);

    // 驗證 FindForm
    assert(child1->FindForm() == form.get());
    assert(child2->FindForm() == form.get());
    assert(panel->FindForm() == form.get());

    // 驗證 GetParent
    assert(child1->GetParent() == panel.get());
    assert(child2->GetParent() == panel.get());

    // 驗證 Contains
    assert(panel->Contains(child1.get()));
    assert(panel->Contains(child2.get()));

    // 驗證 GetChildControls
    auto children = panel->GetChildControls();
    assert(children.size() == 2);
    assert(children[0] == child1.get());
    assert(children[1] == child2.get());

    // 驗證 Z-Order: BringToFront (將 child1 移至頂層)
    child1->BringToFront();
    children = panel->GetChildControls();
    assert(children[0] == child2.get());
    assert(children[1] == child1.get());

    // 驗證 Z-Order: SendToBack (將 child1 移回底層)
    child1->SendToBack();
    children = panel->GetChildControls();
    assert(children[0] == child1.get());
    assert(children[1] == child2.get());

    // 驗證 BringVisualChildToFront 空指標防護
    bool threwNull = false;
    try {
        panel->BringVisualChildToFront(nullptr);
    } catch (const std::invalid_argument&) {
        threwNull = true;
    }
    assert(threwNull);
    (void)threwNull;

    compat::println("  ✔ FindForm, GetParent, Contains 與 Z-Order 調整驗證通過！");
}

// =============================================================================
// 5. 佈局生命週期掛起與恢復 (SuspendLayout / ResumeLayout)
// =============================================================================
static void TestLayoutLifecycle() {
    compat::println("[Test] 測試 5: 佈局生命週期掛起與恢復...");

    cppui::controls::Control ctrl;
    assert(!ctrl.IsLayoutSuspended());

    ctrl.SuspendLayout();
    assert(ctrl.IsLayoutSuspended());

    // 在掛起期間進行多次屬性更動
    ctrl.SetWidth(100.0f);
    ctrl.SetHeight(50.0f);
    ctrl.SetPadding(8.0f);

    // 恢復並立即執行排版
    ctrl.ResumeLayout(true);
    assert(!ctrl.IsLayoutSuspended());

    ctrl.PerformLayout();

    compat::println("  ✔ SuspendLayout 與 ResumeLayout 佈局批次控制驗證通過！");
}

// =============================================================================
// 6. 文字協定與多型同步 (Text, ContentControl, TextBox, Button)
// =============================================================================
static void TestTextProtocol() {
    compat::println("[Test] 測試 6: 文字協定與多型同步...");

    // 1. 基底 Control 文字
    cppui::controls::Control ctrl;
    bool ctrlTextTriggered = false;
    ctrl.OnTextChanged([&ctrlTextTriggered](compat::string_view t) {
        if (t == "Hello Base") ctrlTextTriggered = true;
    });
    ctrl.SetText("Hello Base");
    assert(ctrl.GetText() == "Hello Base");
    assert(ctrlTextTriggered);

    // 2. Button (繼承自 ContentControl)
    auto btn = std::make_shared<cppui::controls::Button>("Click Here");
    assert(btn->GetText() == "Click Here");

    // 透過多型指標操作
    cppui::controls::Control* polyCtrl = btn.get();
    polyCtrl->SetText("Submit Form");
    assert(btn->GetText() == "Submit Form");
    assert(btn->GetContentText() == "Submit Form");

    // 3. TextBox 文字多型
    auto tbox = std::make_shared<cppui::controls::TextBox>();
    cppui::controls::Control* polyTBox = tbox.get();
    polyTBox->SetText("Input Content");
    assert(tbox->GetText() == "Input Content");

    compat::println("  ✔ Control*, Button, TextBox 文字屬性多型存取驗證通過！");
}

// =============================================================================
// 7. 字型環境屬性繼承 (Ambient Font Inheritance)
// =============================================================================
static void TestAmbientFont() {
    compat::println("[Test] 測試 7: 字型環境屬性繼承 (Ambient Font)...");

    auto parent = std::make_shared<cppui::controls::Control>();
    auto child = std::make_shared<cppui::controls::Control>();

    parent->AddVisualChild(child);

    // 未設定前預設為 Segoe UI, 14.0f
    assert(child->GetFontFamily() == "Segoe UI");
    assert(child->GetFontSize() == 14.0f);

    // 父層設定字型
    parent->SetFontFamily("Consolas");
    parent->SetFontSize(18.0f);
    parent->SetFontWeight(cppui::text::FontWeight::Bold);

    // 子層自動向上繼承
    assert(child->GetFontFamily() == "Consolas");
    assert(child->GetFontSize() == 18.0f);
    assert(child->GetFontWeight() == cppui::text::FontWeight::Bold);
    assert(!child->HasExplicitFont());

    // 子層顯式覆寫部分屬性
    child->SetFontSize(24.0f);
    assert(child->HasExplicitFont());
    assert(child->GetFontSize() == 24.0f);
    assert(child->GetFontFamily() == "Consolas"); // 家族仍繼承自父層

    compat::println("  ✔ 字型環境屬性向上繼承與區域覆寫驗證通過！");
}

// =============================================================================
// 8. GDI+ 圖形互操作與 Fluent DSL 擴充
// =============================================================================
static void TestGraphicsAndPainting() {
    compat::println("[Test] 測試 8: GDI+ 圖形互操作與 Fluent DSL 擴充...");

    // 1. CreateGraphics 測試
    CustomCanvasControl canvas;
    canvas.SetSize(100.0f, 100.0f);

    auto g = canvas.CreateGraphics();
    assert(g != nullptr);
    g->Clear(cppui::graphics::Colors::White);
    g->DrawLine(cppui::graphics::Pens::Black(), 0.0f, 0.0f, 100.0f, 100.0f);

    // 2. OnPaint 覆寫與調用
    cppui::rendering::DisplayList dl;
    cppui::rendering::DrawingContext dc(dl);
    canvas.OnRender(dc);
    assert(canvas.wasPaintCalled);

    // 3. OnPaintHandler 回呼
    bool paintHandlerCalled = false;
    canvas.OnPaintHandler([&paintHandlerCalled](cppui::graphics::Graphics&) {
        paintHandlerCalled = true;
    });
    canvas.OnRender(dc);
    assert(paintHandlerCalled);

    // 4. Fluent DSL 鏈式建立與配置
    using namespace cppui::dsl;
    auto myBtn = Button()
        .Name("fluentSubmit")
        .Text("Confirm")
        .Enabled(true)
        .Cursor(cppui::controls::CursorType::Hand)
        .Anchor(cppui::controls::AnchorStyles::Top | cppui::controls::AnchorStyles::Left)
        .Bounds(10.0f, 10.0f, 120.0f, 35.0f)
        .Font(cppui::text::FontDescriptor("Arial", 15.0f, cppui::text::FontWeight::SemiBold));

    assert(myBtn->GetName() == "fluentSubmit");
    assert(myBtn->GetText() == "Confirm");
    assert(myBtn->IsEnabled());
    assert(myBtn->GetCursor() == cppui::controls::CursorType::Hand);
    assert(myBtn->GetLeft() == 10.0f && myBtn->GetTop() == 10.0f);
    assert(myBtn->GetFontFamily() == "Arial");
    assert(myBtn->GetFontSize() == 15.0f);

    compat::println("  ✔ CreateGraphics, OnPaint, OnPaintHandler 與 Fluent DSL 驗證通過！");
}

// =============================================================================
// Main
// =============================================================================
int main() {
    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Windows Control Class Complete Feature Test ");
    compat::println("==========================================================");

    TestNameAndTag();
    TestBoundsAndCoordinates();
    TestStateAndVisibility();
    TestHierarchyAndZOrder();
    TestLayoutLifecycle();
    TestTextProtocol();
    TestAmbientFont();
    TestGraphicsAndPainting();

    compat::println("==========================================================");
    compat::println(" ✔ Control 類別規格對標與全功能單元測試 100% 通過！        ");
    compat::println("==========================================================");
    return 0;
}
