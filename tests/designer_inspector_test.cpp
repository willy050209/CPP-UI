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
#include "ui/controls/Button.hpp"
#include "ui/layout/Border.hpp"

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

static void TestTypographyAndFontWeightSupport() {
    compat::println("[Test 10] Typography & FontWeight Enum Options, Parsing, and AST Binding...");

    // 1. 驗證 FontWeight 與 FontFamily enum 選項
    auto fwOptions = InheritanceMetadataRegistry::GetEnumOptions("FontWeight");
    assert(!fwOptions.empty());
    bool hasBold = false;
    bool hasNormal = false;
    for (const auto& opt : fwOptions) {
        if (opt == "Bold") hasBold = true;
        if (opt == "Normal") hasNormal = true;
    }
    assert(hasBold);
    assert(hasNormal);

    auto ffOptions = InheritanceMetadataRegistry::GetEnumOptions("FontFamily");
    assert(!ffOptions.empty());
    bool hasSegoe = false;
    for (const auto& opt : ffOptions) {
        if (opt == "Segoe UI") hasSegoe = true;
    }
    assert(hasSegoe);

    // 2. 驗證 PropertyEditorModel::ParseEnum 針對 FontWeight 的整數對映
    auto parsedBold = PropertyEditorModel::ParseEnum("FontWeight", "Bold");
    assert(parsedBold.has_value());
    assert(parsedBold->Is<int32_t>());
    assert(parsedBold->Get<int32_t>().value() == 700);

    auto parsedNormal = PropertyEditorModel::ParseEnum("FontWeight", "Normal");
    assert(parsedNormal.has_value());
    assert(parsedNormal->Is<int32_t>());
    assert(parsedNormal->Get<int32_t>().value() == 400);

    auto parsedSemiBold = PropertyEditorModel::ParseEnum("FontWeight", "SemiBold");
    assert(parsedSemiBold.has_value());
    assert(parsedSemiBold->Get<int32_t>().value() == 600);

    // 3. 驗證 PropertyEditorModel::ParseEnum 針對 Visibility 的列舉對映
    auto parsedVis = PropertyEditorModel::ParseEnum("Visibility", "Collapsed");
    assert(parsedVis.has_value());
    assert(parsedVis->Is<layout::Visibility>());
    assert(parsedVis->Get<layout::Visibility>().value() == layout::Visibility::Collapsed);

    // 4. 驗證 InspectorPanel 屬性套用至 AST
    FormDocumentNode doc;
    CommandHistoryManager history;
    SelectionModel selection;
    InspectorPanel inspector(&doc, &history, &selection);

    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas");
    auto textBlock = std::make_unique<WidgetNode>(NodeType::TextBlock, "labelTitle");
    textBlock->SetProperty("Text", PropertyValue(std::string("Hello World")));
    std::string tbUuid = textBlock->GetUuid();

    WidgetNode* rawTb = textBlock.get();
    root->AddChild(std::move(textBlock));
    doc.SetRoot(std::move(root));

    selection.SetSelection(tbUuid);

    // 透過 inspector 修改 FontWeight
    bool setFwOk = inspector.ChangeProperty("FontWeight", PropertyValue(int32_t(700)));
    assert(setFwOk);
    auto fwProp = rawTb->GetProperty("FontWeight");
    assert(fwProp.has_value());
    assert(fwProp->Get<int32_t>().value() == 700);

    // 透過 inspector 修改 FontSize
    bool setFsOk = inspector.ChangeProperty("FontSize", PropertyValue(18.0));
    assert(setFsOk);
    auto fsProp = rawTb->GetProperty("FontSize");
    assert(fsProp.has_value());
    assert(fsProp->Get<double>().value() == 18.0);

    // 透過 inspector 修改 FontFamily
    bool setFfOk = inspector.ChangeProperty("FontFamily", PropertyValue(std::string("Consolas")));
    assert(setFfOk);
    auto ffProp = rawTb->GetProperty("FontFamily");
    assert(ffProp.has_value());
    assert(ffProp->Get<std::string>().value() == "Consolas");

    compat::println("  [PASS] Typography and FontWeight options & AST binding verified.");
}

