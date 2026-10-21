/**
 * @file designer_inspector_test.cpp
 * @brief CPPUI Designer 階段 4 屬性與事件檢查器 (Property & Event Inspector) 單元與安全性測試
 */

#include <cassert>
#include <iostream>
#include <string>
#include <cmath>
#include "compat/Compat.hpp"
#include "ui/Designer.hpp"

using namespace cppui;
using namespace cppui::designer;
using namespace cppui::designer::inspector;
using namespace cppui::designer::surface;

static void TestInheritanceMetadataRegistry() {
    compat::println("[Test 1] Inheritance Chain Aggregation & Event Descriptors...");

    // 1. Button 繼承鏈屬性聚合檢查 (Visual -> UIElement -> Control -> ContentControl -> Button)
    const auto& btnProps = InheritanceMetadataRegistry::GetDescriptorsForType(NodeType::Button);
    assert(!btnProps.empty());

    bool hasOpacity = false;
    bool hasWidth = false;
    bool hasMargin = false;
    bool hasBackground = false;
    bool hasFontSize = false;
    bool hasContent = false;

    for (const auto& p : btnProps) {
        if (p.name == "Opacity") hasOpacity = true;
        if (p.name == "Width") hasWidth = true;
        if (p.name == "Margin") hasMargin = true;
        if (p.name == "Background") hasBackground = true;
        if (p.name == "FontSize") hasFontSize = true;
        if (p.name == "Content") hasContent = true;
    }

    assert(hasOpacity);    // Visual
    assert(hasWidth);      // UIElement
    assert(hasMargin);     // UIElement
    assert(hasBackground); // Control
    assert(hasFontSize);   // Control
    assert(hasContent);    // ContentControl

    // 2. 事件描述項檢查
    const auto& btnEvents = InheritanceMetadataRegistry::GetEventDescriptorsForType(NodeType::Button);
    assert(!btnEvents.empty());

    bool hasClick = false;
    bool hasCommand = false;
    bool hasPointerPressed = false;

    for (const auto& ev : btnEvents) {
        if (ev.eventName == "Click") hasClick = true;
        if (ev.eventName == "Command") hasCommand = true;
        if (ev.eventName == "PointerPressed") hasPointerPressed = true;
    }

    assert(hasClick);
    assert(hasCommand);
    assert(hasPointerPressed);

    // 3. TextBox 特有屬性與事件
    const auto& tbProps = InheritanceMetadataRegistry::GetDescriptorsForType(NodeType::TextBox);
    bool hasPlaceholder = false;
    bool hasIsReadOnly = false;
    for (const auto& p : tbProps) {
        if (p.name == "Placeholder") hasPlaceholder = true;
        if (p.name == "IsReadOnly") hasIsReadOnly = true;
    }
    assert(hasPlaceholder);
    assert(hasIsReadOnly);

    const auto& tbEvents = InheritanceMetadataRegistry::GetEventDescriptorsForType(NodeType::TextBox);
    bool hasTextChanged = false;
    for (const auto& ev : tbEvents) {
        if (ev.eventName == "TextChanged") hasTextChanged = true;
    }
    assert(hasTextChanged);

    compat::println("  [PASS] Inheritance chain aggregation & event descriptors verified.");
}

