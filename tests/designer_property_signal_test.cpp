/**
 * @file designer_property_signal_test.cpp
 * @brief PropertyValue 型別安全、附加屬性、主題 Token 與反應式 Signal 匯流排測試
 */

#include <cassert>
#include <iostream>
#include "compat/Compat.hpp"
#include "ui/Designer.hpp"
#include "ui/core/LifetimeToken.hpp"

using namespace cppui;
using namespace cppui::designer;

static void TestPropertyValueTypeSafetyAndConversions() {
    compat::println("--- Running TestPropertyValueTypeSafetyAndConversions ---");

    PropertyValue vInt(static_cast<int32_t>(42));
    assert(vInt.Is<int32_t>());
    assert(!vInt.Is<std::string>());
    assert(vInt.Get<int32_t>().value() == 42);
    assert(!vInt.Get<std::string>().has_value());
    assert(vInt.Get<std::string>().error().code == DesignerErrorCode::TypeMismatch);
    assert(vInt.ToString() == "42");

    PropertyValue vTh(core::Thickness(4.0f, 8.0f, 12.0f, 16.0f));
    assert(vTh.Is<core::Thickness>());
    assert(vTh.Get<core::Thickness>().value().right == 12.0f);
    assert(vTh.ToString() == "4,8,12,16");

    PropertyValue vColor(core::Color{ 255, 0, 128, 255 });
    assert(vColor.Is<core::Color>());
    assert(vColor.ToString() == "#ffff0080");

    ThemeTokenRef tok;
    tok.tokenType = ThemeTokenType::Color;
    tok.tokenName = "accentPrimary";
    PropertyValue vToken(tok);
    assert(vToken.Is<ThemeTokenRef>());
    assert(vToken.Get<ThemeTokenRef>().value().tokenName == "accentPrimary");
    assert(vToken.ToString() == "@Theme:accentPrimary");
}

static void TestPropertyDescriptorDefaultValues() {
    compat::println("--- Running TestPropertyDescriptorDefaultValues ---");

    // 驗證 Button 預設屬性
    auto btnContent = PropertyMetadataRegistry::GetDefaultValue(NodeType::Button, "Content");
    assert(btnContent.has_value());
    assert(btnContent->Get<std::string>().value() == "");

    assert(PropertyMetadataRegistry::IsDefaultValue(NodeType::Button, "Content", PropertyValue(std::string(""))));
    assert(!PropertyMetadataRegistry::IsDefaultValue(NodeType::Button, "Content", PropertyValue(std::string("送出"))));

    // 驗證 TextBlock 預設屬性
    auto tbFontSize = PropertyMetadataRegistry::GetDefaultValue(NodeType::TextBlock, "FontSize");
    assert(tbFontSize.has_value());
    assert(tbFontSize->Get<double>().value() == 14.0);
}

static void TestAttachedPropertiesValidation() {
    compat::println("--- Running TestAttachedPropertiesValidation ---");

    // Grid.Row
    assert(AttachedProperties::Validate(AttachedProperties::GridRow, PropertyValue(static_cast<int32_t>(0))).has_value());
    assert(AttachedProperties::Validate(AttachedProperties::GridRow, PropertyValue(static_cast<int32_t>(3))).has_value());
    assert(!AttachedProperties::Validate(AttachedProperties::GridRow, PropertyValue(static_cast<int32_t>(-1))).has_value());
    assert(!AttachedProperties::Validate(AttachedProperties::GridRow, PropertyValue(std::string("invalid"))).has_value());

    // Grid.RowSpan
    assert(AttachedProperties::Validate(AttachedProperties::GridRowSpan, PropertyValue(static_cast<int32_t>(1))).has_value());
    assert(!AttachedProperties::Validate(AttachedProperties::GridRowSpan, PropertyValue(static_cast<int32_t>(0))).has_value());

    // Canvas.Left
    assert(AttachedProperties::Validate(AttachedProperties::CanvasLeft, PropertyValue(120.5)).has_value());
    assert(!AttachedProperties::Validate(AttachedProperties::CanvasLeft, PropertyValue(true)).has_value());

    // ToolTip.Text
    assert(AttachedProperties::Validate(AttachedProperties::ToolTipText, PropertyValue(std::string("提示資訊"))).has_value());
}