static void TestLiveRunButtonAndInspectorTextBoxIntegration() {
    compat::println("[Test 11] Button Background Polymorphism, LiveFormRunner & Real TextBox Inspector Integration...");

    // 1. Button 背景色多型指派測試
    std::shared_ptr<controls::Control> ctrl = std::make_shared<controls::Button>("ClickMe");
    core::Color expectedBtnBg{ 220, 80, 60, 255 };
    ctrl->SetBackground(expectedBtnBg);
    assert(ctrl->GetBackground() == expectedBtnBg);

    // 2. LiveFormRunner 實例化 Button 背景色還原測試 (包含 Hex 解析)
    WidgetNode btnNode(NodeType::Button, "OkButton");
    btnNode.SetProperty("Background", std::string("#336699"));
    btnNode.SetProperty("Content", std::string("確定"));
    auto liveElem = preview::LiveFormRunner::CreateWidget(btnNode);
    assert(liveElem != nullptr);
    auto liveBtn = std::dynamic_pointer_cast<controls::Button>(liveElem);
    assert(liveBtn != nullptr);
    assert(liveBtn->GetBackground() == (core::Color{ 0x33, 0x66, 0x99, 255 }));

    // 3. LiveFormRunner 實例化 Border 背景色還原測試
    WidgetNode borderNode(NodeType::Border, "MainBorder");
    borderNode.SetProperty("Background", std::string("#123456"));
    borderNode.SetProperty("BorderBrush", std::string("#654321"));
    auto liveBorderElem = preview::LiveFormRunner::CreateWidget(borderNode);
    assert(liveBorderElem != nullptr);
    auto liveBorder = std::dynamic_pointer_cast<layout::Border>(liveBorderElem);
    assert(liveBorder != nullptr);
    assert(liveBorder->GetBackground() == (core::Color{ 0x12, 0x34, 0x56, 255 }));
    assert(liveBorder->GetBorderBrush() == (core::Color{ 0x65, 0x43, 0x21, 255 }));

    // 4. InspectorPanel 嵌入之真實 TextBox 視覺樹與搜尋過濾驗證
    FormDocumentNode doc;
    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "CanvasRoot");
    auto childBtn = std::make_unique<WidgetNode>(NodeType::Button, "TargetButton");
    std::string childUuid = childBtn->GetUuid();
    root->AddChild(std::move(childBtn));
    doc.SetRoot(std::move(root));

    CommandHistoryManager history;
    SelectionModel selection;
    selection.SetSelection(childUuid);

    InspectorPanel inspector(&doc, &history, &selection);
    // 驗證 InspectorPanel 已包含真實 TextBox 子元件 (搜尋框與行內編輯器)
    assert(inspector.GetVisualChildrenCount() >= 2);

    // 驗證排版測量與排列呼叫
    inspector.Measure(core::Size(320.0f, 600.0f));
    inspector.Arrange(core::Rect(0.0f, 0.0f, 320.0f, 600.0f));

    // 驗證搜尋過濾
    inspector.SetFilterText("Width");
    assert(inspector.GetFilterText() == "Width");
    auto filteredProps = inspector.GetInspectedProperties();
    assert(!filteredProps.empty());
    for (const auto& p : filteredProps) {
        // 屬性名稱應包含 "width" (不區分大小寫)
        std::string pName = p.name;
        for (char& ch : pName) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        assert(pName.find("width") != std::string::npos);
    }

    // 重設搜尋過濾
    inspector.SetFilterText("");
    assert(inspector.GetFilterText().empty());
    auto allProps = inspector.GetInspectedProperties();
    assert(allProps.size() > filteredProps.size());

    // 驗證屬性直接設定與 AST 連結
    bool setWidthOk = inspector.ChangeProperty("Width", PropertyValue(180.0));
    assert(setWidthOk);
    WidgetNode* targetNode = doc.FindByUuid(childUuid);
    assert(targetNode != nullptr);
    auto widthProp = targetNode->GetProperty("Width");
    assert(widthProp.has_value());
    assert(widthProp->Get<double>().value() == 180.0);

    compat::println("  [PASS] Button background polymorphism, LiveFormRunner & Inspector TextBox integration verified.");
}