static void TestMultiSelectionIntersectionAndMixed() {
    compat::println("[Test 2] Multi-Selection Intersection & Mixed Value Placeholder...");

    FormDocumentNode doc;
    auto rootPanel = std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas");
    auto btnNode = std::make_unique<WidgetNode>(NodeType::Button, "myButton");
    auto txtNode = std::make_unique<WidgetNode>(NodeType::TextBox, "myTextBox");

    std::string btnUuid = btnNode->GetUuid();
    std::string txtUuid = txtNode->GetUuid();

    // 設定初始屬性 (同名屬性但數值不同)
    btnNode->SetProperty("Width", PropertyValue(120.0));
    txtNode->SetProperty("Width", PropertyValue(240.0));

    // 設定同名且同值屬性
    btnNode->SetProperty("IsEnabled", PropertyValue(true));
    txtNode->SetProperty("IsEnabled", PropertyValue(true));

    assert(rootPanel->AddChild(std::move(btnNode)).has_value());
    assert(rootPanel->AddChild(std::move(txtNode)).has_value());
    doc.SetRoot(std::move(rootPanel));

    CommandHistoryManager history;
    SelectionModel sm;
    InspectorPanel inspector(&doc, &history, &sm);

    // 1. 單選 Button
    sm.SetSelection(btnUuid);
    auto singleProps = inspector.GetInspectedProperties();
    auto widthProp = inspector.FindProperty("Width");
    assert(widthProp.has_value());
    assert(widthProp->valueState == MultiValueState::SingleValue);
    assert(widthProp->currentValue.Get<double>().value() == 120.0);
    assert(!widthProp->IsMixed());

    // 2. 多選 Button + TextBox
    sm.AddToSelection(txtUuid);
    assert(sm.GetSelectionCount() == 2);

    auto multiProps = inspector.GetInspectedProperties();

    // 檢查交集屬性存在性
    bool hasWidth = false;
    bool hasIsEnabled = false;
    bool hasContent = false;     // Button 專有，不應出現在交集中
    bool hasPlaceholder = false; // TextBox 專有，不應出現在交集中

    for (const auto& p : multiProps) {
        if (p.name == "Width") {
            hasWidth = true;
            assert(p.IsMixed());
            assert(p.valueState == MultiValueState::Mixed);
            assert(PropertyEditorModel::FormatDisplayString(p.currentValue, p.IsMixed()) == "<Multiple Values>");
        }
        if (p.name == "IsEnabled") {
            hasIsEnabled = true;
            assert(!p.IsMixed());
            assert(p.valueState == MultiValueState::SingleValue);
            assert(p.currentValue.Get<bool>().value() == true);
        }
        if (p.name == "Content") hasContent = true;
        if (p.name == "Placeholder") hasPlaceholder = true;
    }

    assert(hasWidth);
    assert(hasIsEnabled);
    assert(!hasContent);
    assert(!hasPlaceholder);

    compat::println("  [PASS] Multi-selection intersection & mixed placeholder verified.");
}

static void TestContextAwareAttachedProperties() {
    compat::println("[Test 3] Context-Aware Attached Properties (Canvas vs Grid vs None)...");

    // 1. Canvas 容器子節點動態感知
    auto canvasAttached = InheritanceMetadataRegistry::GetAttachedPropertiesForParent(NodeType::Canvas);
    bool hasCanvasLeft = false;
    bool hasCanvasTop = false;
    bool hasToolTip = false;
    for (const auto& ap : canvasAttached) {
        if (ap.name == "Canvas.Left") hasCanvasLeft = true;
        if (ap.name == "Canvas.Top") hasCanvasTop = true;
        if (ap.name == "ToolTip.Text") hasToolTip = true;
    }
    assert(hasCanvasLeft);
    assert(hasCanvasTop);
    assert(hasToolTip);

    // 2. Grid 容器子節點動態感知
    auto gridAttached = InheritanceMetadataRegistry::GetAttachedPropertiesForParent(NodeType::Grid);
    bool hasGridRow = false;
    bool hasGridColumn = false;
    bool hasGridRowSpan = false;
    bool hasGridColumnSpan = false;
    for (const auto& ap : gridAttached) {
        if (ap.name == "Grid.Row") hasGridRow = true;
        if (ap.name == "Grid.Column") hasGridColumn = true;
        if (ap.name == "Grid.RowSpan") hasGridRowSpan = true;
        if (ap.name == "Grid.ColumnSpan") hasGridColumnSpan = true;
    }
    assert(hasGridRow);
    assert(hasGridColumn);
    assert(hasGridRowSpan);
    assert(hasGridColumnSpan);

    // 3. 透過 InspectorPanel 測試實際掛載與修改
    FormDocumentNode doc;
    auto canvasRoot = std::make_unique<WidgetNode>(NodeType::Canvas, "canvas1");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "btn1");
    std::string btnUuid = btn->GetUuid();

    assert(canvasRoot->AddChild(std::move(btn)).has_value());
    doc.SetRoot(std::move(canvasRoot));

    CommandHistoryManager history;
    SelectionModel sm;
    InspectorPanel inspector(&doc, &history, &sm);

    sm.SetSelection(btnUuid);
    auto props = inspector.GetInspectedProperties();

    auto leftProp = inspector.FindProperty("Canvas.Left");
    assert(leftProp.has_value());
    assert(leftProp->isAttached);

    // 修改附加屬性
    bool ok = inspector.ChangeAttachedProperty("Canvas.Left", PropertyValue(75.0));
    assert(ok);

    WidgetNode* foundBtn = doc.FindByUuid(btnUuid);
    assert(foundBtn != nullptr);
    auto val = foundBtn->GetAttachedProperty("Canvas.Left");
    assert(val.has_value());
    assert(val->Get<double>().value() == 75.0);

    compat::println("  [PASS] Context-aware attached properties verified.");
}

