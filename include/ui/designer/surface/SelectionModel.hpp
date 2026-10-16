/**
 * @file SelectionModel.hpp
 * @brief 設計表面元件選取狀態機與反應式通知模型
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"

namespace cppui {
namespace designer {
namespace surface {

/// <summary>
/// 設計表面選取模型 (SelectionModel)
/// 負責管理當前畫布上選取的元件 UUID 清單，支援單選、多選追加、反選、批次匡選與狀態異動推播。
/// </summary>
class CPPUI_API SelectionModel {
public:
    SelectionModel();
    ~SelectionModel();
    SelectionModel(SelectionModel&& other) noexcept;
    SelectionModel& operator=(SelectionModel&& other) noexcept;

    SelectionModel(const SelectionModel&) = delete;
    SelectionModel& operator=(const SelectionModel&) = delete;

    /// <summary>
    /// 設定單一選取項目 (會清空先前已選取之項目)。
    /// </summary>
    /// <param name="nodeUuid">節點 UUID 字串。</param>
    void SetSelection(std::string nodeUuid);

    /// <summary>
    /// 追加選取項目 (多選模式)。
    /// </summary>
    /// <param name="nodeUuid">節點 UUID 字串。</param>
    void AddToSelection(std::string nodeUuid);

    /// <summary>
    /// 從選取清單中移除指定節點。
    /// </summary>
    /// <param name="nodeUuid">節點 UUID 字串視圖。</param>
    void RemoveFromSelection(compat::string_view nodeUuid);

    /// <summary>
    /// 清空所有選取狀態。
    /// </summary>
    void ClearSelection();

    /// <summary>
    /// 批次設定選取範圍 (例如框選完成後直接置換)。
    /// </summary>
    /// <param name="uuids">選取節點 UUID 清單。</param>
    void SetSelectionRange(const std::vector<std::string>& uuids);

    /// <summary>
    /// 檢查特定節點是否處於選取狀態。
    /// </summary>
    /// <param name="nodeUuid">節點 UUID 字串視圖。</param>
    /// <returns>若已選取回傳 true。</returns>
    COMPAT_NODISCARD bool IsSelected(compat::string_view nodeUuid) const noexcept;

    /// <summary>
    /// 取得所有當前選取之節點 UUID 清單。
    /// </summary>
    /// <returns>UUID 清單常數引用。</returns>
    COMPAT_NODISCARD const std::vector<std::string>& GetSelectedUuids() const noexcept;

    /// <summary>
    /// 取得主要選取之節點 UUID (通常為最後選取或單選之項目)。
    /// </summary>
    /// <returns>主要選取 UUID，若無選取則為 nullopt。</returns>
    COMPAT_NODISCARD compat::optional<std::string> GetPrimarySelection() const noexcept;

    /// <summary>
    /// 取得當前選取之元件總數。
    /// </summary>
    /// <returns>選取數量。</returns>
    COMPAT_NODISCARD size_t GetSelectionCount() const noexcept;

    /// <summary>
    /// 反應式選取變更通知匯流排 (推播最新被選取的 UUID 陣列)。
    /// </summary>
    /// <returns>DesignerEvent 引用。</returns>
    DesignerEvent<std::vector<std::string>>& SelectionChangedSignal() noexcept;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace surface
} // namespace designer
} // namespace cppui