/// <summary>
/// 驗證 AutoSize 與 AutoSizeMode 屬性註冊、列舉選項與 FormDocument / WidgetNode 檢查器讀寫。
/// </summary>
static void TestAutoSizeInspectorAndMetadata() {
    compat::println("[Test 12] AutoSize & AutoSizeMode Registry & Inspector Metadata Integration...");

    // 1. 驗證 Control (Button) 包含 AutoSize 屬性
    const auto& btnProps = InheritanceMetadataRegistry::GetDescriptorsForType(NodeType::Button);
    bool btnHasAutoSize = false;
    for (const auto& p : btnProps) {
        if (p.name == "AutoSize") {
            btnHasAutoSize = true;
            assert(p.category == PropertyCategory::Layout);
            assert(p.defaultValue.Is<bool>());
            assert(p.defaultValue.Get<bool>().value_or(true) == false);
        }
    }
    assert(btnHasAutoSize);

    // 2. 驗證 FormDocument 包含 AutoSize 與 AutoSizeMode 屬性
    const auto& formProps = InheritanceMetadataRegistry::GetDescriptorsForType(NodeType::FormDocument);
    bool formHasAutoSize = false;
    bool formHasAutoSizeMode = false;
    for (const auto& p : formProps) {
        if (p.name == "AutoSize") {
            formHasAutoSize = true;
            assert(p.category == PropertyCategory::Layout);
            assert(p.defaultValue.Is<bool>());
            assert(p.defaultValue.Get<bool>().value_or(true) == false);
        }
        if (p.name == "AutoSizeMode") {
            formHasAutoSizeMode = true;
            assert(p.category == PropertyCategory::Layout);
            assert(p.defaultValue.Is<std::string>());
            assert(p.defaultValue.Get<std::string>().value_or("") == "GrowAndShrink");
        }
    }
    assert(formHasAutoSize);
    assert(formHasAutoSizeMode);

    // 3. 驗證 AutoSizeMode 列舉選項
    auto autoSizeModeOptions = InheritanceMetadataRegistry::GetEnumOptions("AutoSizeMode");
    assert(autoSizeModeOptions.size() == 2);
    assert(autoSizeModeOptions[0] == "GrowAndShrink");
    assert(autoSizeModeOptions[1] == "GrowOnly");

    // 4. 驗證 InspectorPanel 在無選取 (FormDocument) 模式下讀取與修改 AutoSize / AutoSizeMode
    FormDocumentNode doc;
    CommandHistoryManager history;
    SelectionModel sm;
    InspectorPanel inspector(&doc, &history, &sm);

    // 讀取 FormDocument 預設值
    auto formInspectedProps = inspector.GetInspectedProperties();
    (void)formInspectedProps;
    auto formAutoSizeProp = inspector.FindProperty("AutoSize");
    assert(formAutoSizeProp.has_value());
    assert(formAutoSizeProp->currentValue.Is<bool>());
    assert(formAutoSizeProp->currentValue.Get<bool>().value_or(true) == false);

    auto formAutoSizeModeProp = inspector.FindProperty("AutoSizeMode");
    assert(formAutoSizeModeProp.has_value());
    assert(formAutoSizeModeProp->currentValue.Is<std::string>());
    assert(formAutoSizeModeProp->currentValue.Get<std::string>().value_or("") == "GrowAndShrink");
    assert(formAutoSizeModeProp->enumOptions.size() == 2);

    // 修改 FormDocument AutoSize 為 true
    bool changeAutoSizeOk = inspector.ChangeProperty("AutoSize", PropertyValue(true));
    assert(changeAutoSizeOk);
    assert(doc.GetSettings().autoSize == true);

    // 修改 FormDocument AutoSizeMode 為 "GrowOnly"
    bool changeModeOk = inspector.ChangeProperty("AutoSizeMode", PropertyValue(std::string("GrowOnly")));
    assert(changeModeOk);
    assert(doc.GetSettings().autoSizeMode == "GrowOnly");

    // 重新透過 Inspector 讀取更新後的數值
    auto updatedAutoSizeProp = inspector.FindProperty("AutoSize");
    assert(updatedAutoSizeProp.has_value());
    assert(updatedAutoSizeProp->currentValue.Get<bool>().value_or(false) == true);

    auto updatedModeProp = inspector.FindProperty("AutoSizeMode");
    assert(updatedModeProp.has_value());
    assert(updatedModeProp->currentValue.Get<std::string>().value_or("") == "GrowOnly");

    // 5. 驗證控制項節點之 AutoSize 修改
    auto root = std::make_unique<WidgetNode>(NodeType::Canvas, "CanvasRoot");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "MyButton");
    std::string btnUuid = btn->GetUuid();
    root->AddChild(std::move(btn));
    doc.SetRoot(std::move(root));

    sm.SetSelection(btnUuid);
    bool setBtnAutoSizeOk = inspector.ChangeProperty("AutoSize", PropertyValue(true));
    assert(setBtnAutoSizeOk);

    WidgetNode* node = doc.FindByUuid(btnUuid);
    assert(node != nullptr);
    auto nodeAutoSize = node->GetProperty("AutoSize");
    assert(nodeAutoSize.has_value());
    assert(nodeAutoSize->Is<bool>());
    assert(nodeAutoSize->Get<bool>().value_or(false) == true);

    compat::println("  [PASS] AutoSize & AutoSizeMode registry & inspector metadata integration verified.");
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
    TestTypographyAndFontWeightSupport();
    TestLiveRunButtonAndInspectorTextBoxIntegration();
    TestAutoSizeInspectorAndMetadata();

    compat::println("=== All Phase 4 Inspector Tests Passed Successfully! ===");
    return 0;
}
