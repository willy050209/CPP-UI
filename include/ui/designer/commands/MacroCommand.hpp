/**
 * @file MacroCommand.hpp
 * @brief 複合巨集命令 (MacroCommand) 批次操作交易封裝
 */

#pragma once

#include <vector>
#include <memory>
#include <string>
#include "ui/designer/commands/IUndoableCommand.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 複合巨集命令 (MacroCommand)
/// 將多個不可分割的原子操作打包為單一交易 (Transaction)，支援一鍵還原或重做。
/// </summary>
class CPPUI_API MacroCommand : public IUndoableCommand {
public:
    explicit MacroCommand(std::string description = "");
    ~MacroCommand() override;

    /// <summary>新增子命令至批次交易清單中。</summary>
    void AddCommand(std::unique_ptr<IUndoableCommand> command);

    /// <summary>取得子命令數量。</summary>
    COMPAT_NODISCARD size_t GetCommandCount() const noexcept {
        return m_commands.size();
    }

    compat::expected<void, DesignerError> Execute(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Undo(FormDocumentNode& doc) override;
    compat::expected<void, DesignerError> Redo(FormDocumentNode& doc) override;

    std::string GetDescription() const override;

private:
    std::string m_description;
    std::vector<std::unique_ptr<IUndoableCommand>> m_commands;
};

} // namespace designer
} // namespace cppui
