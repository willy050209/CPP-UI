/**
 * @file IUndoableCommand.hpp
 * @brief 撤銷/重做可逆命令介面 (IUndoableCommand)
 */

#pragma once

#include <string>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"

namespace cppui {
namespace designer {

class FormDocumentNode;

/// <summary>
/// 可撤銷/重做抽象命令介面 (IUndoableCommand)
/// 實作命令模式 (Command Pattern)，保證所有對 AST 的修改皆可無損撤銷與還原。
/// </summary>
class CPPUI_API IUndoableCommand {
public:
    virtual ~IUndoableCommand() = default;

    /// <summary>首次執行命令。</summary>
    COMPAT_NODISCARD virtual compat::expected<void, DesignerError> Execute(FormDocumentNode& doc) = 0;

    /// <summary>撤銷命令，將 AST 還原至操作前狀態。</summary>
    COMPAT_NODISCARD virtual compat::expected<void, DesignerError> Undo(FormDocumentNode& doc) = 0;

    /// <summary>重做命令，將 AST 套用修改。</summary>
    COMPAT_NODISCARD virtual compat::expected<void, DesignerError> Redo(FormDocumentNode& doc) = 0;

    /// <summary>判斷是否可與下一個連續命令進行合併 (Coalescing)。</summary>
    COMPAT_NODISCARD virtual bool CanMergeWith(const IUndoableCommand& other) const {
        (void)other;
        return false;
    }

    /// <summary>合併後續命令內容。</summary>
    virtual void MergeWith(const IUndoableCommand& other) {
        (void)other;
    }

    /// <summary>取得命令可讀描述 (供 UI 顯示「復原: 修改按鈕大小」等)。</summary>
    COMPAT_NODISCARD virtual std::string GetDescription() const = 0;
};

} // namespace designer
} // namespace cppui
