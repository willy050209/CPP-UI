/**
 * @file InspectorPanel.hpp
 * @brief CPPUI Designer 屬性與事件檢查器主控制項 (InspectorPanel)
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/controls/Control.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/commands/CommandHistoryManager.hpp"
#include "ui/designer/surface/SelectionModel.hpp"
#include "ui/designer/inspector/InspectorTypes.hpp"

namespace cppui {
namespace designer {
namespace inspector {

/// <summary>
/// 屬性變更通知載荷
/// </summary>
struct PropertyChangePayload {
    std::string propertyName;
    PropertyValue value;
};

/// <summary>
/// 事件變更通知載荷
/// </summary>
struct EventChangePayload {
    std::string eventName;
    std::string handlerName;
};

/// <summary>
/// 屬性與事件檢查器主控制項 (InspectorPanel)
/// 封裝單選/多選屬性交集計算、多重數值 (Mixed) 狀態機、動態附加屬性感測、
/// 主題 Token 整合、事件自動命名與批次 MacroCommand Undo/Redo 連動。
/// 採用現代 Pimpl 慣用法與深常數性指標 (compat::propagate_const) 確保 ABI 穩定性。
/// </summary>
class CPPUI_API InspectorPanel : public controls::Control {
public:
    /// <summary>
    /// 建構檢查器面板實例。
    /// </summary>
    /// <param name="document">表單文件 AST 根指標。</param>
    /// <param name="historyManager">Undo/Redo 歷史命令管理員指標。</param>
    /// <param name="selectionModel">畫布選取狀態模型指標。</param>
    explicit InspectorPanel(
        FormDocumentNode* document = nullptr,
        CommandHistoryManager* historyManager = nullptr,
        surface::SelectionModel* selectionModel = nullptr
    );

    /// <summary>虛擬解構子，於 .cpp 完整型別處實作。</summary>
    ~InspectorPanel() override;

    InspectorPanel(const InspectorPanel&) = delete;
    InspectorPanel& operator=(const InspectorPanel&) = delete;
    InspectorPanel(InspectorPanel&&) = delete;
    InspectorPanel& operator=(InspectorPanel&&) = delete;

    // --- 外部模型關聯掛載 ---

    void SetDocument(FormDocumentNode* document);
    COMPAT_NODISCARD FormDocumentNode* GetDocument() const noexcept;

    void SetHistoryManager(CommandHistoryManager* historyManager);
    COMPAT_NODISCARD CommandHistoryManager* GetHistoryManager() const noexcept;

    void SetSelectionModel(surface::SelectionModel* selectionModel);
    COMPAT_NODISCARD surface::SelectionModel* GetSelectionModel() const noexcept;

    // --- 分頁與搜尋篩選 ---

    void SetCurrentTab(InspectorTab tab);
    COMPAT_NODISCARD InspectorTab GetCurrentTab() const noexcept;

    void SetFilterText(compat::string_view filter);
    COMPAT_NODISCARD const std::string& GetFilterText() const noexcept;

    // --- 屬性與事件清單查詢 (依當前選取與搜尋條件動態計算) ---

    /// <summary>
    /// 取得目前選取節點之受檢屬性清單 (已依分類排序、附加屬性注入與搜尋過濾)。
    /// </summary>
    COMPAT_NODISCARD std::vector<InspectedProperty> GetInspectedProperties() const;

    /// <summary>
    /// 取得目前選取節點之受檢事件與 Command 清單。
    /// </summary>
    COMPAT_NODISCARD std::vector<EventBindingState> GetInspectedEvents() const;

    /// <summary>
    /// 取得指定名稱之受檢屬性。
    /// </summary>
    COMPAT_NODISCARD compat::optional<InspectedProperty> FindProperty(compat::string_view name) const;

    // --- 屬性修改與 Undo/Redo 整合 ---

    /// <summary>
    /// 修改目前選取節點之屬性數值。
    /// 多選狀態下自動聚合為單一 MacroCommand 推入歷史堆疊。
    /// </summary>
    /// <param name="propName">屬性名稱。</param>
    /// <param name="newValue">新屬性值。</param>
    /// <returns>若成功套用傳回 true。</returns>
    bool ChangeProperty(compat::string_view propName, const PropertyValue& newValue);

    /// <summary>
    /// 修改目前選取節點之附加屬性數值 (如 Canvas.Left, Grid.Row)。
    /// </summary>
    /// <param name="propName">附加屬性名稱。</param>
    /// <param name="newValue">新屬性值。</param>
    /// <returns>若成功套用傳回 true。</returns>
    bool ChangeAttachedProperty(compat::string_view propName, const PropertyValue& newValue);

    /// <summary>
    /// 將指定屬性重設為其預設值 (Reset to Default)。
    /// </summary>
    /// <param name="propName">屬性名稱。</param>
    /// <returns>若成功重設傳回 true。</returns>
    bool ResetPropertyToDefault(compat::string_view propName);

    // --- 事件綁定操作 ---

    /// <summary>
    /// 為目前選取節點綁定事件處理常式名稱。
    /// </summary>
    /// <param name="eventName">事件名稱。</param>
    /// <param name="handlerName">自訂或建議之處理常式名稱。</param>
    /// <returns>若成功綁定傳回 true。</returns>
    bool BindEvent(compat::string_view eventName, compat::string_view handlerName);

    /// <summary>
    /// 解除目前選取節點指定事件之綁定。
    /// </summary>
    /// <param name="eventName">事件名稱。</param>
    /// <returns>若成功解除傳回 true。</returns>
    bool UnbindEvent(compat::string_view eventName);

    /// <summary>
    /// 自動為目前選取節點生成標準命名之處理常式並完成綁定 (例如 "OnSubmitBtnClick")。
    /// </summary>
    /// <param name="eventName">事件名稱。</param>
    /// <returns>生成的處理常式名稱，若選取為空則傳回空字串。</returns>
    std::string AutoBindEvent(compat::string_view eventName);

    // --- 響應式事件通知通道 (DesignerEvent) ---

    /// <summary>屬性值變更通知事件通道。</summary>
    DesignerEvent<PropertyChangePayload>& PropertyChangedSignal() noexcept;

    /// <summary>事件綁定變更通知事件通道。</summary>
    DesignerEvent<EventChangePayload>& EventChangedSignal() noexcept;

    /// <summary>分頁切換通知事件通道。</summary>
    DesignerEvent<InspectorTab>& TabChangedSignal() noexcept;

protected:
    void OnRender(rendering::DrawingContext& dc) override;
    void OnPointerPressedCore(input::PointerEventArgs& e) override;
    void OnPointerWheelCore(input::PointerEventArgs& e) override;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace inspector
} // namespace designer
} // namespace cppui
