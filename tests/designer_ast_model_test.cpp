/**
 * @file designer_ast_model_test.cpp
 * @brief Designer AST 節點模型、容器約束、循環引用與命名驗證單元測試
 */

#include <cassert>
#include <iostream>
#include "compat/Compat.hpp"
#include "ui/Designer.hpp"

using namespace cppui::designer;

static void TestAllNodeTypesCreationAndNaming() {
    compat::println("--- Running TestAllNodeTypesCreationAndNaming ---");

    const NodeType allTypes[] = {
        NodeType::Canvas, NodeType::StackPanel, NodeType::VirtualizingStackPanel,
        NodeType::Grid, NodeType::Border, NodeType::ContentControl, NodeType::ScrollViewer, NodeType::Popup,
        NodeType::TreeView, NodeType::ContextMenu, NodeType::MenuItem, NodeType::Button, NodeType::CheckBox,
        NodeType::ComboBox, NodeType::DataGrid, NodeType::Label, NodeType::ListBox, NodeType::PictureBox,
        NodeType::ProgressBar, NodeType::RadioButton, NodeType::Separator, NodeType::Slider, NodeType::TextBlock,
        NodeType::TextBox
    };

    for (NodeType t : allTypes) {
        WidgetNode node(t, "testNode");
        assert(node.GetType() == t);
        assert(node.GetName() == "testNode");
        assert(!node.GetUuid().empty());
        assert(node.GetChildCount() == 0);
        assert(node.GetParent() == nullptr);
    }
}

static void TestContainerCardinalityLeaf() {
    compat::println("--- Running TestContainerCardinalityLeaf ---");

    WidgetNode button(NodeType::Button, "btn");
    assert(button.GetCategory() == ContainerCategory::Leaf);

    auto child = std::make_unique<WidgetNode>(NodeType::TextBlock, "txt");
    auto res = button.AddChild(std::move(child));
    assert(!res.has_value());
    assert(res.error().code == DesignerErrorCode::InvalidChildType);
    assert(button.GetChildCount() == 0);
}

static void TestContainerCardinalitySingleChild() {
    compat::println("--- Running TestContainerCardinalitySingleChild ---");

    WidgetNode border(NodeType::Border, "border1");
    assert(border.GetCategory() == ContainerCategory::SingleChild);

    auto child1 = std::make_unique<WidgetNode>(NodeType::Button, "btn1");
    auto res1 = border.AddChild(std::move(child1));
    assert(res1.has_value());
    assert(border.GetChildCount() == 1);

    auto child2 = std::make_unique<WidgetNode>(NodeType::Button, "btn2");
    auto res2 = border.AddChild(std::move(child2));
    assert(!res2.has_value());
    assert(res2.error().code == DesignerErrorCode::ContainerFull);
    assert(border.GetChildCount() == 1);
}

static void TestContainerCardinalityHierarchical() {
    compat::println("--- Running TestContainerCardinalityHierarchical ---");

    WidgetNode menu(NodeType::ContextMenu, "ctxMenu");
    assert(menu.GetCategory() == ContainerCategory::Hierarchical);

    // ContextMenu 拒絕 Button 作為子節點
    auto invalidChild = std::make_unique<WidgetNode>(NodeType::Button, "btn");
    auto resInvalid = menu.AddChild(std::move(invalidChild));
    assert(!resInvalid.has_value());
    assert(resInvalid.error().code == DesignerErrorCode::InvalidChildType);

    // ContextMenu 接受 MenuItem 與 Separator
    auto item = std::make_unique<WidgetNode>(NodeType::MenuItem, "item1");
    auto sep = std::make_unique<WidgetNode>(NodeType::Separator, "sep1");
    assert(menu.AddChild(std::move(item)).has_value());
    assert(menu.AddChild(std::move(sep)).has_value());
    assert(menu.GetChildCount() == 2);
}