static void TestPropertyEditorsAndThemeTokens() {
    compat::println("[Test 4] Specialized Property Editors & Theme Token Integration...");

    // 1. Bool 編輯器
    auto b1 = PropertyEditorModel::ParseBool("true");
    assert(b1.has_value() && b1->Get<bool>().value() == true);
    auto b2 = PropertyEditorModel::ParseBool("0");
    assert(b2.has_value() && b2->Get<bool>().value() == false);
    auto bErr = PropertyEditorModel::ParseBool("invalid");
    assert(!bErr.has_value());

    // 2. 數值編輯器
    auto n1 = PropertyEditorModel::ParseNumber("128", true);
    assert(n1.has_value() && n1->Get<int32_t>().value() == 128);
    auto n2 = PropertyEditorModel::ParseNumber("3.14159", false);
    assert(n2.has_value() && std::abs(n2->Get<double>().value() - 3.14159) < 1e-4);

    // 3. Thickness 四欄連動解析
    auto th1 = PropertyEditorModel::ParseThickness("16");
    assert(th1.has_value());
    assert(th1->Get<core::Thickness>().value() == core::Thickness(16.0f, 16.0f, 16.0f, 16.0f));

    auto th2 = PropertyEditorModel::ParseThickness("8, 16");
    assert(th2.has_value());
    assert(th2->Get<core::Thickness>().value() == core::Thickness(8.0f, 16.0f, 8.0f, 16.0f));

    auto th4 = PropertyEditorModel::ParseThickness("1, 2, 3, 4");
    assert(th4.has_value());
    assert(th4->Get<core::Thickness>().value() == core::Thickness(1.0f, 2.0f, 3.0f, 4.0f));

    // 4. Color 十六進位解析
    auto col1 = PropertyEditorModel::ParseColor("#FF5722");
    assert(col1.has_value());
    assert(col1->Get<core::Color>().value() == (core::Color{ 255, 87, 34, 255 }));

    // 5. Enum 解析
    auto align = PropertyEditorModel::ParseEnum("HorizontalAlignment", "Center");
    assert(align.has_value());
    assert(align->Get<layout::HorizontalAlignment>().value() == layout::HorizontalAlignment::Center);

    // 6. ThemeToken 整合
    auto tokenVal = PropertyEditorModel::CreateThemeTokenValue(ThemeTokenType::Color, "accentPrimary");
    assert(tokenVal.Is<ThemeTokenRef>());
    assert(tokenVal.Get<ThemeTokenRef>().value().tokenName == "accentPrimary");
    assert(PropertyEditorModel::FormatDisplayString(tokenVal, false) == "@accentPrimary");

    compat::println("  [PASS] Specialized property editors & theme tokens verified.");
}

