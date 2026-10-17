/**
 * @file CodePreviewWindow.hpp
 * @brief 視覺化表單設計器獨立即時代碼預覽視窗 (CodePreviewWindow)
 */

#pragma once

#include <memory>
#include <string>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/app/Form.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/surface/SelectionModel.hpp"
#include "ui/designer/preview/LiveCodePreviewPanel.hpp"

namespace cppui {
namespace designer {
namespace preview {

/// <summary>
/// 獨立即時代碼預覽視窗 (CodePreviewWindow)
/// 提供語法高亮、行號顯示、五種架構程式碼分頁切換與代碼複製功能。
/// 採用現代深常數性 Pimpl 慣用法封裝。
/// </summary>
class CPPUI_API CodePreviewWindow : public app::Form {
public:
    /// <summary>
    /// 建構獨立代碼預覽視窗實例。
    /// </summary>
    /// <param name="document">表單文件 AST 根指標。</param>
    /// <param name="selectionModel">選取模型指標。</param>
    explicit CodePreviewWindow(
        FormDocumentNode* document = nullptr,
        surface::SelectionModel* selectionModel = nullptr
    );

    /// <summary>
    /// 虛擬解構子，於 .cpp 完整型別處實作。
    /// </summary>
    ~CodePreviewWindow() override;

    CodePreviewWindow(const CodePreviewWindow&) = delete;
    CodePreviewWindow& operator=(const CodePreviewWindow&) = delete;
    CodePreviewWindow(CodePreviewWindow&&) = delete;
    CodePreviewWindow& operator=(CodePreviewWindow&&) = delete;

    /// <summary>
    /// 設定表單文件 AST 參照並重新生成代碼。
    /// </summary>
    /// <param name="document">文件指標。</param>
    void SetDocument(FormDocumentNode* document);

    /// <summary>
    /// 設定當前代碼分頁檢視模式。
    /// </summary>
    /// <param name="mode">預覽模式。</param>
    void SetViewMode(PreviewViewMode mode);

    /// <summary>
    /// 取得當前檢視模式。
    /// </summary>
    /// <returns>PreviewViewMode 列舉。</returns>
    COMPAT_NODISCARD PreviewViewMode GetViewMode() const noexcept;

    /// <summary>
    /// 強制立即更新並重新著色代碼。
    /// </summary>
    void RefreshCode();

    /// <summary>
    /// 取得當前分頁之純文字代碼。
    /// </summary>
    /// <returns>代碼字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetCurrentCode() const noexcept;

    /// <summary>
    /// 複製當前分頁代碼至系統剪貼簿。
    /// </summary>
    void CopyCodeToClipboard();

    struct Impl;

private:
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace preview
} // namespace designer
} // namespace cppui
