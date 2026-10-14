/**
 * @file autosize_test.cpp
 * @brief 視窗自動適配內容尺寸 (Form AutoSize & SizeToContent) 單元測試套件
 */

#include "compat/compat.hpp"
#include "ui/app/Form.hpp"
#include "ui/app/FormEventArgs.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/graphics/Bitmap.hpp"
#include <cassert>
#include <cmath>
#include <memory>

namespace {

/// <summary>
/// 測試預設為手動模式 (SizeToContent::Manual) 與預設尺寸 (800x600)。
/// </summary>
static void TestDefaultManualMode() {
    compat::println("[Test] 測試預設模式 (SizeToContent::Manual)...");

    cppui::app::Form form;
    assert(!form.GetAutoSize());
    assert(form.GetSizeToContent() == cppui::app::SizeToContent::Manual);
    assert(form.GetAutoSizeMode() == cppui::app::AutoSizeMode::GrowAndShrink);
    assert(form.GetFormSize().width == 800.0f);
    assert(form.GetFormSize().height == 600.0f);

    compat::println(" -> 通過: 預設模式與初始尺寸正確。");
}

/// <summary>
/// 測試 SetWidth 與 SetHeight 能正確同步 Form 客戶區尺寸。
/// </summary>
static void TestSetWidthAndHeight() {
    compat::println("[Test] 測試 SetWidth 與 SetHeight 尺寸同步...");

    cppui::app::Form form;
    form.SetWidth(950.0f);
    form.SetHeight(720.0f);

    assert(form.GetFormSize().width == 950.0f);
    assert(form.GetFormSize().height == 720.0f);

    compat::println(" -> 通過: SetWidth 與 SetHeight 正確同步 Form 尺寸。");
}

/// <summary>
/// 測試 SetAutoSize(true) / SizeToContent::WidthAndHeight 依內容自動展開。
/// </summary>
static void TestAutoSizeWidthAndHeight() {
    compat::println("[Test] 測試 SetAutoSize(true) 依內容展開...");

    cppui::app::Form form;
    auto panel = std::make_shared<cppui::layout::StackPanel>(cppui::layout::Orientation::Horizontal);
    auto btn1 = std::make_shared<cppui::controls::Button>("Button 1");
    btn1->SetWidth(300.0f);
    btn1->SetHeight(120.0f);
    auto btn2 = std::make_shared<cppui::controls::Button>("Button 2");
    btn2->SetWidth(400.0f);
    btn2->SetHeight(150.0f);

    panel->AddChild(btn1);
    panel->AddChild(btn2);
    form.SetContent(panel);

    form.SetAutoSize(true);
    assert(form.GetAutoSize());
    assert(form.GetSizeToContent() == cppui::app::SizeToContent::WidthAndHeight);

    // 總期望寬度應至少為 300 + 400 = 700，高度應至少為 150
    assert(form.GetFormSize().width >= 700.0f);
    assert(form.GetFormSize().height >= 150.0f);

    compat::println(" -> 通過: 自動度量並正確展開視窗寬高。");
}

/// <summary>
/// 測試單一維度自適應 (SizeToContent::Width 與 SizeToContent::Height)。
/// </summary>
static void TestSingleAxisAutoSize() {
    compat::println("[Test] 測試單軸自適應 (Width Only / Height Only)...");

    {
        cppui::app::Form form;
        form.SetFormSize(800, 600);

        auto btn = std::make_shared<cppui::controls::Button>("Wide Button");
        btn->SetWidth(1050.0f);
        btn->SetHeight(200.0f);
        form.SetContent(btn);

        form.SetSizeToContent(cppui::app::SizeToContent::Width);
        assert(form.GetFormSize().width >= 1050.0f);
        assert(form.GetFormSize().height == 600.0f); // 高度維持不變
    }

    {
        cppui::app::Form form;
        form.SetFormSize(800, 600);

        auto btn = std::make_shared<cppui::controls::Button>("Tall Button");
        btn->SetWidth(300.0f);
        btn->SetHeight(850.0f);
        form.SetContent(btn);

        form.SetSizeToContent(cppui::app::SizeToContent::Height);
        assert(form.GetFormSize().width == 800.0f); // 寬度維持不變
        assert(form.GetFormSize().height >= 850.0f);
    }

    compat::println(" -> 通過: 單軸自適應正確約束目標維度。");
}

/// <summary>
/// 測試 AutoSizeMode::GrowOnly 模式 (內容小於目前尺寸時不收縮)。
/// </summary>
static void TestAutoSizeModeGrowOnly() {
    compat::println("[Test] 測試 AutoSizeMode::GrowOnly 模式...");

    cppui::app::Form form;
    form.SetFormSize(800, 600);
    form.SetAutoSizeMode(cppui::app::AutoSizeMode::GrowOnly);

    auto smallBtn = std::make_shared<cppui::controls::Button>("Small");
    smallBtn->SetWidth(200.0f);
    smallBtn->SetHeight(100.0f);
    form.SetContent(smallBtn);
    form.SetAutoSize(true);

    // 內容尺寸較小，但 GrowOnly 禁止縮小，故維持 800x600
    assert(form.GetFormSize().width >= 800.0f);
    assert(form.GetFormSize().height >= 600.0f);

    // 換成超大元件
    auto largeBtn = std::make_shared<cppui::controls::Button>("Large");
    largeBtn->SetWidth(1100.0f);
    largeBtn->SetHeight(900.0f);
    form.SetContent(largeBtn);

    assert(form.GetFormSize().width >= 1100.0f);
    assert(form.GetFormSize().height >= 900.0f);

    compat::println(" -> 通過: GrowOnly 模式正確防止逆向縮小且支援正常擴展。");
}

/// <summary>
/// 測試 AutoSizeMode::GrowAndShrink 模式 (內容變小時同步收縮)。
/// </summary>
static void TestAutoSizeModeGrowAndShrink() {
    compat::println("[Test] 測試 AutoSizeMode::GrowAndShrink 模式...");

    cppui::app::Form form;
    form.SetFormSize(800, 600);
    form.SetAutoSizeMode(cppui::app::AutoSizeMode::GrowAndShrink);

    auto smallBtn = std::make_shared<cppui::controls::Button>("Small");
    smallBtn->SetWidth(350.0f);
    smallBtn->SetHeight(250.0f);
    form.SetContent(smallBtn);
    form.SetAutoSize(true);

    assert(form.GetFormSize().width == 350.0f);
    assert(form.GetFormSize().height == 250.0f);

    compat::println(" -> 通過: GrowAndShrink 正確收縮視窗尺寸。");
}

/// <summary>
/// 測試 Min/Max 約束對 AutoSize 的限制。
/// </summary>
static void TestMinMaxConstraints() {
    compat::println("[Test] 測試 MinWidth/MaxWidth 與 MinHeight/MaxHeight 約束...");

    cppui::app::Form form;
    form.SetMinWidth(400.0f);
    form.SetMaxWidth(900.0f);
    form.SetMinHeight(300.0f);
    form.SetMaxHeight(700.0f);

    // 1. 小於最小尺寸，應 Clamp 至 Min
    auto tinyBtn = std::make_shared<cppui::controls::Button>("Tiny");
    tinyBtn->SetWidth(100.0f);
    tinyBtn->SetHeight(100.0f);
    form.SetContent(tinyBtn);
    form.SetAutoSize(true);

    assert(form.GetFormSize().width == 400.0f);
    assert(form.GetFormSize().height == 300.0f);

    // 2. 大於最大尺寸，應 Clamp 至 Max
    auto giantBtn = std::make_shared<cppui::controls::Button>("Giant");
    giantBtn->SetWidth(1500.0f);
    giantBtn->SetHeight(1200.0f);
    form.SetContent(giantBtn);

    assert(form.GetFormSize().width == 900.0f);
    assert(form.GetFormSize().height == 700.0f);

    compat::println(" -> 通過: 尺寸嚴格受 Min/Max 約束限制。");
}

/// <summary>
/// 測試 CaptureToBitmap 與 AutoSize 協同運作。
/// </summary>
static void TestCaptureToBitmapWithAutoSize() {
    compat::println("[Test] 測試 CaptureToBitmap 自動觸發 AutoSize 尺寸適配...");

    cppui::app::Form form;
    auto panel = std::make_shared<cppui::layout::StackPanel>(cppui::layout::Orientation::Vertical);
    auto tb = std::make_shared<cppui::controls::TextBlock>("AutoSize Snapshot Test");
    tb->SetWidth(500.0f);
    tb->SetHeight(100.0f);
    panel->AddChild(tb);

    form.SetContent(panel);
    form.SetAutoSize(true);

    auto bmp = form.CaptureToBitmap();
    assert(bmp != nullptr);
    assert(bmp->GetWidth() >= 500);
    assert(bmp->GetHeight() >= 100);

    compat::println(" -> 通過: CaptureToBitmap 精確匹配自適應尺寸並成功渲染點陣圖。");
}

} // namespace

int main() {
    compat::println("=================================================");
    compat::println("       CPPUI Form AutoSize 功能單元測試套件       ");
    compat::println("=================================================");

    TestDefaultManualMode();
    TestSetWidthAndHeight();
    TestAutoSizeWidthAndHeight();
    TestSingleAxisAutoSize();
    TestAutoSizeModeGrowOnly();
    TestAutoSizeModeGrowAndShrink();
    TestMinMaxConstraints();
    TestCaptureToBitmapWithAutoSize();

    compat::println("=================================================");
    compat::println("所有 Form AutoSize 單元測試均成功通過 (100% Passed)！");
    compat::println("=================================================");
    return 0;
}
