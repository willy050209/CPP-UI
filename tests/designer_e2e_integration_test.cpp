/**
 * @file designer_e2e_integration_test.cpp
 * @brief CPPUI Designer 階段 5 即時代碼預覽、匯出與端到端整合 (E2E Integration) 測試套件
 */

#include <cassert>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include "compat/Compat.hpp"
#include "ui/Designer.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/DataGrid.hpp"
#include "ui/controls/ProgressBar.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Canvas.hpp"

using namespace cppui;
using namespace cppui::designer;
using namespace cppui::designer::preview;
using namespace cppui::designer::exporter;
using namespace cppui::designer::surface;

// 輔助函式：建立標準 Admin 管理表單 AST
static FormDocumentNode BuildAdminFormAst() {
    FormDocumentNode doc;
    FormSettings settings;
    settings.className = "AdminForm";
    settings.targetNamespace = "AdminApp";
    settings.title = "CPPUI Admin Management System";
    settings.initialSize = core::Size(1024.0f, 768.0f);
    doc.SetSettings(settings);

    // 建立 3x2 Grid 根容器
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "rootGrid");
    rootGrid->SetRowDefinitions({
        layout::RowDefinition(layout::GridLength::Pixel(50.0f)),
        layout::RowDefinition(layout::GridLength::Star(1.0f)),
        layout::RowDefinition(layout::GridLength::Pixel(40.0f))
    });
    rootGrid->SetColumnDefinitions({
        layout::ColumnDefinition(layout::GridLength::Pixel(220.0f)),
        layout::ColumnDefinition(layout::GridLength::Star(1.0f))
    });

    // 側邊欄 StackPanel (Row 0, Col 0, RowSpan 3)
    auto sidebar = std::make_unique<WidgetNode>(NodeType::StackPanel, "sidebarPanel");
    sidebar->SetAttachedProperty("Grid.Row", PropertyValue(int32_t(0)));
    sidebar->SetAttachedProperty("Grid.Column", PropertyValue(int32_t(0)));
    sidebar->SetAttachedProperty("Grid.RowSpan", PropertyValue(int32_t(3)));
    sidebar->SetProperty("Spacing", PropertyValue(10.0));

    auto navLabel = std::make_unique<WidgetNode>(NodeType::Label, "navTitle");
    navLabel->SetProperty("Content", PropertyValue(std::string("Navigation Menu")));

    auto dashBtn = std::make_unique<WidgetNode>(NodeType::Button, "dashBtn");
    dashBtn->SetProperty("Content", PropertyValue(std::string("Dashboard")));
    dashBtn->AddEventBinding(EventBindingDescriptor{ "Click", "OnDashBtnClick" });

    auto userBtn = std::make_unique<WidgetNode>(NodeType::Button, "userBtn");
    userBtn->SetProperty("Content", PropertyValue(std::string("User Accounts")));
    userBtn->AddEventBinding(EventBindingDescriptor{ "Click", "OnUserBtnClick" });

    sidebar->AddChild(std::move(navLabel));
    sidebar->AddChild(std::move(dashBtn));
    sidebar->AddChild(std::move(userBtn));

    // 頂部搜尋欄 (Row 0, Col 1)
    auto searchBox = std::make_unique<WidgetNode>(NodeType::TextBox, "searchBox");
    searchBox->SetAttachedProperty("Grid.Row", PropertyValue(int32_t(0)));
    searchBox->SetAttachedProperty("Grid.Column", PropertyValue(int32_t(1)));
    searchBox->SetProperty("Placeholder", PropertyValue(std::string("Search records...")));

    // 中央表格 (Row 1, Col 1)
    auto dataGrid = std::make_unique<WidgetNode>(NodeType::DataGrid, "userDataGrid");
    dataGrid->SetAttachedProperty("Grid.Row", PropertyValue(int32_t(1)));
    dataGrid->SetAttachedProperty("Grid.Column", PropertyValue(int32_t(1)));

    // 底部狀態進度條 (Row 2, Col 1)
    auto progressBar = std::make_unique<WidgetNode>(NodeType::ProgressBar, "loadProgressBar");
    progressBar->SetAttachedProperty("Grid.Row", PropertyValue(int32_t(2)));
    progressBar->SetAttachedProperty("Grid.Column", PropertyValue(int32_t(1)));
    progressBar->SetProperty("Value", PropertyValue(65.0));

    rootGrid->AddChild(std::move(sidebar));
    rootGrid->AddChild(std::move(searchBox));
    rootGrid->AddChild(std::move(dataGrid));
    rootGrid->AddChild(std::move(progressBar));

    doc.SetRoot(std::move(rootGrid));
    return doc;
}

