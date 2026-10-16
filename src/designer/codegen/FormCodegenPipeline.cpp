/**
 * @file FormCodegenPipeline.cpp
 * @brief CPPUI Designer 程式碼生成總體編排管道實作
 */

#include "ui/designer/codegen/FormCodegenPipeline.hpp"
#include "ui/designer/codegen/FluentDslEmitter.hpp"
#include "ui/designer/codegen/FormClassEmitter.hpp"
#include "ui/designer/codegen/CMakeSnippetEmitter.hpp"

namespace cppui {
namespace designer {
namespace codegen {

struct FormCodegenPipeline::Impl {
    FluentDslEmitter dslEmitter;
    FormClassEmitter classEmitter;
    CMakeSnippetEmitter cmakeEmitter;
};

FormCodegenPipeline::FormCodegenPipeline() : m_impl(std::make_unique<Impl>()) {}
FormCodegenPipeline::~FormCodegenPipeline() = default;
FormCodegenPipeline::FormCodegenPipeline(FormCodegenPipeline&& other) noexcept = default;
FormCodegenPipeline& FormCodegenPipeline::operator=(FormCodegenPipeline&& other) noexcept = default;

compat::expected<CodegenResult, DesignerError> FormCodegenPipeline::Generate(
    const FormDocumentNode& document,
    const CodegenOptions& options
) const {
    if (!document.GetRoot()) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidNode;
        err.message = "FormDocumentNode does not contain a root container";
        return compat::unexpected(std::move(err));
    }

    CodegenResult result;

    // 1. 生成 Form Partial Class 三件套與 SourceMap
    result.designerHeader = m_impl->classEmitter.EmitDesignerHeader(document, options, &result.sourceMap);
    result.userHeader = m_impl->classEmitter.EmitUserHeader(document, options);
    result.userSource = m_impl->classEmitter.EmitUserSource(document, options);

    // 2. 生成獨立 Fluent DSL 片段
    result.fluentDslSnippet = m_impl->dslEmitter.EmitDocument(document, options);

    // 3. 生成 CMake 指令碼片段 (若啟用)
    if (options.emitCMake) {
        result.cmakeSnippet = m_impl->cmakeEmitter.EmitTargetSnippet(document, options);
    }

    return result;
}

compat::expected<std::string, DesignerError> FormCodegenPipeline::GenerateFluentSnippet(
    const WidgetNode& rootNode,
    const CodegenOptions& options
) const {
    return m_impl->dslEmitter.EmitNode(rootNode, options, 0);
}

} // namespace codegen
} // namespace designer
} // namespace cppui
