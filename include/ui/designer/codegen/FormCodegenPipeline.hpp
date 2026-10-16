/**
 * @file FormCodegenPipeline.hpp
 * @brief CPPUI Designer 程式碼生成總體編排管道 (FormCodegenPipeline)
 */

#pragma once

#include <memory>
#include <string>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/AstTypes.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/codegen/CodegenTypes.hpp"

namespace cppui {
namespace designer {
namespace codegen {

/// <summary>
/// 表單代碼生成主管道 (FormCodegenPipeline)
/// 統一協調整合 FluentDslEmitter、FormClassEmitter 與 CMakeSnippetEmitter，
/// 提供一站式 AST 代碼轉換與 SourceMap 追蹤服務。
/// </summary>
class CPPUI_API FormCodegenPipeline {
public:
    FormCodegenPipeline();
    ~FormCodegenPipeline();
    FormCodegenPipeline(FormCodegenPipeline&& other) noexcept;
    FormCodegenPipeline& operator=(FormCodegenPipeline&& other) noexcept;

    FormCodegenPipeline(const FormCodegenPipeline&) = delete;
    FormCodegenPipeline& operator=(const FormCodegenPipeline&) = delete;

    /// <summary>
    /// 依據表單文件 AST 執行完整代碼生成流程。
    /// 產出 designer.hpp, .hpp, .cpp, Fluent 片段, CMake 腳本與雙向 SourceMap。
    /// </summary>
    /// <param name="document">表單文件 AST 根。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <returns>生成結果物件，若失敗則回傳結構化錯誤。</returns>
    COMPAT_NODISCARD compat::expected<CodegenResult, DesignerError> Generate(
        const FormDocumentNode& document,
        const CodegenOptions& options = CodegenOptions{}
    ) const;

    /// <summary>
    /// 針對單一控制項節點 (及其子樹) 生成純粹的 Fluent DSL 代碼片段。
    /// </summary>
    /// <param name="rootNode">目標節點。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <returns>Fluent DSL 代碼字串，若失敗則回傳結構化錯誤。</returns>
    COMPAT_NODISCARD compat::expected<std::string, DesignerError> GenerateFluentSnippet(
        const WidgetNode& rootNode,
        const CodegenOptions& options = CodegenOptions{}
    ) const;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace codegen
} // namespace designer
} // namespace cppui
