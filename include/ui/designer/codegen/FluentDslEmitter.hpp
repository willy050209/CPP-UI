/**
 * @file FluentDslEmitter.hpp
 * @brief 現代 C++20 宣告式 Fluent RAD DSL 階層程式碼發射器
 */

#pragma once

#include <memory>
#include <string>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/WidgetNode.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/codegen/CodegenTypes.hpp"

namespace cppui {
namespace designer {
namespace codegen {

/// <summary>
/// 宣告式 Fluent RAD DSL 程式碼發射器 (FluentDslEmitter)
/// 將 AST 控制項節點樹轉換為鏈式宣告式 C++ DSL 程式碼片段。
/// </summary>
class CPPUI_API FluentDslEmitter {
public:
    /// <summary>預設建構子。</summary>
    FluentDslEmitter();

    /// <summary>解構子，於 .cpp 完整型別處實作。</summary>
    ~FluentDslEmitter();

    /// <summary>移動建構子。</summary>
    FluentDslEmitter(FluentDslEmitter&& other) noexcept;

    /// <summary>移動賦值運算子。</summary>
    FluentDslEmitter& operator=(FluentDslEmitter&& other) noexcept;

    FluentDslEmitter(const FluentDslEmitter&) = delete;
    FluentDslEmitter& operator=(const FluentDslEmitter&) = delete;

    /// <summary>
    /// 將單一控制項節點 (及其子樹) 發射為 Fluent DSL 程式碼。
    /// </summary>
    /// <param name="node">目標節點。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <param name="indentLevel">起始縮排層級。</param>
    /// <returns>格式化後的 Fluent DSL C++ 字串。</returns>
    COMPAT_NODISCARD std::string EmitNode(
        const WidgetNode& node,
        const CodegenOptions& options = CodegenOptions{},
        size_t indentLevel = 0
    ) const;

    /// <summary>
    /// 將整份表單文件發射為包含命名空間與函式包裝的 Fluent DSL 完整片段。
    /// </summary>
    /// <param name="document">目標表單文件。</param>
    /// <param name="options">代碼生成選項。</param>
    /// <returns>完整 C++ 原始碼字串。</returns>
    COMPAT_NODISCARD std::string EmitDocument(
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
