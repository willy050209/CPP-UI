/**
 * @file FormClassEmitter.hpp
 * @brief 表單 Partial Class / Code-Behind 代碼發射器 (產出 designer.hpp, .hpp 與 .cpp)
 */

#pragma once

#include <memory>
#include <string>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/codegen/CodegenTypes.hpp"

namespace cppui {
namespace designer {
namespace codegen {

/// <summary>
/// 表單類別發射器 (FormClassEmitter)
/// 負責產出分離式設計器代碼 (ClassName.designer.hpp) 與使用者邏輯樣板 (ClassName.hpp / ClassName.cpp)。
/// </summary>
class CPPUI_API FormClassEmitter {
public:
    FormClassEmitter();
    ~FormClassEmitter();
    FormClassEmitter(FormClassEmitter&& other) noexcept;
    FormClassEmitter& operator=(FormClassEmitter&& other) noexcept;

    FormClassEmitter(const FormClassEmitter&) = delete;
    FormClassEmitter& operator=(const FormClassEmitter&) = delete;

    /// <summary>
    /// 發射 ClassName.designer.hpp (設計器管理標頭檔)。
    /// 包含成員控制項指標宣告、InitializeComponent() 拓撲組裝與虛擬事件處理常式宣告。
    /// </summary>
    /// <param name="document">表單文件。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <param name="outSourceMap">可選的 SourceMap 輸出，用於追蹤行號與符號位置。</param>
    /// <returns>標頭檔內容字串。</returns>
    COMPAT_NODISCARD std::string EmitDesignerHeader(
        const FormDocumentNode& document,
        const CodegenOptions& options = CodegenOptions{},
        SourceMap* outSourceMap = nullptr
    ) const;

    /// <summary>
    /// 發射 ClassName.hpp (使用者自訂類別標頭檔樣板)。
    /// </summary>
    /// <param name="document">表單文件。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <returns>標頭檔內容字串。</returns>
    COMPAT_NODISCARD std::string EmitUserHeader(
        const FormDocumentNode& document,
        const CodegenOptions& options = CodegenOptions{}
    ) const;

    /// <summary>
    /// 發射 ClassName.cpp (使用者實作檔樣板)。
    /// 包含呼叫 InitializeComponent() 的建構子與事件處理常式骨架。
    /// </summary>
    /// <param name="document">表單文件。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <returns>實作檔內容字串。</returns>
    COMPAT_NODISCARD std::string EmitUserSource(
        const FormDocumentNode& document,
        const CodegenOptions& options = CodegenOptions{}
    ) const;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace codegen
} // namespace designer
} // namespace cppui
