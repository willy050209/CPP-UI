/**
 * @file CommandHistoryManager.hpp
 * @brief Undo/Redo 命令歷史管理員、存檔點 (SavePoint) 與 IsDirty 狀態機
 */

#pragma once

#include <vector>
#include <memory>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/commands/IUndoableCommand.hpp"

namespace cppui {
namespace designer {

class FormDocumentNode;

/// <summary>
/// 命令歷史堆疊管理員 (CommandHistoryManager)
/// 管理 Undo/Redo 狀態機，支援命令連續合併 (Coalescing)、最大堆疊限制與 SavePoint 髒狀態追蹤。
/// </summary>
class CPPUI_API CommandHistoryManager {
public:
    explicit CommandHistoryManager(size_t maxHistorySize = 200);
    ~CommandHistoryManager();

    /// <summary>執行新命令並推入歷史堆疊 (自動清除已作廢之 Redo 分支)。</summary>
    compat::expected<void, DesignerError> Execute(FormDocumentNode& doc, std::unique_ptr<IUndoableCommand> command);

    /// <summary>撤銷上一步操作。</summary>
    compat::expected<void, DesignerError> Undo(FormDocumentNode& doc);

    /// <summary>重做已撤銷之操作。</summary>
    compat::expected<void, DesignerError> Redo(FormDocumentNode& doc);

    /// <summary>是否可執行 Undo。</summary>
    COMPAT_NODISCARD bool CanUndo() const noexcept;

    /// <summary>是否可執行 Redo。</summary>
    COMPAT_NODISCARD bool CanRedo() const noexcept;

    /// <summary>取得目前 Undo 堆疊長度。</summary>
    COMPAT_NODISCARD size_t GetUndoCount() const noexcept;

    /// <summary>取得目前 Redo 堆疊長度。</summary>
    COMPAT_NODISCARD size_t GetRedoCount() const noexcept;

    /// <summary>設定目前命令指標為存檔點 (Save Point)。</summary>
    void SetSavePoint() noexcept;

    /// <summary>判斷自上次存檔以來文件是否已被修改 (IsDirty)。</summary>
    COMPAT_NODISCARD bool IsDirty() const noexcept;

    /// <summary>清空所有歷史堆疊並重置存檔點。</summary>
    void Clear() noexcept;

    /// <summary>取得即將 Undo 的命令描述。</summary>
    COMPAT_NODISCARD std::string GetUndoDescription() const;

    /// <summary>取得即將 Redo 的命令描述。</summary>
    COMPAT_NODISCARD std::string GetRedoDescription() const;

private:
    size_t m_maxHistorySize;
    std::vector<std::unique_ptr<IUndoableCommand>> m_history;
    size_t m_currentIndex = 0;
    size_t m_savePointIndex = 0;
};

} // namespace designer
} // namespace cppui
