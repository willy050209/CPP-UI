/**
 * @file IdentifierValidator.hpp
 * @brief C++ 成員變數名稱語法合法性、關鍵字衝突與全域消歧義檢驗器
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
/// C++ 變數識別字語法與碰撞檢驗器 (IdentifierValidator)
/// </summary>
class CPPUI_API IdentifierValidator {
public:
    /// <summary>
    /// 檢查變數名稱是否符合合法 C++ 識別字語法，且未與關鍵字或框架保留字衝突。
    /// </summary>
    /// <param name="name">待驗證名稱。</param>
    /// <returns>若合法回傳 void，否則回傳錯誤代碼。</returns>
    COMPAT_NODISCARD static compat::expected<void, DesignerError> ValidateIdentifier(
        compat::string_view name
    ) noexcept;

    /// <summary>
    /// 判斷名稱是否在目標表單文件中具備全域唯一性。
    /// </summary>
    /// <param name="doc">表單文件根節點。</param>
    /// <param name="name">變數名稱。</param>
    /// <param name="excludeUuid">欲排除比對之節點 UUID (用於節點自身重新命名)。</param>
    /// <returns>若全域唯一傳回 true。</returns>
    COMPAT_NODISCARD static bool IsUniqueInDocument(
        const FormDocumentNode& doc,
        compat::string_view name,
        compat::string_view excludeUuid = ""
    ) noexcept;

    /// <summary>
    /// 依節點型別自動遞增生成全域唯一名稱 (例如 button1, button2, grid1)。
    /// </summary>
    /// <param name="doc">表單文件根節點。</param>
    /// <param name="type">節點型別。</param>
    /// <returns>全域唯一變數名稱。</returns>
    COMPAT_NODISCARD static std::string GenerateUniqueName(
        const FormDocumentNode& doc,
        NodeType type
    );
};

} // namespace designer
} // namespace cppui
