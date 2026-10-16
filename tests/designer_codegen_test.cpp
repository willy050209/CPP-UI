/**
 * @file designer_codegen_test.cpp
 * @brief CPPUI Designer 階段 2 程式碼生成引擎 (Codegen Engine) 完整單元測試
 */

#include <cassert>
#include <iostream>
#include <string>
#include "compat/Compat.hpp"
#include "ui/Designer.hpp"

using namespace cppui;
using namespace cppui::designer;
using namespace cppui::designer::codegen;

static void TestNamingAndStyles() {
    compat::println("[Test 1] Identifier Formatting & Naming Styles...");

    assert(FormatIdentifier("submit_button", NamingStyle::CamelCase) == "submitButton");
    assert(FormatIdentifier("submit_button", NamingStyle::PascalCase) == "SubmitButton");
    assert(FormatIdentifier("submit_button", NamingStyle::SnakeCase) == "submit_button");
    assert(FormatIdentifier("submit_button", NamingStyle::MemberMStyle) == "m_submitButton");

    assert(FormatIdentifier("btn-click-me", NamingStyle::CamelCase) == "btnClickMe");
    assert(FormatIdentifier("m_txtName", NamingStyle::CamelCase) == "txtName");
    assert(FormatIdentifier("m_txtName", NamingStyle::MemberMStyle) == "m_txtName");

    assert(GetWidgetCppType(NodeType::Button) == "cppui::controls::Button");
    assert(GetWidgetCppType(NodeType::Grid) == "cppui::layout::Grid");
    assert(GetWidgetDslFactory(NodeType::Button) == "Button");

    compat::println("  [PASS] Naming styles and widget type mappings verified.");
}

static void TestFluentDslGeneration() {
    compat::println("[Test 2] Fluent DSL Generation & Default Elision...");

    FluentDslEmitter emitter;
    CodegenOptions opts;
    opts.elideDefaults = true;

    // 1. 單一 Button
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "myButton");
    btn->SetProperty("Text", std::string("Click Me"));
    btn->SetProperty("Width", 150.0);
    btn->SetProperty("Height", 40.0);
    btn->SetProperty("Background", core::Color(255, 0, 0, 255));

    std::string dslBtn = emitter.EmitNode(*btn, opts, 0);
    assert(dslBtn.find("Button(\"Click Me\")") != std::string::npos);
    assert(dslBtn.find(".Width(150.0f)") != std::string::npos);
    assert(dslBtn.find(".Height(40.0f)") != std::string::npos);
    assert(dslBtn.find(".Background(cppui::core::Color(255, 0, 0, 255))") != std::string::npos);

    // 2. 巢狀 StackPanel (VStack 持有 Button 與 Label)
    auto vstack = std::make_unique<WidgetNode>(NodeType::StackPanel, "mainStack");
    vstack->SetProperty("Orientation", layout::Orientation::Vertical);
    vstack->SetProperty("Spacing", 10.0);

    auto lbl = std::make_unique<WidgetNode>(NodeType::Label, "titleLabel");
    lbl->SetProperty("Text", std::string("Welcome"));

    auto btn2 = std::make_unique<WidgetNode>(NodeType::Button, "okBtn");
    btn2->SetProperty("Text", std::string("OK"));

    auto addLblRes = vstack->AddChild(std::move(lbl));
    assert(addLblRes.has_value());
    auto addBtnRes = vstack->AddChild(std::move(btn2));
    assert(addBtnRes.has_value());

    std::string dslStack = emitter.EmitNode(*vstack, opts, 0);
    assert(dslStack.find("VStack({") != std::string::npos);
    assert(dslStack.find("Label(\"Welcome\")") != std::string::npos);
    assert(dslStack.find("Button(\"OK\")") != std::string::npos);
    assert(dslStack.find(".Spacing(10.0f)") != std::string::npos);

    // 3. Grid 附加屬性
    auto grid = std::make_unique<WidgetNode>(NodeType::Grid, "mainGrid");
    auto gridChild = std::make_unique<WidgetNode>(NodeType::Button, "cellBtn");
    gridChild->SetProperty("Text", std::string("In Grid"));
    gridChild->SetAttachedProperty("Grid.Row", int32_t(1));
    gridChild->SetAttachedProperty("Grid.Column", int32_t(2));
    gridChild->SetAttachedProperty("Grid.RowSpan", int32_t(2));
    gridChild->SetAttachedProperty("Grid.ColumnSpan", int32_t(3));

    auto addGridChildRes = grid->AddChild(std::move(gridChild));
    assert(addGridChildRes.has_value());

    std::string dslGrid = emitter.EmitNode(*grid, opts, 0);
    assert(dslGrid.find("Grid({") != std::string::npos);
    assert(dslGrid.find(".Row(1)") != std::string::npos);
    assert(dslGrid.find(".Column(2)") != std::string::npos);
    assert(dslGrid.find(".RowSpan(2)") != std::string::npos);
    assert(dslGrid.find(".ColumnSpan(3)") != std::string::npos);

    compat::println("  [PASS] Fluent DSL emitted with correct chaining and attached properties.");
}