static void TestAdminFormAssembly() {
    compat::println("[Test 1] Complete Admin Form AST Assembly & Hierarchy Verification...");

    FormDocumentNode doc = BuildAdminFormAst();
    assert(doc.GetSettings().className == "AdminForm");
    assert(doc.GetSettings().targetNamespace == "AdminApp");
    assert(doc.GetSettings().title == "CPPUI Admin Management System");

    const auto* root = doc.GetRoot();
    assert(root != nullptr);
    assert(root->GetType() == NodeType::Grid);
    assert(root->GetChildCount() == 4);

    const auto* dashBtn = doc.FindByName("dashBtn");
    assert(dashBtn != nullptr);
    assert(dashBtn->GetProperty("Content")->Get<std::string>().value() == "Dashboard");
    assert(dashBtn->GetEventBindings().size() == 1);
    assert(dashBtn->GetEventBindings()[0].handlerName == "OnDashBtnClick");

    compat::println("  [PASS] Admin Form AST assembly and cardinality verified.");
}

static void TestLiveCodePreviewAndSourceMap() {
    compat::println("[Test 2] Live Code Preview & SourceMap Synchronous Tracking...");

    FormDocumentNode doc = BuildAdminFormAst();
    SelectionModel sm;
    LiveCodePreviewPanel previewPanel(&doc, &sm);

    // 1. 驗證 5 種預覽檢視模式
    previewPanel.SetViewMode(PreviewViewMode::FluentDsl);
    assert(!previewPanel.GetActiveCodeText().empty());
    assert(previewPanel.GetActiveCodeText().find("Grid") != std::string::npos);

    previewPanel.SetViewMode(PreviewViewMode::ClassicDesignerHpp);
    assert(!previewPanel.GetActiveCodeText().empty());
    assert(previewPanel.GetActiveCodeText().find("InitializeComponent") != std::string::npos);
    assert(previewPanel.GetActiveCodeText().find("dashBtn") != std::string::npos);

    previewPanel.SetViewMode(PreviewViewMode::ClassicHeader);
    assert(!previewPanel.GetActiveCodeText().empty());
    assert(previewPanel.GetActiveCodeText().find("class AdminForm") != std::string::npos);

    previewPanel.SetViewMode(PreviewViewMode::ClassicSource);
    assert(!previewPanel.GetActiveCodeText().empty());
    assert(previewPanel.GetActiveCodeText().find("OnDashBtnClick") != std::string::npos);

    previewPanel.SetViewMode(PreviewViewMode::CMakeLists);
    assert(!previewPanel.GetActiveCodeText().empty());
    assert(previewPanel.GetActiveCodeText().find("add_executable") != std::string::npos);

    // 2. 驗證 SourceMap 行號高亮
    previewPanel.SetViewMode(PreviewViewMode::ClassicDesignerHpp);
    const auto* dashBtn = doc.FindByName("dashBtn");
    assert(dashBtn != nullptr);

    sm.SetSelection(dashBtn->GetUuid());
    auto span = previewPanel.GetHighlightedSpan();
    assert(span.has_value());
    assert(span->startLine > 0);
    assert(span->endLine >= span->startLine);

    compat::println("  [PASS] Live Code Preview across 5 modes and SourceMap line highlighting verified.");
}

static void TestLiveFormInstantiationAndInteraction() {
    compat::println("[Test 3] Live Form Instantiation & Interactive Event Routing...");

    FormDocumentNode doc = BuildAdminFormAst();

    std::string triggeredUuid;
    std::string triggeredEvent;

    auto eventCallback = [&](const std::string& uuid, const std::string& ev) {
        triggeredUuid = uuid;
        triggeredEvent = ev;
    };

    auto form = LiveFormRunner::CreateForm(doc, eventCallback);
    assert(form != nullptr);
    assert(form->GetTitle() == "CPPUI Admin Management System");
    assert(form->GetFormSize().width == 1024.0f);
    assert(form->GetFormSize().height == 768.0f);

    // 驗證真實控制項樹結構
    auto content = form->GetContent();
    assert(content != nullptr);

    auto grid = std::dynamic_pointer_cast<layout::Grid>(content);
    assert(grid != nullptr);
    assert(grid->GetChildren().size() == 4);

    // 取得側邊欄內的按鈕
    auto sidebar = std::dynamic_pointer_cast<layout::StackPanel>(grid->GetChildren()[0]);
    assert(sidebar != nullptr);
    assert(sidebar->GetChildren().size() == 3);

    auto btn = std::dynamic_pointer_cast<controls::Button>(sidebar->GetChildren()[1]);
    assert(btn != nullptr);
    assert(btn->GetText() == "Dashboard");

    // 模擬真實按鈕點擊
    const auto* dashBtnNode = doc.FindByName("dashBtn");
    assert(dashBtnNode != nullptr);

    btn->PerformClick();
    assert(triggeredUuid == dashBtnNode->GetUuid());
    assert(triggeredEvent == "Click");

    compat::println("  [PASS] Live Form instantiation and interactive event routing verified.");
}

