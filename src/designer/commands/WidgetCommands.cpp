/**
 * @file WidgetCommands.cpp
 * @brief WidgetCommands 拓撲增刪移命令實作
 */

#include "ui/designer/commands/WidgetCommands.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/ast/CardinalityValidator.hpp"

namespace cppui {
namespace designer {

// ============================================================================
// AddWidgetCommand
// ============================================================================

AddWidgetCommand::AddWidgetCommand(std::string parentUuid, std::unique_ptr<WidgetNode> node, size_t index)
    : m_parentUuid(std::move(parentUuid))
    , m_node(std::move(node))
    , m_targetIndex(index) {
    if (m_node) {
        m_childUuid = m_node->GetUuid();
    }
}

AddWidgetCommand::~AddWidgetCommand() = default;

compat::expected<void, DesignerError> AddWidgetCommand::Execute(FormDocumentNode& doc) {
    return Redo(doc);
}

compat::expected<void, DesignerError> AddWidgetCommand::Redo(FormDocumentNode& doc) {
    if (!m_node) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidNode;
        err.message = "AddWidgetCommand: node is null or already in tree";
        return compat::unexpected(std::move(err));
    }

    WidgetNode* parent = doc.FindByUuid(m_parentUuid);
    if (!parent) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "AddWidgetCommand: parent node not found: " + m_parentUuid;
        return compat::unexpected(std::move(err));
    }

    auto canAdd = CardinalityValidator::CanAddChild(*parent, m_node->GetType());
    if (!canAdd.has_value()) {
        return canAdd;
    }

    m_childUuid = m_node->GetUuid();
    if (m_targetIndex == static_cast<size_t>(-1) || m_targetIndex >= parent->GetChildCount()) {
        m_actualIndex = parent->GetChildCount();
        auto res = parent->AddChild(std::move(m_node));
        if (!res.has_value()) {
            return res;
        }
    } else {
        m_actualIndex = m_targetIndex;
        auto res = parent->InsertChild(m_actualIndex, std::move(m_node));
        if (!res.has_value()) {
            return res;
        }
    }

    doc.NodeAddedSignal().Notify({ m_parentUuid, m_childUuid, m_actualIndex });
    return {};
}

compat::expected<void, DesignerError> AddWidgetCommand::Undo(FormDocumentNode& doc) {
    WidgetNode* parent = doc.FindByUuid(m_parentUuid);
    if (!parent) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "AddWidgetCommand::Undo: parent node not found: " + m_parentUuid;
        return compat::unexpected(std::move(err));
    }

    auto removedRes = parent->RemoveChildByUuid(m_childUuid);
    if (!removedRes.has_value()) {
        return compat::unexpected(removedRes.error());
    }

    m_node = std::move(removedRes.value());
    doc.NodeRemovedSignal().Notify({ m_parentUuid, m_childUuid });
    return {};
}

std::string AddWidgetCommand::GetDescription() const {
    return "新增控制項";
}

// ============================================================================
// RemoveWidgetCommand
// ============================================================================

RemoveWidgetCommand::RemoveWidgetCommand(std::string parentUuid, std::string childUuid)
    : m_parentUuid(std::move(parentUuid))
    , m_childUuid(std::move(childUuid)) {}

RemoveWidgetCommand::~RemoveWidgetCommand() = default;

compat::expected<void, DesignerError> RemoveWidgetCommand::Execute(FormDocumentNode& doc) {
    return Redo(doc);
}

compat::expected<void, DesignerError> RemoveWidgetCommand::Redo(FormDocumentNode& doc) {
    WidgetNode* parent = doc.FindByUuid(m_parentUuid);
    if (!parent) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "RemoveWidgetCommand: parent not found: " + m_parentUuid;
        return compat::unexpected(std::move(err));
    }

    WidgetNode* child = parent->FindByUuid(m_childUuid);
    if (!child) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "RemoveWidgetCommand: child not found: " + m_childUuid;
        return compat::unexpected(std::move(err));
    }

    auto idxOpt = parent->IndexOfChild(child);
    if (!idxOpt.has_value()) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "RemoveWidgetCommand: child is not a direct child of parent";
        return compat::unexpected(std::move(err));
    }
    m_index = idxOpt.value();

    auto removed = parent->RemoveChild(m_index);
    if (!removed.has_value()) {
        return compat::unexpected(removed.error());
    }

    m_removedSubtree = std::move(removed.value());
    doc.NodeRemovedSignal().Notify({ m_parentUuid, m_childUuid });
    return {};
}

