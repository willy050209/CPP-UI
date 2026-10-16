/**
 * @file PropertyCommands.cpp
 * @brief PropertyCommands 實作 (含屬性變更通知與連續操作 Coalescing 合併)
 */

#include "ui/designer/commands/PropertyCommands.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/metadata/AttachedProperties.hpp"

namespace cppui {
namespace designer {

// ============================================================================
// SetPropertyCommand
// ============================================================================

SetPropertyCommand::SetPropertyCommand(std::string nodeUuid, std::string propertyName, PropertyValue newValue)
    : m_nodeUuid(std::move(nodeUuid))
    , m_propertyName(std::move(propertyName))
    , m_newValue(std::move(newValue))
    , m_timestamp(std::chrono::steady_clock::now()) {}

SetPropertyCommand::~SetPropertyCommand() = default;

compat::expected<void, DesignerError> SetPropertyCommand::Execute(FormDocumentNode& doc) {
    WidgetNode* node = doc.FindByUuid(m_nodeUuid);
    if (!node) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "SetPropertyCommand: node not found: " + m_nodeUuid;
        return compat::unexpected(std::move(err));
    }

    auto existing = node->GetProperty(m_propertyName);
    if (existing.has_value()) {
        m_oldValue = *existing;
    } else {
        m_oldValue = PropertyValue{};
    }

    node->SetProperty(m_propertyName, m_newValue);
    m_timestamp = std::chrono::steady_clock::now();
    doc.PropertyChangedSignal().Notify({ m_nodeUuid, m_propertyName, m_oldValue, m_newValue });
    return {};
}

compat::expected<void, DesignerError> SetPropertyCommand::Redo(FormDocumentNode& doc) {
    WidgetNode* node = doc.FindByUuid(m_nodeUuid);
    if (!node) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "SetPropertyCommand::Redo: node not found: " + m_nodeUuid;
        return compat::unexpected(std::move(err));
    }

    node->SetProperty(m_propertyName, m_newValue);
    doc.PropertyChangedSignal().Notify({ m_nodeUuid, m_propertyName, m_oldValue, m_newValue });
    return {};
}

compat::expected<void, DesignerError> SetPropertyCommand::Undo(FormDocumentNode& doc) {
    WidgetNode* node = doc.FindByUuid(m_nodeUuid);
    if (!node) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "SetPropertyCommand::Undo: node not found: " + m_nodeUuid;
        return compat::unexpected(std::move(err));
    }

    if (m_oldValue.IsEmpty()) {
        node->RemoveProperty(m_propertyName);
    } else {
        node->SetProperty(m_propertyName, m_oldValue);
    }

    doc.PropertyChangedSignal().Notify({ m_nodeUuid, m_propertyName, m_newValue, m_oldValue });
    return {};
}

bool SetPropertyCommand::CanMergeWith(const IUndoableCommand& other) const {
    const auto* otherCmd = dynamic_cast<const SetPropertyCommand*>(&other);
    if (!otherCmd) {
        return false;
    }
    if (otherCmd->m_nodeUuid != m_nodeUuid || otherCmd->m_propertyName != m_propertyName) {
        return false;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(otherCmd->m_timestamp - m_timestamp).count();
    return elapsed >= 0 && elapsed <= 300;
}

void SetPropertyCommand::MergeWith(const IUndoableCommand& other) {
    const auto& otherCmd = dynamic_cast<const SetPropertyCommand&>(other);
    m_newValue = otherCmd.m_newValue;
    m_timestamp = otherCmd.m_timestamp;
}

std::string SetPropertyCommand::GetDescription() const {
    return "修改屬性 " + m_propertyName;
}

// ============================================================================
// SetAttachedPropertyCommand
// ============================================================================

SetAttachedPropertyCommand::SetAttachedPropertyCommand(std::string nodeUuid, std::string propertyName, PropertyValue newValue)
    : m_nodeUuid(std::move(nodeUuid))
    , m_propertyName(std::move(propertyName))
    , m_newValue(std::move(newValue)) {}

SetAttachedPropertyCommand::~SetAttachedPropertyCommand() = default;

compat::expected<void, DesignerError> SetAttachedPropertyCommand::Execute(FormDocumentNode& doc) {
    auto valid = AttachedProperties::Validate(m_propertyName, m_newValue);
    if (!valid.has_value()) {
        return valid;
    }

    WidgetNode* node = doc.FindByUuid(m_nodeUuid);
    if (!node) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "SetAttachedPropertyCommand: node not found: " + m_nodeUuid;
        return compat::unexpected(std::move(err));
    }

    auto existing = node->GetAttachedProperty(m_propertyName);
    if (existing.has_value()) {
        m_oldValue = *existing;
    } else {
        m_oldValue = PropertyValue{};
    }

    node->SetAttachedProperty(m_propertyName, m_newValue);
    doc.PropertyChangedSignal().Notify({ m_nodeUuid, m_propertyName, m_oldValue, m_newValue });
    return {};
}

compat::expected<void, DesignerError> SetAttachedPropertyCommand::Redo(FormDocumentNode& doc) {
    WidgetNode* node = doc.FindByUuid(m_nodeUuid);
    if (!node) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "SetAttachedPropertyCommand::Redo: node not found: " + m_nodeUuid;
        return compat::unexpected(std::move(err));
    }

    node->SetAttachedProperty(m_propertyName, m_newValue);
    doc.PropertyChangedSignal().Notify({ m_nodeUuid, m_propertyName, m_oldValue, m_newValue });
    return {};
}

compat::expected<void, DesignerError> SetAttachedPropertyCommand::Undo(FormDocumentNode& doc) {
    WidgetNode* node = doc.FindByUuid(m_nodeUuid);
    if (!node) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "SetAttachedPropertyCommand::Undo: node not found: " + m_nodeUuid;
        return compat::unexpected(std::move(err));
    }

    if (m_oldValue.IsEmpty()) {
        node->RemoveAttachedProperty(m_propertyName);
    } else {
        node->SetAttachedProperty(m_propertyName, m_oldValue);
    }

    doc.PropertyChangedSignal().Notify({ m_nodeUuid, m_propertyName, m_newValue, m_oldValue });
    return {};
}

std::string SetAttachedPropertyCommand::GetDescription() const {
    return "修改附加屬性 " + m_propertyName;
}

} // namespace designer
} // namespace cppui
