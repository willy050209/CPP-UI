/**
 * @file CMakeSnippetEmitter.hpp
 * @brief CMake 專案與 Target 建置設定程式碼發射器
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
/// CMake 構建指令碼發射器 (CMakeSnippetEmitter)
/// 負責為生成的表單代碼產生可直接納入既有專案或獨立編譯的 CMakeLists.txt 代碼片段。
/// </summary>
class CPPUI_API CMakeSnippetEmitter {
public:
    CMakeSnippetEmitter();
    ~CMakeSnippetEmitter();
    CMakeSnippetEmitter(CMakeSnippetEmitter&& other) noexcept;
    CMakeSnippetEmitter& operator=(CMakeSnippetEmitter&& other) noexcept;

    CMakeSnippetEmitter(const CMakeSnippetEmitter&) = delete;
    CMakeSnippetEmitter& operator=(const CMakeSnippetEmitter&) = delete;

    /// <summary>
    /// 發射可嵌入既有 CMakeLists.txt 的 Target 指令碼片段。
    /// </summary>
    /// <param name="document">表單文件。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <returns>CMake 片段字串。</returns>
    COMPAT_NODISCARD std::string EmitTargetSnippet(
        const FormDocumentNode& document,
        const CodegenOptions& options = CodegenOptions{}
    ) const;

    /// <summary>
    /// 發射獨立且完整的 CMakeLists.txt 檔案內容。
    /// </summary>
    /// <param name="document">表單文件。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <returns>完整 CMakeLists.txt 內容字串。</returns>
    COMPAT_NODISCARD std::string EmitStandaloneCMakeLists(
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