static void TestThemeSwitchingInLiveForm() {
    compat::println("[Test 4] Theme Switching Parity in Live Form...");

    FormDocumentNode doc = BuildAdminFormAst();
    auto form = LiveFormRunner::CreateForm(doc);
    assert(form != nullptr);

    // 驗證暗色主題
    theme::ThemeManager::Get().SetTheme(theme::Theme::Dark());
    assert(theme::ThemeManager::Get().GetCurrentTheme().GetMode() == theme::ThemeMode::Dark);

    // 切換為亮色主題
    theme::ThemeManager::Get().SetTheme(theme::Theme::Light());
    assert(theme::ThemeManager::Get().GetCurrentTheme().GetMode() == theme::ThemeMode::Light);

    // 還原為預設暗色
    theme::ThemeManager::Get().SetTheme(theme::Theme::Dark());

    compat::println("  [PASS] Theme switching in Live Form verified.");
}

static void TestLosslessProjectSerialization() {
    compat::println("[Test 5] Lossless Project Serialization & Round-Trip (.cppui.json)...");

    FormDocumentNode doc = BuildAdminFormAst();

    auto tempPath = std::filesystem::temp_directory_path() / "test_admin_form.cppui.json";
    bool saveOk = ProjectExporter::SaveProjectJson(doc, tempPath.string());
    assert(saveOk);

    auto loadRes = ProjectExporter::LoadProjectJson(tempPath.string());
    assert(loadRes.has_value());

    auto loadedDoc = std::move(*loadRes);
    assert(loadedDoc != nullptr);
    assert(loadedDoc->GetSettings().className == "AdminForm");
    assert(loadedDoc->GetSettings().title == "CPPUI Admin Management System");

    const auto* root = loadedDoc->GetRoot();
    assert(root != nullptr);
    assert(root->GetType() == NodeType::Grid);
    assert(root->GetChildCount() == 4);

    const auto* btn = loadedDoc->FindByName("dashBtn");
    assert(btn != nullptr);
    assert(btn->GetProperty("Content")->Get<std::string>().value() == "Dashboard");
    assert(btn->GetEventBindings().size() == 1);
    assert(btn->GetEventBindings()[0].handlerName == "OnDashBtnClick");

    std::filesystem::remove(tempPath);
    compat::println("  [PASS] Lossless project serialization & round-trip verified.");
}

static void TestCMakeProjectExportDirectory() {
    compat::println("[Test 6] Full CMake Project Directory Export & File Integrity...");

    FormDocumentNode doc = BuildAdminFormAst();

    auto exportDir = std::filesystem::temp_directory_path() / "cppui_e2e_export_test";
    std::filesystem::remove_all(exportDir);

    ProjectExportOptions opts;
    opts.outputDirectory = exportDir.string();
    opts.projectName = "AdminSystem";
    opts.formClassName = "AdminForm";
    opts.targetNamespace = "AdminApp";

    auto res = ProjectExporter::ExportProject(doc, opts);
    assert(res.success);

    // 驗證生成的關鍵檔案完整性
    assert(std::filesystem::exists(exportDir / "CMakeLists.txt"));
    assert(std::filesystem::exists(exportDir / ".gitignore"));
    assert(std::filesystem::exists(exportDir / "include" / "AdminApp" / "AdminForm.designer.hpp"));
    assert(std::filesystem::exists(exportDir / "include" / "AdminApp" / "AdminForm.hpp"));
    assert(std::filesystem::exists(exportDir / "src" / "AdminForm.cpp"));
    assert(std::filesystem::exists(exportDir / "src" / "main.cpp"));
    assert(std::filesystem::exists(exportDir / "assets"));

    compat::println("  [PASS] CMake project export directory structure and file integrity verified.");
}