compat::expected<void, DesignerError> RemoveWidgetCommand::Undo(FormDocumentNode& doc) {
    if (!m_removedSubtree) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidNode;
        err.message = "RemoveWidgetCommand::Undo: removed subtree is null";
        return compat::unexpected(std::move(err));
    }

    WidgetNode* parent = doc.FindByUuid(m_parentUuid);
    if (!parent) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "RemoveWidgetCommand::Undo: parent not found: " + m_parentUuid;
        return compat::unexpected(std::move(err));
    }

    size_t insertIdx = m_index;
    if (insertIdx > parent->GetChildCount()) {
        insertIdx = parent->GetChildCount();
    }

    auto res = parent->InsertChild(insertIdx, std::move(m_removedSubtree));
    if (!res.has_value()) {
        return res;
    }

    doc.NodeAddedSignal().Notify({ m_parentUuid, m_childUuid, insertIdx });
    return {};
}

std::string RemoveWidgetCommand::GetDescription() const {
    return "移除控制項";
}

// ============================================================================
// MoveWidgetCommand
// ============================================================================

MoveWidgetCommand::MoveWidgetCommand(std::string nodeUuid, std::string newParentUuid, size_t newIndex)
    : m_nodeUuid(std::move(nodeUuid))
    , m_newParentUuid(std::move(newParentUuid))
    , m_newIndex(newIndex) {}

MoveWidgetCommand::~MoveWidgetCommand() = default;

compat::expected<void, DesignerError> MoveWidgetCommand::Execute(FormDocumentNode& doc) {
    WidgetNode* node = doc.FindByUuid(m_nodeUuid);
    if (!node || !node->GetParent()) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "MoveWidgetCommand: node or its parent not found";
        return compat::unexpected(std::move(err));
    }
    m_oldParentUuid = node->GetParent()->GetUuid();
    auto oldIdxOpt = node->GetParent()->IndexOfChild(node);
    if (!oldIdxOpt.has_value()) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "MoveWidgetCommand: child not found in its parent";
        return compat::unexpected(std::move(err));
    }
    m_oldIndex = oldIdxOpt.value();
    return Redo(doc);
}

compat::expected<void, DesignerError> MoveWidgetCommand::Redo(FormDocumentNode& doc) {
    WidgetNode* oldParent = doc.FindByUuid(m_oldParentUuid);
    WidgetNode* newParent = doc.FindByUuid(m_newParentUuid);
    if (!oldParent || !newParent) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "MoveWidgetCommand: old or new parent not found";
        return compat::unexpected(std::move(err));
    }

    WidgetNode* node = oldParent->FindByUuid(m_nodeUuid);
    if (!node) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "MoveWidgetCommand: node to move not found";
        return compat::unexpected(std::move(err));
    }

    auto cycleCheck = CardinalityValidator::CheckCycle(*newParent, *node);
    if (!cycleCheck.has_value()) {
        return cycleCheck;
    }

    auto canAdd = CardinalityValidator::CanAddChild(*newParent, node->GetType());
    if (!canAdd.has_value()) {
        return canAdd;
    }

    auto removedRes = oldParent->RemoveChildByUuid(m_nodeUuid);
    if (!removedRes.has_value()) {
        return compat::unexpected(removedRes.error());
    }

    size_t targetIdx = m_newIndex;
    if (targetIdx > newParent->GetChildCount()) {
        targetIdx = newParent->GetChildCount();
    }

    auto insertRes = newParent->InsertChild(targetIdx, std::move(removedRes.value()));
    if (!insertRes.has_value()) {
        return insertRes;
    }

    doc.NodeMovedSignal().Notify({ m_nodeUuid, m_oldParentUuid, m_newParentUuid, targetIdx });
    return {};
}

compat::expected<void, DesignerError> MoveWidgetCommand::Undo(FormDocumentNode& doc) {
    WidgetNode* newParent = doc.FindByUuid(m_newParentUuid);
    WidgetNode* oldParent = doc.FindByUuid(m_oldParentUuid);
    if (!newParent || !oldParent) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "MoveWidgetCommand::Undo: parents not found";
        return compat::unexpected(std::move(err));
    }

    auto removedRes = newParent->RemoveChildByUuid(m_nodeUuid);
    if (!removedRes.has_value()) {
        return compat::unexpected(removedRes.error());
    }

    size_t targetIdx = m_oldIndex;
    if (targetIdx > oldParent->GetChildCount()) {
        targetIdx = oldParent->GetChildCount();
    }

    auto insertRes = oldParent->InsertChild(targetIdx, std::move(removedRes.value()));
    if (!insertRes.has_value()) {
        return insertRes;
    }

    doc.NodeMovedSignal().Notify({ m_nodeUuid, m_newParentUuid, m_oldParentUuid, targetIdx });
    return {};
}

std::string MoveWidgetCommand::GetDescription() const {
    return "移動控制項";
}

} // namespace designer
} // namespace cppui
