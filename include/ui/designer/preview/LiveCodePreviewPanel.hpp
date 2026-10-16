/**
 * @file LiveCodePreviewPanel.hpp
 * @brief CPPUI Designer 即時代碼預覽面板 (LiveCodePreviewPanel)
 */

#pragma once

#include <memory>
#include <string>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/controls/Control.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/surface/SelectionModel.hpp"
#include "ui/designer/codegen/CodegenTypes.hpp"

namespace cppui {
namespace designer {
namespace preview {

/// <summary>
/// 即時代碼預覽檢視模式列舉
/// </summary>
enum class PreviewViewMode : uint8_t {
    FluentDsl          = 0, // 宣告式 Fluent DSL 代碼片段
    ClassicDesignerHpp = 1, // MainForm.designer.hpp 設計器成員宣告與初始化
    ClassicHeader      = 2, // MainForm.hpp 使用者邏輯標頭檔
    ClassicSource      = 3, // MainForm.cpp 使用者實作檔
    CMakeLists         = 4  // CMakeLists.txt 構建腳本
};

/// <summary>
/// 即時代碼預覽面板主控制項 (LiveCodePreviewPanel)
/// 訂閱 AST 領域模型異動匯流排，在背景即時重構 5 種代碼格式，
/// 並透過 SourceMap 雙向索引實現當前選取控制項之行號自動高亮定位。
/// 採用現代深常數性 Pimpl 模式封裝。
/// </summary>
class CPPUI_API LiveCodePreviewPanel : public controls::Control {
public:
    /// <summary>
    /// 建構即時代碼預覽面板實例。
    /// </summary>
    /// <param name="document">表單文件 AST 根指標。</param>
    /// <param name="selectionModel">選取模型指標。</param>
    explicit LiveCodePreviewPanel(
        FormDocumentNode* document = nullptr,
        surface::SelectionModel* selectionModel = nullptr
    );

    /// <summary>虛擬解構子，於 .cpp 完整型別處實作。</summary>
    ~LiveCodePreviewPanel() override;

    LiveCodePreviewPanel(const LiveCodePreviewPanel&) = delete;
    LiveCodePreviewPanel& operator=(const LiveCodePreviewPanel&) = delete;
    LiveCodePreviewPanel(LiveCodePreviewPanel&&) = delete;
    LiveCodePreviewPanel& operator=(LiveCodePreviewPanel&&) = delete;

    // --- 外部模型關聯掛載 ---

    void SetDocument(FormDocumentNode* document);
    COMPAT_NODISCARD FormDocumentNode* GetDocument() const noexcept;

    void SetSelectionModel(surface::SelectionModel* selectionModel);
    COMPAT_NODISCARD surface::SelectionModel* GetSelectionModel() const noexcept;

    // --- 檢視模式與代碼存取 ---

    /// <summary>設定當前檢視模式。</summary>
    /// <param name="mode">檢視模式列舉。</param>
    void SetViewMode(PreviewViewMode mode);

    /// <summary>取得當前檢視模式。</summary>
    COMPAT_NODISCARD PreviewViewMode GetViewMode() const noexcept;

    /// <summary>取得當前檢視模式下生成的代碼文字。</summary>
    COMPAT_NODISCARD const std::string& GetActiveCodeText() const noexcept;

    /// <summary>強制立即重新生成全格式代碼並更新快取。</summary>
    void Refresh();

    // --- SourceMap 雙向行號高亮 ---

    /// <summary>
    /// 依據當前選取之節點取得在 ClassicDesignerHpp 中的代碼行號高亮範圍。
    /// </summary>
    /// <returns>若有選取且映射成功回傳行號範圍，否則回傳 nullopt。</returns>
    COMPAT_NODISCARD compat::optional<codegen::SourceSpan> GetHighlightedSpan() const;

    /// <summary>
    /// 查詢指定 UUID 節點在 ClassicDesignerHpp 中的行號範圍。
    /// </summary>
    /// <param name="nodeUuid">節點不可變 UUID。</param>
    /// <returns>行號範圍，若不存在為 nullopt。</returns>
    COMPAT_NODISCARD compat::optional<codegen::SourceSpan> GetSpanForNode(compat::string_view nodeUuid) const;

    // --- 反應式通知通道 ---

    /// <summary>代碼重新生成完成通知通道 (傳遞當前檢視模式與最新代碼)。</summary>
    DesignerEvent<std::pair<PreviewViewMode, std::string>>& CodeUpdatedSignal() noexcept;

protected:
    void OnRender(rendering::DrawingContext& dc) override;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace preview
} // namespace designer
} // namespace cppui
