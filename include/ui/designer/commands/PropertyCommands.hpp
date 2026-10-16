/**
 * @file PropertyCommands.hpp
 * @brief 屬性設定與附加屬性可撤銷命令 (含時間戳記與連續操作 Coalescing 合併)
 */

#pragma once

#include <string>
#include <chrono>
#include "ui/designer/commands/IUndoableCommand.hpp"
#include "ui/designer/metadata/PropertyValue.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 設定控制項自身屬性命令 (SetPropertyCommand)
/// 內建連續操作合併機制 (Coalescing)，短時間內連續微調同一屬性自動合併為單一 Undo 步驟。
/// </summary>
class CPPUI_API SetPropertyCommand : public IUndoableCommand {
public:
    SetPropertyCommand(std::string nodeUuid, std::string propertyName, PropertyValue newValue);
    ~SetPropertyCommand() override;

    compat::expected<void, DesignerError> Execute(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Undo(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Redo(FormDocumentNode& doc) override;

    bool CanMergeWith(const IUndoableCommand& other) const override;
    void MergeWith(const IUndoableCommand& other) override;

    std::string GetDescription() const override;

    const std::string& GetNodeUuid() const noexcept { return m_nodeUuid; }
    const std::string& GetPropertyName() const noexcept { return m_propertyName; }
    const PropertyValue& GetNewValue() const noexcept { return m_newValue; }

private:
    std::string m_nodeUuid;
    std::string m_propertyName;
    PropertyValue m_newValue;
    PropertyValue m_oldValue;
    std::chrono::steady_clock::time_point m_timestamp;
};

/// <summary>
/// 設定控制項附加屬性命令 (SetAttachedPropertyCommand)
/// </summary>
class CPPUI_API SetAttachedPropertyCommand : public IUndoableCommand {
public:
    SetAttachedPropertyCommand(std::string nodeUuid, std::string propertyName, PropertyValue newValue);
    ~SetAttachedPropertyCommand() override;

    compat::expected<void, DesignerError> Execute(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Undo(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Redo(FormDocumentNode& doc) override;

    std::string GetDescription() const override;

private:
    std::string m_nodeUuid;
    std::string m_propertyName;
    PropertyValue m_newValue;
    PropertyValue m_oldValue;
};

} // namespace designer
} // namespace cppui