static void TestInputDefenseAndBoundarySafety() {
    compat::println("[Test 5] Input Defense & Extreme Value Safety...");

    // 1. 數值非數字字元防禦
    auto resNonNum = PropertyValidators::ValidateDouble("abc");
    assert(!resNonNum.isValid);

    // 2. NaN 與 Inf 邊界防禦
    auto resNaN = PropertyValidators::ValidateDouble("NaN");
    assert(!resNaN.isValid);

    auto resInf = PropertyValidators::ValidateDouble("inf");
    assert(!resInf.isValid);

    auto resNegInf = PropertyValidators::ValidateDouble("-infinity");
    assert(!resNegInf.isValid);

    // 3. 尺寸維度防禦 (禁止負數)
    auto resNegDim = PropertyValidators::ValidateDimension("-20");
    assert(!resNegDim.isValid);

    auto resPosDim = PropertyValidators::ValidateDimension("320.0");
    assert(resPosDim.isValid);

    // 4. Thickness 非法欄位數防禦
    auto resThick3 = PropertyValidators::ValidateThickness("1, 2, 3");
    assert(!resThick3.isValid);

    auto resThickNeg = PropertyValidators::ValidateThickness("10, -5");
    assert(!resThickNeg.isValid);

    // 5. 色彩十六進位格式防禦
    auto resHexInvalid = PropertyValidators::ValidateColorHex("#ZZ0011");
    assert(!resHexInvalid.isValid);

    auto resHexLen = PropertyValidators::ValidateColorHex("12345");
    assert(!resHexLen.isValid);

    // 6. C++ 識別字合法性防禦 (保留字與非法字元)
    auto resKw = PropertyValidators::ValidateIdentifier("class");
    assert(!resKw.isValid);

    auto resDigitStart = PropertyValidators::ValidateIdentifier("1button");
    assert(!resDigitStart.isValid);

    auto resValidId = PropertyValidators::ValidateIdentifier("submitButton_1");
    assert(resValidId.isValid);

    compat::println("  [PASS] Input defense & extreme value safety verified.");
}

static void TestEventAutoNamingAndAstBinding() {
    compat::println("[Test 6] Event Auto-Naming & AST Binding...");

    // 1. 自動命名常規化測試
    assert(InheritanceMetadataRegistry::GenerateDefaultHandlerName("submitBtn", "Click") == "OnSubmitBtnClick");
    assert(InheritanceMetadataRegistry::GenerateDefaultHandlerName("txtInput", "TextChanged") == "OnTxtInputTextChanged");
    assert(InheritanceMetadataRegistry::GenerateDefaultHandlerName("", "Click") == "OnClick");

    // 2. 透過 Inspector 自動綁定至 AST
    FormDocumentNode doc;
    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "root");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "saveBtn");
    std::string btnUuid = btn->GetUuid();

    assert(root->AddChild(std::move(btn)).has_value());
    doc.SetRoot(std::move(root));

    CommandHistoryManager history;
    SelectionModel sm;
    InspectorPanel inspector(&doc, &history, &sm);

    sm.SetSelection(btnUuid);
    inspector.SetCurrentTab(InspectorTab::Events);

    auto events = inspector.GetInspectedEvents();
    assert(!events.empty());

    // 執行自動綁定
    std::string handler = inspector.AutoBindEvent("Click");
    assert(handler == "OnSaveBtnClick");

    // 驗證 AST 寫入
    WidgetNode* foundBtn = doc.FindByUuid(btnUuid);
    assert(foundBtn != nullptr);
    const auto& bindings = foundBtn->GetEventBindings();
    assert(bindings.size() == 1);
    assert(bindings[0].eventName == "Click");
    assert(bindings[0].handlerName == "OnSaveBtnClick");

    // 驗證解除綁定
    bool unbindOk = inspector.UnbindEvent("Click");
    assert(unbindOk);
    assert(foundBtn->GetEventBindings().empty());

    compat::println("  [PASS] Event auto-naming & AST binding verified.");
}

