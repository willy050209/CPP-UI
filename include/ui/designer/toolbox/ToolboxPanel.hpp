/**
 * @file ToolboxPanel.hpp
 * @brief 視覺化表單設計器垂直分類控制項工具箱 (ToolboxPanel)
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/controls/Control.hpp"
#include "ui/designer/ast/AstTypes.hpp"

namespace cppui {
namespace designer {
namespace toolbox {

/// <summary>
/// 工具箱控制項項目規格
/// </summary>
struct ToolboxItem {
    NodeType type;
    std::string icon;
    std::string displayName;
    std::string englishName;
};

/// <summary>
/// 工具箱分類分組規格 (支援收合/展開)
/// </summary>
struct ToolboxCategory {
    std::string name;
    bool isExpanded = true;
    std::vector<ToolboxItem> items;
};

/// <summary>
/// 拖曳放置回呼常式委派型別
/// </summary>
using ItemDropCallback = std::function<void(NodeType type, const core::Point& dropPoint)>;

/// <summary>
/// 垂直清單分類控制項工具箱面板 (ToolboxPanel)
/// 提供常用控制項、版面容器、資料與集合、元件與對話框之清晰分類、
/// 可摺疊折疊項目、即時搜尋過濾、按鈕武裝放置與滑鼠拖曳放置能力。
/// </summary>
class CPPUI_API ToolboxPanel : public controls::Control {
public:
    /// <summary>
    /// 建構垂直分類工具箱面板實例。
    /// </summary>
    ToolboxPanel();

    /// <summary>
    /// 虛擬解構子，於 .cpp 完整型別處實作。
    /// </summary>
    ~ToolboxPanel() override;

    ToolboxPanel(const ToolboxPanel&) = delete;
    ToolboxPanel& operator=(const ToolboxPanel&) = delete;
    ToolboxPanel(ToolboxPanel&&) = delete;
    ToolboxPanel& operator=(ToolboxPanel&&) = delete;

    /// <summary>
    /// 取得當前所有分類分組。
    /// </summary>
    /// <returns>分類群組唯讀參照。</returns>
    COMPAT_NODISCARD const std::vector<ToolboxCategory>& GetCategories() const noexcept;

    /// <summary>
    /// 設定分類展開或摺疊狀態。
    /// </summary>
    /// <param name="categoryIndex">分類索引。</param>
    /// <param name="expanded">true 為展開，false 為摺疊。</param>
    void SetCategoryExpanded(size_t categoryIndex, bool expanded);

    /// <summary>
    /// 切換分類展開/摺疊狀態。
    /// </summary>
    /// <param name="categoryIndex">分類索引。</param>
    void ToggleCategory(size_t categoryIndex);

    /// <summary>
    /// 取得目前選取/武裝的控制項型別。
    /// </summary>
    /// <returns>NodeType 數值，若未選取則為 nullopt。</returns>
    COMPAT_NODISCARD compat::optional<NodeType> GetSelectedType() const noexcept;

    /// <summary>
    /// 設定目前選取/武裝的控制項型別。
    /// </summary>
    /// <param name="type">節點型別。</param>
    void SetSelectedType(compat::optional<NodeType> type);

    /// <summary>
    /// 設定搜尋關鍵字以即時過濾控制項。
    /// </summary>
    /// <param name="filter">搜尋字串。</param>
    void SetFilterText(compat::string_view filter);

    /// <summary>
    /// 取得當前搜尋過濾字串。
    /// </summary>
    /// <returns>過濾字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetFilterText() const noexcept;

    /// <summary>
    /// 註冊雙擊項目回呼 (快捷新增至中央畫布)。
    /// </summary>
    /// <param name="callback">回呼常式。</param>
    void OnItemDoubleClicked(std::function<void(NodeType)> callback);

    /// <summary>
    /// 註冊項目拖曳釋放回呼常式。
    /// </summary>
    /// <param name="callback">拖曳放置回呼常式。</param>
    void OnItemDropped(ItemDropCallback callback);

    /// <summary>
    /// 公開指標事件處理常式 (供手動觸發與測試)。
    /// </summary>
    void HandlePointerPressed(input::PointerEventArgs& e);
    void HandlePointerMoved(input::PointerEventArgs& e);
    void HandlePointerReleased(input::PointerEventArgs& e);

protected:
    void OnRender(rendering::DrawingContext& dc) override;
    void OnPointerPressedCore(input::PointerEventArgs& e) override;
    void OnPointerMovedCore(input::PointerEventArgs& e) override;
    void OnPointerReleasedCore(input::PointerEventArgs& e) override;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace toolbox
} // namespace designer
} // namespace cppui
