/**
 * @file CommandHistoryManager.cpp
 * @brief CommandHistoryManager 狀態機、連續合併與存檔點追蹤實作
 */

#include "ui/designer/commands/CommandHistoryManager.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"

namespace cppui {
namespace designer {

CommandHistoryManager::CommandHistoryManager(size_t maxHistorySize)
    : m_maxHistorySize(maxHistorySize > 0 ? maxHistorySize : 200) {}

CommandHistoryManager::~CommandHistoryManager() = default;

compat::expected<void, DesignerError> CommandHistoryManager::Execute(
    FormDocumentNode& doc,
    std::unique_ptr<IUndoableCommand> command
) {
    if (!command) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidNode;
        err.message = "Command pointer cannot be null";
        return compat::unexpected(std::move(err));
    }

    // 1. 檢查是否可與前一項命令進行 Coalescing 合併
    if (m_currentIndex > 0 && m_history[m_currentIndex - 1]->CanMergeWith(*command)) {
        auto res = command->Execute(doc);
        if (!res.has_value()) {
            return res;
        }
        m_history[m_currentIndex - 1]->MergeWith(*command);
        return {};
    }

    // 2. 若當前指標後方存在已撤銷的 Redo 分支，則將其截斷
    if (m_currentIndex < m_history.size()) {
        if (m_savePointIndex > m_currentIndex && m_savePointIndex != static_cast<size_t>(-1)) {
            m_savePointIndex = static_cast<size_t>(-1); // 存檔點位於被剪除的未來分支中，標記為失效
        }
        m_history.erase(m_history.begin() + m_currentIndex, m_history.end());
    }

    // 3. 執行新命令
    auto res = command->Execute(doc);
    if (!res.has_value()) {
        return res;
    }

    // 4. 推入歷史堆疊
    m_history.push_back(std::move(command));
    m_currentIndex++;

    // 5. 確保未超出最大堆疊長度
    if (m_history.size() > m_maxHistorySize) {
        m_history.erase(m_history.begin());
        m_currentIndex--;
        if (m_savePointIndex != static_cast<size_t>(-1)) {
            if (m_savePointIndex > 0) {
                m_savePointIndex--;
            } else {
                m_savePointIndex = static_cast<size_t>(-1);
            }
        }
    }

    return {};
}

compat::expected<void, DesignerError> CommandHistoryManager::Undo(FormDocumentNode& doc) {
    if (!CanUndo()) {
        DesignerError err;
        err.code = DesignerErrorCode::UndoStackEmpty;
        err.message = "Cannot undo: history stack is empty";
        return compat::unexpected(std::move(err));
    }

    auto res = m_history[m_currentIndex - 1]->Undo(doc);
    if (!res.has_value()) {
        return res;
    }

    m_currentIndex--;
    return {};
}

compat::expected<void, DesignerError> CommandHistoryManager::Redo(FormDocumentNode& doc) {
    if (!CanRedo()) {
        DesignerError err;
        err.code = DesignerErrorCode::RedoStackEmpty;
        err.message = "Cannot redo: no forward commands in history";
        return compat::unexpected(std::move(err));
    }

    auto res = m_history[m_currentIndex]->Redo(doc);
    if (!res.has_value()) {
        return res;
    }

    m_currentIndex++;
    return {};
}

bool CommandHistoryManager::CanUndo() const noexcept {
    return m_currentIndex > 0;
}

bool CommandHistoryManager::CanRedo() const noexcept {
    return m_currentIndex < m_history.size();
}

size_t CommandHistoryManager::GetUndoCount() const noexcept {
    return m_currentIndex;
}

size_t CommandHistoryManager::GetRedoCount() const noexcept {
    return m_history.size() - m_currentIndex;
}

void CommandHistoryManager::SetSavePoint() noexcept {
    m_savePointIndex = m_currentIndex;
}

bool CommandHistoryManager::IsDirty() const noexcept {
    return m_currentIndex != m_savePointIndex;
}

void CommandHistoryManager::Clear() noexcept {
    m_history.clear();
    m_currentIndex = 0;
    m_savePointIndex = 0;
}

std::string CommandHistoryManager::GetUndoDescription() const {
    if (CanUndo()) {
        return m_history[m_currentIndex - 1]->GetDescription();
    }
    return "";
}

std::string CommandHistoryManager::GetRedoDescription() const {
    if (CanRedo()) {
        return m_history[m_currentIndex]->GetDescription();
    }
    return "";
}

} // namespace designer
} // namespace cppui
