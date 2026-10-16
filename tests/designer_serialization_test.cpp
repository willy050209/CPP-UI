/**
 * @file designer_serialization_test.cpp
 * @brief .cppui.json 無損序列化 (Round-Trip)、預設值剔除與損毀輸入容錯單元測試
 */

#include <cassert>
#include <iostream>
#include "compat/Compat.hpp"
#include "ui/Designer.hpp"

using namespace cppui;
using namespace cppui::designer;

static void TestRoundTripLossless() {
    compat::println("--- Running TestRoundTripLossless ---");

    FormSettings s;
    s.className = "ComplexForm";
    s.targetNamespace = "EnterpriseApp";
    s.title = "企業級表單測試";
    s.initialSize = core::Size(1024.0f, 768.0f);
    s.defaultTheme = "Dark";
    s.useMvvm = true;
    s.useIoC = true;

    FormDocumentNode doc1(s);

    // Root Grid
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "rootGrid");
    rootGrid->SetRowDefinitions({
        layout::RowDefinition(layout::GridLength::Auto()),
        layout::RowDefinition(layout::GridLength::Star(1.0f)),
        layout::RowDefinition(layout::GridLength::Pixel(50.0f))
    });
    rootGrid->SetColumnDefinitions({
        layout::ColumnDefinition(layout::GridLength::Auto()),
        layout::ColumnDefinition(layout::GridLength::Star(1.0f))
    });

    // 1. StackPanel
    auto stack = std::make_unique<WidgetNode>(NodeType::StackPanel, "navStack");
    stack->SetProperty("Orientation", PropertyValue(layout::Orientation::Horizontal));
    stack->SetProperty("Spacing", PropertyValue(8.0));
    stack->SetAttachedProperty(AttachedProperties::GridRow, PropertyValue(static_cast<int32_t>(0)));
    stack->SetAttachedProperty(AttachedProperties::GridColumn, PropertyValue(static_cast<int32_t>(0)));

    // 2. Button
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "submitBtn");
    btn->SetProperty("Content", PropertyValue(std::string("送出表單")));
    btn->SetProperty("Width", PropertyValue(120.0));
    btn->SetProperty("Height", PropertyValue(36.0));
    btn->AddEventBinding({ "Click", "OnSubmitClicked" });
    btn->AddDataBinding({ "Content", "ViewModel.SubmitLabel", BindingMode::OneWay });

    // 3. TextBox
    auto tb = std::make_unique<WidgetNode>(NodeType::TextBox, "inputAccount");
    tb->SetProperty("Placeholder", PropertyValue(std::string("請輸入帳號...")));
    tb->SetProperty("Width", PropertyValue(200.0));
    tb->AddDataBinding({ "Text", "ViewModel.AccountName", BindingMode::TwoWay });

    assert(stack->AddChild(std::move(btn)).has_value());
    assert(stack->AddChild(std::move(tb)).has_value());
    assert(rootGrid->AddChild(std::move(stack)).has_value());
    doc1.SetRoot(std::move(rootGrid));

    // 第一次序列化: AST1 -> JSON1
    auto ser1 = FormSerializer::Serialize(doc1);
    assert(ser1.has_value());
    const std::string& json1 = ser1.value();
    assert(!json1.empty());

    // 反序列化: JSON1 -> AST2
    auto deser1 = FormSerializer::Deserialize(json1);
    assert(deser1.has_value());
    std::unique_ptr<FormDocumentNode> doc2 = std::move(deser1.value());
    assert(doc2 != nullptr);

    // 第二次序列化: AST2 -> JSON2
    auto ser2 = FormSerializer::Serialize(*doc2);
    assert(ser2.has_value());
    const std::string& json2 = ser2.value();

    // 驗證確定性雜湊與逐位元完全相等 (Deterministic Equality)
    assert(json1 == json2);

    // 驗證 AST2 屬性與拓撲完全正確
    assert(doc2->GetSettings().className == "ComplexForm");
    assert(doc2->GetSettings().initialSize.width == 1024.0f);
    assert(doc2->GetRoot()->GetType() == NodeType::Grid);
    assert(doc2->GetRoot()->GetRowDefinitions().size() == 3);
    assert(doc2->GetRoot()->GetColumnDefinitions().size() == 2);
    assert(doc2->GetRoot()->GetChildCount() == 1);

    WidgetNode* stack2 = doc2->GetRoot()->GetChild(0);
    assert(stack2 != nullptr);
    assert(stack2->GetType() == NodeType::StackPanel);
    assert(stack2->GetChildCount() == 2);

    WidgetNode* btn2 = stack2->GetChild(0);
    assert(btn2 != nullptr);
    assert(btn2->GetProperty("Content")->Get<std::string>().value() == "送出表單");
    assert(btn2->GetEventBindings().size() == 1);
    assert(btn2->GetEventBindings()[0].handlerName == "OnSubmitClicked");
    assert(btn2->GetDataBindings().size() == 1);
    assert(btn2->GetDataBindings()[0].sourcePath == "ViewModel.SubmitLabel");
}