static void TestReparentCycleDetection() {
    compat::println("--- Running TestReparentCycleDetection ---");

    // 構建階層樹: grid -> stack -> border -> button
    auto grid = std::make_unique<WidgetNode>(NodeType::Grid, "grid");
    auto stack = std::make_unique<WidgetNode>(NodeType::StackPanel, "stack");
    auto border = std::make_unique<WidgetNode>(NodeType::Border, "border");
    auto button = std::make_unique<WidgetNode>(NodeType::Button, "button");

    WidgetNode* stackPtr = stack.get();
    WidgetNode* borderPtr = border.get();
    WidgetNode* btnPtr = button.get();

    assert(border->AddChild(std::move(button)).has_value());
    assert(stack->AddChild(std::move(border)).has_value());
    assert(grid->AddChild(std::move(stack)).has_value());

    // 1. 嘗試將祖先 grid 搬移至其子孫 border 中，應被攔截
    auto cycle1 = CardinalityValidator::CheckCycle(*borderPtr, *grid);
    assert(!cycle1.has_value());
    assert(cycle1.error().code == DesignerErrorCode::CyclicHierarchy);

    // 2. 嘗試將祖先 stack 搬移至 button 中，應被攔截
    auto cycle2 = CardinalityValidator::CheckCycle(*btnPtr, *stackPtr);
    assert(!cycle2.has_value());
    assert(cycle2.error().code == DesignerErrorCode::CyclicHierarchy);

    // 3. 自身不能掛入自身
    auto cycleSelf = CardinalityValidator::CheckCycle(*grid, *grid);
    assert(!cycleSelf.has_value());
    assert(cycleSelf.error().code == DesignerErrorCode::CyclicHierarchy);

    // 4. 合法搬移 (例如將 button 搬移至 grid 直屬)
    auto validMove = CardinalityValidator::CheckCycle(*grid, *btnPtr);
    assert(validMove.has_value());
}

static void TestIdentifierValidator() {
    compat::println("--- Running TestIdentifierValidator ---");

    // 1. 合法名稱
    assert(IdentifierValidator::ValidateIdentifier("submitButton").has_value());
    assert(IdentifierValidator::ValidateIdentifier("btn_1").has_value());
    assert(IdentifierValidator::ValidateIdentifier("_privateLabel").has_value());

    // 2. 非法語法
    assert(!IdentifierValidator::ValidateIdentifier("").has_value());
    assert(!IdentifierValidator::ValidateIdentifier("1button").has_value());
    assert(!IdentifierValidator::ValidateIdentifier("my-button").has_value());
    assert(!IdentifierValidator::ValidateIdentifier("my button").has_value());

    // 3. C++ 關鍵字衝突
    auto kwRes = IdentifierValidator::ValidateIdentifier("class");
    assert(!kwRes.has_value());
    assert(kwRes.error().code == DesignerErrorCode::ReservedKeywordCollision);

    auto intRes = IdentifierValidator::ValidateIdentifier("int");
    assert(!intRes.has_value());
    assert(intRes.error().code == DesignerErrorCode::ReservedKeywordCollision);

    // 4. CppUI 框架成員衝突
    auto memRes = IdentifierValidator::ValidateIdentifier("SetTitle");
    assert(!memRes.has_value());
    assert(memRes.error().code == DesignerErrorCode::FrameworkMemberCollision);

    // 5. 文件唯一性與自動編號消歧義
    FormDocumentNode doc;
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "grid1");
    auto btn1 = std::make_unique<WidgetNode>(NodeType::Button, "button1");
    assert(rootGrid->AddChild(std::move(btn1)).has_value());
    doc.SetRoot(std::move(rootGrid));

    assert(!IdentifierValidator::IsUniqueInDocument(doc, "button1"));
    assert(IdentifierValidator::IsUniqueInDocument(doc, "button2"));

    std::string genName = IdentifierValidator::GenerateUniqueName(doc, NodeType::Button);
    assert(genName == "button2");
}

int main() {
    compat::println("==================================================");
    compat::println("Starting designer_ast_model_test...");
    compat::println("==================================================");

    TestAllNodeTypesCreationAndNaming();
    TestContainerCardinalityLeaf();
    TestContainerCardinalitySingleChild();
    TestContainerCardinalityHierarchical();
    TestReparentCycleDetection();
    TestIdentifierValidator();

    compat::println("All designer_ast_model_test passed successfully!");
    return 0;
}