static void TestBatchMacroCommandUndoRedo() {
    compat::println("[Test 7] Multi-Selection Batch MacroCommand Undo/Redo...");

    FormDocumentNode doc;
    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "root");
    auto btn1 = std::make_unique<WidgetNode>(NodeType::Button, "btn1");
    auto btn2 = std::make_unique<WidgetNode>(NodeType::Button, "btn2");

    std::string uuid1 = btn1->GetUuid();
    std::string uuid2 = btn2->GetUuid();

    btn1->SetProperty("Width", PropertyValue(100.0));
    btn2->SetProperty("Width", PropertyValue(100.0));

    assert(root->AddChild(std::move(btn1)).has_value());
    assert(root->AddChild(std::move(btn2)).has_value());
    doc.SetRoot(std::move(root));

    CommandHistoryManager history;
    SelectionModel sm;
    InspectorPanel inspector(&doc, &history, &sm);

    // 多選兩個按鈕
    sm.SetSelection(uuid1);
    sm.AddToSelection(uuid2);
    assert(sm.GetSelectionCount() == 2);

    // 批次修改屬性
    bool ok = inspector.ChangeProperty("Width", PropertyValue(250.0));
    assert(ok);

    WidgetNode* n1 = doc.FindByUuid(uuid1);
    WidgetNode* n2 = doc.FindByUuid(uuid2);
    assert(n1->GetProperty("Width")->Get<double>().value() == 250.0);
    assert(n2->GetProperty("Width")->Get<double>().value() == 250.0);

    // 驗證單次 Undo 同時還原兩個節點
    assert(history.CanUndo());
    auto undoRes = history.Undo(doc);
    assert(undoRes.has_value());

    assert(n1->GetProperty("Width")->Get<double>().value() == 100.0);
    assert(n2->GetProperty("Width")->Get<double>().value() == 100.0);

    // 驗證單次 Redo 同時前進兩個節點
    assert(history.CanRedo());
    auto redoRes = history.Redo(doc);
    assert(redoRes.has_value());

    assert(n1->GetProperty("Width")->Get<double>().value() == 250.0);
    assert(n2->GetProperty("Width")->Get<double>().value() == 250.0);

    compat::println("  [PASS] Multi-selection batch MacroCommand Undo/Redo verified.");
}

static void TestResetToDefaultIntegrity() {
    compat::println("[Test 8] Reset to Default Integrity...");

    FormDocumentNode doc;
    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "root");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "resetBtn");
    std::string uuid = btn->GetUuid();

    // 修改預設值
    btn->SetProperty("IsEnabled", PropertyValue(false));

    assert(root->AddChild(std::move(btn)).has_value());
    doc.SetRoot(std::move(root));

    CommandHistoryManager history;
    SelectionModel sm;
    InspectorPanel inspector(&doc, &history, &sm);

    sm.SetSelection(uuid);
    WidgetNode* node = doc.FindByUuid(uuid);
    assert(node->GetProperty("IsEnabled")->Get<bool>().value() == false);

    // 執行 Reset to Default
    bool resetOk = inspector.ResetPropertyToDefault("IsEnabled");
    assert(resetOk);

    // 預設值為 true
    assert(node->GetProperty("IsEnabled")->Get<bool>().value() == true);

    // Undo 復原為 false
    assert(history.CanUndo());
    history.Undo(doc);
    assert(node->GetProperty("IsEnabled")->Get<bool>().value() == false);

    compat::println("  [PASS] Reset to Default integrity verified.");
}

