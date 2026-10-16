/**
 * @file MacroCommand.cpp
 * @brief MacroCommand 實作 (交易式保證原子批次執行與復原)
 */

#include "ui/designer/commands/MacroCommand.hpp"

namespace cppui {
namespace designer {

MacroCommand::MacroCommand(std::string description)
    : m_description(std::move(description)) {}

MacroCommand::~MacroCommand() = default;

void MacroCommand::AddCommand(std::unique_ptr<IUndoableCommand> command) {
    if (command) {
        m_commands.push_back(std::move(command));
    }
}

compat::expected<void, DesignerError> MacroCommand::Execute(FormDocumentNode& doc) {
    size_t executedCount = 0;
    for (size_t i = 0; i < m_commands.size(); ++i) {
        auto res = m_commands[i]->Execute(doc);
        if (!res.has_value()) {
            // 交易失敗，倒序還原已執行的操作
            for (size_t j = executedCount; j > 0; --j) {
                (void)m_commands[j - 1]->Undo(doc);
            }
            return res;
        }
        executedCount++;
    }
    return {};
}

compat::expected<void, DesignerError> MacroCommand::Redo(FormDocumentNode& doc) {
    size_t executedCount = 0;
    for (size_t i = 0; i < m_commands.size(); ++i) {
        auto res = m_commands[i]->Redo(doc);
        if (!res.has_value()) {
            for (size_t j = executedCount; j > 0; --j) {
                (void)m_commands[j - 1]->Undo(doc);
            }
            return res;
        }
        executedCount++;
    }
    return {};
}

compat::expected<void, DesignerError> MacroCommand::Undo(FormDocumentNode& doc) {
    for (size_t i = m_commands.size(); i > 0; --i) {
        auto res = m_commands[i - 1]->Undo(doc);
        if (!res.has_value()) {
            return res;
        }
    }
    return {};
}

std::string MacroCommand::GetDescription() const {
    if (!m_description.empty()) {
        return m_description;
    }
    return "複合操作 (" + std::to_string(m_commands.size()) + " 項)";
}

} // namespace designer
} // namespace cppui
