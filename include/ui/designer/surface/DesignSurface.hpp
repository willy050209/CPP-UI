/**
 * @file DesignSurface.hpp
 * @brief 視覺化畫布設計表面主控制項 (DesignSurface)
 */

#pragma once

#include <memory>
#include <string>
#include <map>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/controls/Control.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/commands/CommandHistoryManager.hpp"
#include "ui/designer/surface/SurfaceTypes.hpp"
#include "ui/designer/surface/SelectionModel.hpp"

namespace cppui {
namespace designer {
namespace surface {

/// <summary>
/// 視覺化畫布設計表面主控制項 (DesignSurface)
/// 封裝 AST 元件樹渲染、選取狀態機、八向縮放控制柄、智慧吸附導引線與指標拖曳互動。
/// </summary>
class CPPUI_API DesignSurface : public controls::Control {
public:
    /// <summary>
    /// 建構設計表面實例。
    /// </summary>
    /// <param name="document">表單文件 AST 根指標。</param>
    /// <param name="historyManager">Undo/Redo 歷史命令管理員指標。</param>
    explicit DesignSurface(
        FormDocumentNode* document = nullptr,
        CommandHistoryManager* historyManager = nullptr
    );

    /// <summary>虛擬解構子，於 .cpp 完整型別處實作。</summary>
    ~DesignSurface() override;

    DesignSurface(const DesignSurface&) = delete;
    DesignSurface& operator=(const DesignSurface&) = delete;
    DesignSurface(DesignSurface&&) = delete;
    DesignSurface& operator=(DesignSurface&&) = delete;

    // --- 表單文件與歷史管理員掛載 ---

    void SetDocument(FormDocumentNode* document);
    COMPAT_NODISCARD FormDocumentNode* GetDocument() const noexcept;

    void SetHistoryManager(CommandHistoryManager* historyManager);
    COMPAT_NODISCARD CommandHistoryManager* GetHistoryManager() const noexcept;

    // --- 選取模型與組態選項 ---

    COMPAT_NODISCARD SelectionModel& GetSelectionModel() noexcept;
    COMPAT_NODISCARD const SelectionModel& GetSelectionModel() const noexcept;

    COMPAT_NODISCARD SurfaceOptions& GetOptions() noexcept;
    COMPAT_NODISCARD const SurfaceOptions& GetOptions() const noexcept;
    void SetOptions(const SurfaceOptions& options);

    /// <summary>
    /// 設定畫布字體縮放比例。
    /// </summary>
    /// <param name="scale">縮放倍率 (預設 1.0f)。</param>
    void SetFontScale(float scale);

    /// <summary>
    /// 取得畫布當前字體縮放比例。
    /// </summary>
    /// <returns>縮放倍率。</returns>
    COMPAT_NODISCARD float GetFontScale() const noexcept;

    // --- 邊界快取管理 ---

    void SetNodeBounds(const std::string& nodeUuid, const core::Rect& bounds);
    COMPAT_NODISCARD core::Rect GetNodeBounds(const std::string& nodeUuid) const;
    COMPAT_NODISCARD const std::map<std::string, core::Rect>& GetAllNodeBounds() const noexcept;

    // --- 編輯命令操作捷徑 ---

    /// <summary>
    /// 新增控制項至畫布中（建立 AddWidgetCommand 並推入歷史堆疊）。
    /// </summary>
    /// <param name="type">節點型別。</param>
    /// <param name="position">放置畫布座標。</param>
    /// <param name="parentUuid">父容器節點 UUID，若為空則預設加入根容器中。</param>
    /// <returns>新增成功回傳該節點的 UUID，若失敗則回傳空字串。</returns>
    std::string AddNewWidget(NodeType type, const core::Point& position, const std::string& parentUuid = "");

    /// <summary>刪除當前選取的元件 (建立 RemoveWidgetCommand 並推入歷史堆疊)。</summary>
    /// <returns>若成功刪除至少一個元件回傳 true。</returns>
    bool DeleteSelected();

    /// <summary>執行撤銷 (Undo)。</summary>
    /// <returns>若成功撤銷回傳 true。</returns>
    bool Undo();

    /// <summary>執行重做 (Redo)。</summary>
    /// <returns>若成功重做回傳 true。</returns>
    bool Redo();

    // --- 拖曳互動狀態查詢 (供單元測試與診斷) ---

    COMPAT_NODISCARD DragMode GetCurrentDragMode() const noexcept;
    COMPAT_NODISCARD HandleType GetActiveHandle() const noexcept;
    COMPAT_NODISCARD const std::vector<SnapLine>& GetActiveGuides() const noexcept;
    COMPAT_NODISCARD core::Rect GetRubberBandRect() const noexcept;

    // 公開指標事件處理常式 (支援測試模擬與手動觸發)
    void OnPointerPressed(input::PointerEventArgs& e);
    void OnPointerMoved(input::PointerEventArgs& e);
    void OnPointerReleased(input::PointerEventArgs& e);

protected:
    void OnRender(rendering::DrawingContext& dc) override;
    void OnPostRender(rendering::DrawingContext& dc) override;

    void OnPointerPressedCore(input::PointerEventArgs& e) override;
    void OnPointerMovedCore(input::PointerEventArgs& e) override;
    void OnPointerReleasedCore(input::PointerEventArgs& e) override;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace surface
} // namespace designer
} // namespace cppui