static void TestDefectFixesRegression() {
    compat::println("[Test 9] Phase 8 Defect Fixes Regression Tests...");

    // a) Verify Color Property Parsing & Editing
    // 1. Valid hex '#336699'
    auto c1 = PropertyEditorModel::ParseColor("#336699");
    assert(c1.has_value());
    auto color1 = c1->Get<core::Color>().value();
    assert(color1.r == 0x33);
    assert(color1.g == 0x66);
    assert(color1.b == 0x99);
    assert(color1.a == 255);
    assert(color1 == (core::Color{ 0x33, 0x66, 0x99, 255 }));

    // 2. Valid hex '336699' (without # prefix)
    auto c2 = PropertyEditorModel::ParseColor("336699");
    assert(c2.has_value());
    auto color2 = c2->Get<core::Color>().value();
    assert(color2.r == 0x33);
    assert(color2.g == 0x66);
    assert(color2.b == 0x99);
    assert(color2.a == 255);
    assert(color2 == (core::Color{ 0x33, 0x66, 0x99, 255 }));

    // 3. Valid hex '#AABBCCDD' (with alpha component)
    auto c3 = PropertyEditorModel::ParseColor("#AABBCCDD");
    assert(c3.has_value());
    auto color3 = c3->Get<core::Color>().value();
    assert(color3.r == 0xAA);
    assert(color3.g == 0xBB);
    assert(color3.b == 0xCC);
    assert(color3.a == 0xDD);
    assert(color3 == (core::Color{ 0xAA, 0xBB, 0xCC, 0xDD }));

    // 4. Invalid hex strings
    auto cInvalid = PropertyEditorModel::ParseColor("#invalid");
    assert(!cInvalid.has_value());

    auto cEmpty = PropertyEditorModel::ParseColor("");
    assert(!cEmpty.has_value());

    // AST and Inspector setup for editing tests
    FormDocumentNode doc;
    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "SubmitBtn");
    std::string btnUuid = btn->GetUuid();

    assert(root->AddChild(std::move(btn)).has_value());
    doc.SetRoot(std::move(root));

    CommandHistoryManager history;
    SelectionModel sm;
    InspectorPanel inspector(&doc, &history, &sm);

    sm.SetSelection(btnUuid);

    // Verify changing a color property on a WidgetNode via inspector.ChangeProperty
    core::Color expectedColor{ 0x33, 0x66, 0x99, 255 };
    bool colorChangeOk = inspector.ChangeProperty("Background", PropertyValue(expectedColor));
    assert(colorChangeOk);

    WidgetNode* btnNode = doc.FindByUuid(btnUuid);
    assert(btnNode != nullptr);
    auto bgProp = btnNode->GetProperty("Background");
    assert(bgProp.has_value());
    assert(bgProp->Get<core::Color>().value() == expectedColor);

    // b) Verify Event Binding & Unbinding
    inspector.SetCurrentTab(InspectorTab::Events);
    std::string autoHandler = inspector.AutoBindEvent("Click");
    assert(autoHandler == "OnSubmitBtnClick");

    const auto& eventBindings = btnNode->GetEventBindings();
    assert(eventBindings.size() == 1);
    assert(eventBindings[0].eventName == "Click");
    assert(eventBindings[0].handlerName == "OnSubmitBtnClick");

    bool unbindOk = inspector.UnbindEvent("Click");
    assert(unbindOk);
    assert(btnNode->GetEventBindings().empty());

    // c) Verify Boolean Property Click Handling
    inspector.SetCurrentTab(InspectorTab::Properties);
    bool setFalseOk = inspector.ChangeProperty("IsEnabled", PropertyValue(false));
    assert(setFalseOk);
    auto boolProp = btnNode->GetProperty("IsEnabled");
    assert(boolProp.has_value());
    assert(boolProp->Get<bool>().value() == false);

    bool currVal = boolProp->Get<bool>().value();
    bool toggleOk = inspector.ChangeProperty("IsEnabled", PropertyValue(!currVal));
    assert(toggleOk);
    auto toggledProp = btnNode->GetProperty("IsEnabled");
    assert(toggledProp.has_value());
    assert(toggledProp->Get<bool>().value() == true);

    compat::println("  [PASS] Phase 8 defect fixes regression verified.");
}

int main() {
    compat::println("=== CPPUI Designer Phase 4 Inspector Test Suite ===");

    TestInheritanceMetadataRegistry();
    TestMultiSelectionIntersectionAndMixed();
    TestContextAwareAttachedProperties();
    TestPropertyEditorsAndThemeTokens();
    TestInputDefenseAndBoundarySafety();
    TestEventAutoNamingAndAstBinding();
    TestBatchMacroCommandUndoRedo();
    TestResetToDefaultIntegrity();
    TestDefectFixesRegression();

    compat::println("=== All Phase 4 Inspector Tests Passed Successfully! ===");
    return 0;
}