static void TestIncrementalUserCodeProtection() {
    compat::println("[Test 7] Incremental User Code Protection & Stub Preservation...");

    FormDocumentNode doc = BuildAdminFormAst();
    auto exportDir = std::filesystem::temp_directory_path() / "cppui_e2e_export_test";

    ProjectExportOptions opts;
    opts.outputDirectory = exportDir.string();
    opts.projectName = "AdminSystem";
    opts.formClassName = "AdminForm";
    opts.targetNamespace = "AdminApp";
    opts.incrementalUserCode = true;

    // 1. 模擬使用者在 AdminForm.cpp 中編寫了客製業務邏輯
    auto cppPath = exportDir / "src" / "AdminForm.cpp";
    std::ofstream out(cppPath.string(), std::ios::app);
    out << "\n// === Custom User Business Logic ===\n"
        << "int g_customUserCounter = 42;\n"
        << "void CustomUserHelper() {}\n";
    out.close();

    // 2. 在 AST 中新增一個全新事件綁定 OnExportReportClick
    auto* userBtn = doc.FindByName("userBtn");
    assert(userBtn != nullptr);
    userBtn->AddEventBinding(EventBindingDescriptor{ "RightClick", "OnExportReportClick" });

    // 3. 再次執行匯出
    auto res = ProjectExporter::ExportProject(doc, opts);
    assert(res.success);

    // 4. 讀取 AdminForm.cpp 驗證既有程式碼完整保留且追加了新 Stub
    std::string content;
    {
        std::ifstream in(cppPath.string());
        content.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }

    assert(content.find("int g_customUserCounter = 42;") != std::string::npos);
    assert(content.find("void CustomUserHelper() {}") != std::string::npos);
    assert(content.find("OnExportReportClick") != std::string::npos);

    std::error_code ec;
    std::filesystem::remove_all(exportDir, ec);
    compat::println("  [PASS] Incremental user code protection and stub appending verified.");
}

static void TestLiveFormRunnerStylingAndVisibility() {
    compat::println("[Test 8] LiveFormRunner Control Styling & Visibility Reflection...");

    FormDocumentNode doc;
    FormSettings settings;
    settings.title = "StylingTest";
    settings.initialSize = core::Size(600.0f, 400.0f);
    doc.SetSettings(settings);

    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas");
    root->SetProperty("Background", PropertyValue(core::Color(30, 40, 50, 255)));

    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "styledBtn");
    btn->SetProperty("Content", PropertyValue(std::string("Click Me")));
    btn->SetProperty("Background", PropertyValue(core::Color(255, 0, 0, 255)));
    btn->SetProperty("Foreground", PropertyValue(core::Color(0, 255, 0, 255)));
    btn->SetProperty("FontSize", PropertyValue(18.0));
    btn->SetProperty("FontFamily", PropertyValue(std::string("Arial")));
    btn->SetProperty("FontWeight", PropertyValue(int32_t(700)));
    btn->SetProperty("Visibility", PropertyValue(layout::Visibility::Collapsed));

    root->AddChild(std::move(btn));
    doc.SetRoot(std::move(root));

    auto form = LiveFormRunner::CreateForm(doc);
    assert(form != nullptr);
    assert(form->GetTitle() == "StylingTest");
    assert(form->GetBackground() == core::Color(30, 40, 50, 255));

    auto canvas = std::dynamic_pointer_cast<layout::Canvas>(form->GetContent());
    assert(canvas != nullptr);
    assert(canvas->GetChildrenCount() == 1);

    auto* childBtn = dynamic_cast<controls::Button*>(canvas->GetChild(0));
    assert(childBtn != nullptr);
    assert(childBtn->GetVisibility() == layout::Visibility::Collapsed);
    assert(childBtn->GetBackground() == core::Color(255, 0, 0, 255));
    assert(childBtn->GetForeground() == core::Color(0, 255, 0, 255));
    assert(childBtn->GetFontSize() == 18.0f);
    assert(childBtn->GetFontFamily() == "Arial");
    assert(childBtn->GetFontWeight() == text::FontWeight::Bold);

    compat::println("  [PASS] LiveFormRunner styling & visibility reflection verified.");
}

int main() {
    compat::println("=== CPPUI Designer Phase 5 E2E Integration Test Suite ===");

    TestAdminFormAssembly();
    TestLiveCodePreviewAndSourceMap();
    TestLiveFormInstantiationAndInteraction();
    TestThemeSwitchingInLiveForm();
    TestLosslessProjectSerialization();
    TestCMakeProjectExportDirectory();
    TestIncrementalUserCodeProtection();
    TestLiveFormRunnerStylingAndVisibility();

    compat::println("=== All Phase 5 E2E Integration Tests Passed Successfully! ===");
    return 0;
}