static void TestDefaultValueElision() {
    compat::println("--- Running TestDefaultValueElision ---");

    FormDocumentNode doc;
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "rootGrid");

    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "myBtn");
    // 設定未修改之預設屬性
    btn->SetProperty("Width", PropertyValue(0.0));
    btn->SetProperty("Visibility", PropertyValue(layout::Visibility::Visible));
    // 設定非預設屬性
    btn->SetProperty("Content", PropertyValue(std::string("確定")));

    assert(rootGrid->AddChild(std::move(btn)).has_value());
    doc.SetRoot(std::move(rootGrid));

    auto serRes = FormSerializer::Serialize(doc);
    assert(serRes.has_value());
    const std::string& json = serRes.value();

    // 驗證預設值剔除：未更動屬性不應被輸出至 JSON 字串中
    assert(json.find("\"Content\": \"確定\"") != std::string::npos);
    assert(json.find("\"Width\"") == std::string::npos);
    assert(json.find("\"Visibility\"") == std::string::npos);
}

static void TestMalformedJsonFaultTolerance() {
    compat::println("--- Running TestMalformedJsonFaultTolerance ---");

    // 1. 空字串
    auto r1 = FormSerializer::Deserialize("");
    assert(!r1.has_value());
    assert(r1.error().code == DesignerErrorCode::InvalidJson);

    // 2. 未結束之 JSON
    auto r2 = FormSerializer::Deserialize("{ \"schemaVersion\": \"1.0.0\", \"root\": {");
    assert(!r2.has_value());
    assert(r2.error().code == DesignerErrorCode::InvalidJson);

    // 3. 缺少 schemaVersion
    auto r3 = FormSerializer::Deserialize("{ \"root\": {} }");
    assert(!r3.has_value());
    assert(r3.error().code == DesignerErrorCode::SchemaMismatch);
    assert(r3.error().jsonPointer == "/schemaVersion");

    // 4. 未知節點型別
    auto r4 = FormSerializer::Deserialize(R"({
        "schemaVersion": "1.0.0",
        "root": {
            "type": "NonExistentUnknownControl",
            "uuid": "123",
            "id": "bad"
        }
    })");
    assert(!r4.has_value());
    assert(r4.error().code == DesignerErrorCode::InvalidJson);
    assert(r4.error().jsonPointer == "/root/type");

    // 5. 容器基數破壞 (Border 含有 2 個子節點)
    auto r5 = FormSerializer::Deserialize(R"({
        "schemaVersion": "1.0.0",
        "root": {
            "type": "Border",
            "uuid": "b1",
            "id": "border1",
            "children": [
                { "type": "Button", "uuid": "btn1", "id": "b1" },
                { "type": "Button", "uuid": "btn2", "id": "b2" }
            ]
        }
    })");
    assert(!r5.has_value());
    assert(r5.error().code == DesignerErrorCode::ContainerFull);
    assert(r5.error().jsonPointer == "/root/children/1");
}

int main() {
    compat::println("==================================================");
    compat::println("Starting designer_serialization_test...");
    compat::println("==================================================");

    TestRoundTripLossless();
    TestDefaultValueElision();
    TestMalformedJsonFaultTolerance();

    compat::println("All designer_serialization_test passed successfully!");
    return 0;
}
