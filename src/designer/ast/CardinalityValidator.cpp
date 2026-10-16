/**
 * @file CardinalityValidator.cpp
 * @brief CardinalityValidator 容器約束與循環引用驗證實作
 */

#include "ui/designer/ast/CardinalityValidator.hpp"
#include "ui/designer/ast/WidgetNode.hpp"

namespace cppui {
namespace designer {

compat::expected<void, DesignerError> CardinalityValidator::CanAddChild(
    const WidgetNode& parent,
    NodeType childType
) noexcept {
    const ContainerCategory category = parent.GetCategory();

    // 1. 葉節點禁止包含任何子節點
    if (category == ContainerCategory::Leaf) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidChildType;
        err.message = "Leaf controls cannot host any child elements";
        return compat::unexpected(std::move(err));
    }

    // 2. 單一子節點容器最多容納 1 個子節點
    if (category == ContainerCategory::SingleChild) {
        if (parent.GetChildCount() >= 1) {
            DesignerError err;
            err.code = DesignerErrorCode::ContainerFull;
            err.message = "SingleChild container already contains a child element";
            return compat::unexpected(std::move(err));
        }
    }

    // 3. 階層式容器之特殊約束 (例如 ContextMenu 與 MenuItem 僅允許包含 MenuItem 或 Separator)
    if (parent.GetType() == NodeType::ContextMenu || parent.GetType() == NodeType::MenuItem) {
        if (childType != NodeType::MenuItem && childType != NodeType::Separator) {
            DesignerError err;
            err.code = DesignerErrorCode::InvalidChildType;
            err.message = "ContextMenu and MenuItem only accept MenuItem or Separator as children";
            return compat::unexpected(std::move(err));
        }
    }

    return {};
}

compat::expected<void, DesignerError> CardinalityValidator::CheckCycle(
    const WidgetNode& targetParent,
    const WidgetNode& nodeToMove
) noexcept {
    // 自身不能成為自身的子節點
    if (&targetParent == &nodeToMove) {
        DesignerError err;
        err.code = DesignerErrorCode::CyclicHierarchy;
        err.message = "Cannot reparent node to itself";
        return compat::unexpected(std::move(err));
    }

    // 向上追溯 targetParent 的祖先節點鏈
    const WidgetNode* curr = &targetParent;
    while (curr != nullptr) {
        if (curr == &nodeToMove) {
            DesignerError err;
            err.code = DesignerErrorCode::CyclicHierarchy;
            err.message = "Cyclic hierarchy detected: target parent is a descendant of the node being moved";
            return compat::unexpected(std::move(err));
        }
        curr = curr->GetParent();
    }

    return {};
}

} // namespace designer
} // namespace cppui