static void TestFormPartialClassGeneration() {
    compat::println("[Test 3] Form Partial Class (Code-Behind) Trio Generation...");

    FormDocumentNode doc;
    FormSettings settings;
    settings.className = "UserRegistrationForm";
    settings.targetNamespace = "test_app";
    settings.title = "User Registration";
    settings.initialSize = core::Size(800.0f, 600.0f);
    doc.SetSettings(settings);

    // 根容器 Grid
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "layoutRoot");
    std::vector<layout::RowDefinition> rows;
    rows.emplace_back(layout::GridLength::Pixel(50.0f));
    rows.emplace_back(layout::GridLength::Star(1.0f));
    rootGrid->SetRowDefinitions(rows);

    // 子元件 1: TextBox (具名且有 TextChanged 事件)
    auto txtUser = std::make_unique<WidgetNode>(NodeType::TextBox, "txtUsername");
    txtUser->SetProperty("Placeholder", std::string("Enter your name"));
    txtUser->SetProperty("Width", 200.0);
    txtUser->SetAttachedProperty("Grid.Row", int32_t(0));
    txtUser->SetAttachedProperty("Grid.Column", int32_t(0));
    txtUser->AddEventBinding(EventBindingDescriptor{ "TextChanged", "OnUsernameChanged" });

    // 子元件 2: Button (具名且有 Click 事件)
    auto btnSubmit = std::make_unique<WidgetNode>(NodeType::Button, "btnRegister");
    btnSubmit->SetProperty("Text", std::string("Register Now"));
    btnSubmit->SetProperty("Width", 120.0);
    btnSubmit->SetAttachedProperty("Grid.Row", int32_t(1));
    btnSubmit->SetAttachedProperty("Grid.Column", int32_t(0));
    btnSubmit->AddEventBinding(EventBindingDescriptor{ "Click", "OnRegisterClicked" });

    auto r1 = rootGrid->AddChild(std::move(txtUser));
    assert(r1.has_value());
    auto r2 = rootGrid->AddChild(std::move(btnSubmit));
    assert(r2.has_value());

    doc.SetRoot(std::move(rootGrid));

    // 使用 FormCodegenPipeline 執行生成
    FormCodegenPipeline pipeline;
    CodegenOptions opts;
    opts.formClassName = "UserRegistrationForm";
    opts.rootNamespace = "test_app";

    auto genRes = pipeline.Generate(doc, opts);
    assert(genRes.has_value());

    const auto& res = genRes.value();

    // 檢驗 designer.hpp
    assert(res.designerHeader.find("class UserRegistrationForm_Designer : public cppui::app::Form") != std::string::npos);
    assert(res.designerHeader.find("std::shared_ptr<cppui::controls::TextBox> m_txtUsername;") != std::string::npos);
    assert(res.designerHeader.find("std::shared_ptr<cppui::controls::Button> m_btnRegister;") != std::string::npos);
    assert(res.designerHeader.find("void InitializeComponent();") != std::string::npos);
    assert(res.designerHeader.find("virtual void OnUsernameChanged(compat::string_view text)") != std::string::npos);
    assert(res.designerHeader.find("virtual void OnRegisterClicked(const cppui::input::PointerEventArgs& e)") != std::string::npos);
    assert(res.designerHeader.find("SetTitle(\"User Registration\");") != std::string::npos);
    assert(res.designerHeader.find("SetFormSize(800, 600);") != std::string::npos);
    assert(res.designerHeader.find("m_txtUsername->OnTextChanged") != std::string::npos);
    assert(res.designerHeader.find("m_btnRegister->OnClick") != std::string::npos);
    assert(res.designerHeader.find("SetContent(m_layoutRoot);") != std::string::npos);

    // 檢驗 UserHeader (UserRegistrationForm.hpp)
    assert(res.userHeader.find("class UserRegistrationForm : public UserRegistrationForm_Designer") != std::string::npos);
    assert(res.userHeader.find("void OnUsernameChanged(compat::string_view text) override;") != std::string::npos);
    assert(res.userHeader.find("void OnRegisterClicked(const cppui::input::PointerEventArgs& e) override;") != std::string::npos);

    // 檢驗 UserSource (UserRegistrationForm.cpp)
    assert(res.userSource.find("UserRegistrationForm::UserRegistrationForm() {") != std::string::npos);
    assert(res.userSource.find("InitializeComponent();") != std::string::npos);
    assert(res.userSource.find("void UserRegistrationForm::OnRegisterClicked") != std::string::npos);
    assert(res.userSource.find("void UserRegistrationForm::OnUsernameChanged") != std::string::npos);

    // 檢驗 CMake Snippet
    assert(res.cmakeSnippet.find("add_executable(UserRegistrationForm") != std::string::npos);
    assert(res.cmakeSnippet.find("target_link_libraries(UserRegistrationForm PRIVATE cppui)") != std::string::npos);

    compat::println("  [PASS] Form partial class trio and CMake snippet verified.");
}

