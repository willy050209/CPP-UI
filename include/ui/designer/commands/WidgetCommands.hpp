/**
 * @file WidgetCommands.hpp
 * @brief 控制項樹狀拓撲增刪移可撤銷命令 (Add/Remove/Move)
 */

#pragma once

#include <memory>
#include <string>
#include "ui/designer/commands/IUndoableCommand.hpp"
#include "ui/designer/ast/WidgetNode.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 新增控制項命令 (AddWidgetCommand)
/// </summary>
class CPPUI_API AddWidgetCommand : public IUndoableCommand {
public:
    AddWidgetCommand(std::string parentUuid, std::unique_ptr<WidgetNode> node, size_t index = static_cast<size_t>(-1));
    ~AddWidgetCommand() override;

    compat::expected<void, DesignerError> Execute(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Undo(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Redo(FormDocumentNode& doc) override;
    std::string GetDescription() const override;

private:
    std::string m_parentUuid;
    std::unique_ptr<WidgetNode> m_node;
    size_t m_targetIndex;
    size_t m_actualIndex = 0;
    std::string m_childUuid;
};

/// <summary>
/// 移除控制項命令 (RemoveWidgetCommand)
/// 完整保留被刪除節點及其深層子樹，確保 Undo 時無失真復原。
/// </summary>
class CPPUI_API RemoveWidgetCommand : public IUndoableCommand {
public:
    RemoveWidgetCommand(std::string parentUuid, std::string childUuid);
    ~RemoveWidgetCommand() override;

    compat::expected<void, DesignerError> Execute(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Undo(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Redo(FormDocumentNode& doc) override;
    std::string GetDescription() const override;

private:
    std::string m_parentUuid;
    std::string m_childUuid;
    size_t m_index = 0;
    std::unique_ptr<WidgetNode> m_removedSubtree;
};

/// <summary>
/// 節點階層移動/重新掛載命令 (MoveWidgetCommand)
/// </summary>
class CPPUI_API MoveWidgetCommand : public IUndoableCommand {
public:
    MoveWidgetCommand(std::string nodeUuid, std::string newParentUuid, size_t newIndex);
    ~MoveWidgetCommand() override;

    compat::expected<void, DesignerError> Execute(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Undo(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Redo(FormDocumentNode& doc) override;
    std::string GetDescription() const override;

private:
    std::string m_nodeUuid;
    std::string m_oldParentUuid;
    std::string m_newParentUuid;
    size_t m_oldIndex = 0;
    size_t m_newIndex = 0;
};

} // namespace designer
} // namespace cppui
