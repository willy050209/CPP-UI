/**
 * @file usability_integration_test.cpp
 * @brief 單元測試：驗證頂層全功能傘狀標頭檔 (ui/CppUI.hpp) 整合性、子系統完備性與 Timer 路徑單一性
 */

#include <cassert>
#include <memory>
#include <string>

// 僅引入單一頂層聚合標頭
#include "ui/CppUI.hpp"

namespace cppui {
namespace test {

static void TestUmbrellaHeaderCoreAndControls() {
    compat::println("[Test] 1. Verifying CppUI.hpp Core & Controls Subsystems...");

    // 核心型別
    core::Point pt(10.0f, 20.0f);
    core::Size sz(100.0f, 50.0f);
    core::Rect rect(pt, sz);
    core::Color col(255, 0, 128);
    assert(rect.width == 100.0f);
    assert(col.a == 255);

    // 控制項型別
    auto btn = std::make_shared<controls::Button>("測試按鈕");
    auto tb = std::make_shared<controls::TextBox>("初始文字");
    auto combo = std::make_shared<controls::ComboBox>();
    combo->AddItem("項目 A");
    combo->AddItem("項目 B");
    combo->SetSelectedIndex(1);
    assert(combo->GetSelectedItem() == "項目 B");

    auto grid = std::make_shared<controls::DataGrid>();
    grid->AddColumn("編號", controls::DataGridLength::Pixel(50.0f));
    grid->AddColumn("標題", controls::DataGridLength::Star(1.0f));
    grid->AddRow({ "101", "第一行資料" });
    assert(grid->GetRowCount() == 1);

    auto tree = std::make_shared<controls::TreeView>();
    auto rootNode = tree->AddRootItem("根節點");
    rootNode->AddItem("子節點");
    assert(tree->GetItems().size() == 1);

    compat::println("  -> PASSED: Core & Controls created and operated successfully via CppUI.hpp.");
}

static void TestUmbrellaHeaderLayoutAndApp() {
    compat::println("[Test] 2. Verifying CppUI.hpp Layout, App & Timer Subsystems...");

    auto panel = std::make_shared<layout::StackPanel>(layout::Orientation::Vertical);
    auto btn = std::make_shared<controls::Button>("項目");
    panel->AddChild(btn);

    auto vpanel = std::make_shared<layout::VirtualizingStackPanel>();
    vpanel->SetItemCount(500);
    assert(vpanel->GetItemCount() == 500);

    // 驗證唯一路徑 app::Timer 正確可用
    auto timer = std::make_shared<app::Timer>(250);
    assert(timer->GetInterval() == 250);
    assert(!timer->IsEnabled());

    // 驗證佈局管理器調度
    layout::LayoutManager::Get().UpdateLayout(*panel, core::Size(800.0f, 600.0f));
    assert(panel->IsMeasureValid());
    assert(panel->IsArrangeValid());

    compat::println("  -> PASSED: Layout, App and app::Timer operated seamlessly.");
}

static void TestUmbrellaHeaderGraphicsAndDiagnostics() {
    compat::println("[Test] 3. Verifying Graphics, Diagnostics & PAL Facade...");

    // 向量繪圖與影像
    auto bmp = std::make_shared<graphics::Bitmap>(32, 32);
    assert(bmp->GetWidth() == 32);
    graphics::Graphics g(bmp);
    g.Clear(graphics::Colors::Navy);

    // 診斷選項與追蹤器
    diagnostics::DiagnosticsOptions opts;
    opts.showFpsCounter = true;
    diagnostics::DirtyRectTracker::Get().SetOptions(opts);
    assert(diagnostics::DirtyRectTracker::Get().GetOptions().showFpsCounter);

    // 剪貼簿靜態門面
    pal::Clipboard::SetText("CppUI.hpp 測試文字");
    assert(pal::Clipboard::ContainsText());
    assert(pal::Clipboard::GetText() == "CppUI.hpp 測試文字");

    compat::println("  -> PASSED: Graphics, Diagnostics & PAL operated seamlessly.");
}

} // namespace test
} // namespace cppui

int main() {
    compat::println("==========================================================");
    compat::println(" [CPPUI Test] Usability & Developer Experience Test       ");
    compat::println("==========================================================");

    cppui::test::TestUmbrellaHeaderCoreAndControls();
    cppui::test::TestUmbrellaHeaderLayoutAndApp();
    cppui::test::TestUmbrellaHeaderGraphicsAndDiagnostics();

    compat::println("==========================================================");
    compat::println(" ALL USABILITY & DX INTEGRATION TESTS PASSED (100%)       ");
    compat::println("==========================================================");
    return 0;
}