static void TestSourceMapLookup() {
    compat::println("[Test 4] SourceMap Bidirectional Lookup...");

    FormDocumentNode doc;
    auto root = std::make_unique<WidgetNode>(NodeType::StackPanel, "myRoot");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "myTestBtn");
    std::string btnUuid = btn->GetUuid();

    auto addRes = root->AddChild(std::move(btn));
    assert(addRes.has_value());
    doc.SetRoot(std::move(root));

    FormCodegenPipeline pipeline;
    CodegenOptions opts;
    opts.formClassName = "MyWindow";

    auto genRes = pipeline.Generate(doc, opts);
    assert(genRes.has_value());

    const auto& sm = genRes.value().sourceMap;

    // 1. 依 UUID 反查符號
    auto symByUuid = sm.FindByNodeUuid(btnUuid);
    assert(symByUuid.has_value());
    assert(symByUuid->memberName == "m_myTestBtn");
    assert(symByUuid->typeName == "cppui::controls::Button");
    assert(symByUuid->spanInDesignerHpp.startLine > 0);

    // 2. 依變數名稱反查符號
    auto symByName = sm.FindByVariableName("m_myTestBtn");
    assert(symByName.has_value());
    assert(symByName->nodeUuid == btnUuid);

    // 3. 依行號反查符號
    size_t declLine = symByUuid->spanInDesignerHpp.startLine;
    auto symByLine = sm.FindByLine(declLine);
    assert(symByLine.has_value());
    assert(symByLine->nodeUuid == btnUuid);

    compat::println("  [PASS] SourceMap UUID, variable name, and line lookups verified.");
}

static void TestErrorHandlingAndEmptyDocs() {
    compat::println("[Test 5] Error Handling on Malformed AST...");

    FormDocumentNode emptyDoc; // 無根節點
    FormCodegenPipeline pipeline;

    auto res = pipeline.Generate(emptyDoc);
    assert(!res.has_value());
    assert(res.error().code == DesignerErrorCode::InvalidNode);

    compat::println("  [PASS] Error handled gracefully for invalid AST.");
}

int main() {
    compat::println("=================================================");
    compat::println(" [CPPUI Designer Phase 2] Codegen Engine Unit Tests");
    compat::println("=================================================");

    TestNamingAndStyles();
    TestFluentDslGeneration();
    TestFormPartialClassGeneration();
    TestSourceMapLookup();
    TestErrorHandlingAndEmptyDocs();

    compat::println("=================================================");
    compat::println(" All 5 Designer Codegen Engine Test Suites Passed!");
    compat::println("=================================================");

    return 0;
}
