/**
 * @file FormSerializer.hpp
 * @brief 表單文件無損 JSON 序列化與容錯反序列化公開介面
 */

#pragma once

#include <memory>
#include <string>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"

namespace cppui {
namespace designer {

class FormDocumentNode;

/// <summary>
/// 表單文件無損序列化與反序列化器 (FormSerializer)
/// 遵循 .cppui.json 確定性鍵值排序、預設值剔除與結構化錯誤回報規範。
/// </summary>
class CPPUI_API FormSerializer {
public:
    /// <summary>
    /// 將表單文件 AST 序列化為 Git 友善的確定性 JSON 字串。
    /// 自動執行預設值剔除 (Default Value Elision)，大幅精簡輸出檔案。
    /// </summary>
    /// <param name="doc">表單文件根節點。</param>
    /// <returns>序列化成功之 JSON 字串，或結構化錯誤資訊。</returns>
    COMPAT_NODISCARD static compat::expected<std::string, DesignerError> Serialize(
        const FormDocumentNode& doc
    );

    /// <summary>
    /// 從 JSON 字串反序列化重建完整的 FormDocumentNode 階層樹。
    /// 具備嚴格容錯機制，輸入損毀時安全回傳帶有 JSON Pointer 的錯誤代碼，絕不崩潰。
    /// </summary>
    /// <param name="jsonText">.cppui.json 內容文字。</param>
    /// <returns>反序列化建立之 FormDocumentNode 指標，或結構化錯誤資訊。</returns>
    COMPAT_NODISCARD static compat::expected<std::unique_ptr<FormDocumentNode>, DesignerError> Deserialize(
        compat::string_view jsonText
    );
};

} // namespace designer
} // namespace cppui