static void TestReactiveSignalChangeBus() {
    compat::println("--- Running TestReactiveSignalChangeBus ---");

    FormDocumentNode doc;
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "grid1");
    std::string rootUuid = rootGrid->GetUuid();
    doc.SetRoot(std::move(rootGrid));

    int32_t addCount = 0;
    std::string lastAddedUuid = "";
    auto connAdd = doc.NodeAddedSignal().Connect([&](const NodeAddedPayload& payload) {
        addCount++;
        lastAddedUuid = payload.childUuid;
    });

    int32_t propChangeCount = 0;
    std::string lastPropName = "";
    auto connProp = doc.PropertyChangedSignal().Connect([&](const PropertyChangedPayload& payload) {
        propChangeCount++;
        lastPropName = payload.propertyName;
    });

    // 觸發新增節點通知
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "button1");
    std::string btnUuid = btn->GetUuid();
    assert(doc.GetRoot()->AddChild(std::move(btn)).has_value());
    doc.NodeAddedSignal().Notify({ rootUuid, btnUuid, 0 });

    assert(addCount == 1);
    assert(lastAddedUuid == btnUuid);

    // 觸發屬性修改通知
    doc.PropertyChangedSignal().Notify({ btnUuid, "Content", PropertyValue(std::string("")), PropertyValue(std::string("確定")) });
    assert(propChangeCount == 1);
    assert(lastPropName == "Content");

    connAdd.Disconnect();
    connProp.Disconnect();
}

static void TestLifetimeTokenProtection() {
    compat::println("--- Running TestLifetimeTokenProtection ---");

    FormDocumentNode doc;
    int32_t triggerCount = 0;

    core::LifetimeToken token = core::LifetimeToken::Create();
    doc.PropertyChangedSignal().Connect(token, [&](const PropertyChangedPayload&) {
        triggerCount++;
    });

    // Token 存活期內，能正常觸發
    doc.PropertyChangedSignal().Notify({ "uuid", "Width", PropertyValue(0.0), PropertyValue(100.0) });
    assert(triggerCount == 1);

    // 使 Token 失效 (模擬宿主物件解構時呼叫 Invalidate)
    token.Invalidate();

    // Token 失效後，不應再收到通知，且絕不觸發 Dangling Callback
    doc.PropertyChangedSignal().Notify({ "uuid", "Width", PropertyValue(100.0), PropertyValue(200.0) });
    assert(triggerCount == 1);
}

static void TestCodePreviewWindowLifecycleAndSignalUnsubscribe() {
    compat::println("--- Running TestCodePreviewWindowLifecycleAndSignalUnsubscribe ---");

    FormDocumentNode doc;
    auto rootCanvas = std::make_unique<WidgetNode>(NodeType::Canvas, "rootCanvas");
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "btn1");
    std::string btnUuid = btn->GetUuid();
    assert(rootCanvas->AddChild(std::move(btn)).has_value());
    doc.SetRoot(std::move(rootCanvas));

    surface::SelectionModel sm;

    // 1. 驗證 CodePreviewWindow 於獨立作用域中建構、連線並解構
    {
        auto previewWin = std::make_shared<preview::CodePreviewWindow>(&doc, &sm);
        previewWin->RefreshCode();
        assert(!previewWin->GetCurrentCode().empty());
        // 作用域結束，previewWin 析構 -> Impl 析構 -> ScopedConnection 自動 Disconnect + LifetimeToken 失效
    }

    // 2. 視窗解構後觸發屬性廣播 (模擬 InspectorPanel 修改屬性)
    // 若信號未取消註冊，殘留的 lambda 會引發 Heap-use-after-free (ASan 崩潰)
    doc.PropertyChangedSignal().Notify({ btnUuid, "Width", PropertyValue(100.0), PropertyValue(200.0) });
    doc.NodeAddedSignal().Notify({ "rootCanvas", "btn2", 1 });
    doc.NodeMovedSignal().Notify({ btnUuid, "rootCanvas", "rootCanvas", 1 });
    doc.NodeRemovedSignal().Notify({ "rootCanvas", "btn2" });
    doc.SettingsChangedSignal().Notify({ doc.GetSettings(), doc.GetSettings() });

    // 3. 驗證 LiveCodePreviewPanel 於獨立作用域中建構並解構
    {
        auto previewPanel = std::make_shared<preview::LiveCodePreviewPanel>(&doc, &sm);
        previewPanel->SetViewMode(preview::PreviewViewMode::ClassicSource);
        assert(!previewPanel->GetActiveCodeText().empty());
        // 作用域結束，previewPanel 析構 -> ScopedConnection 自動 Disconnect
    }

    // 4. 面板解構後再次觸發廣播
    doc.PropertyChangedSignal().Notify({ btnUuid, "Height", PropertyValue(50.0), PropertyValue(80.0) });
    doc.SettingsChangedSignal().Notify({ doc.GetSettings(), doc.GetSettings() });

    compat::println("  [PASS] CodePreviewWindow & LiveCodePreviewPanel lifecycle unsubscription verified (0 UAF).");
}

int main() {
    compat::println("==================================================");
    compat::println("Starting designer_property_signal_test...");
    compat::println("==================================================");

    TestPropertyValueTypeSafetyAndConversions();
    TestPropertyDescriptorDefaultValues();
    TestAttachedPropertiesValidation();
    TestReactiveSignalChangeBus();
    TestLifetimeTokenProtection();
    TestCodePreviewWindowLifecycleAndSignalUnsubscribe();

    compat::println("All designer_property_signal_test passed successfully!");
    return 0;
}
