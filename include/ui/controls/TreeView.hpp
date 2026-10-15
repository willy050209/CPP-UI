/**
 * @file TreeView.hpp
 * @brief 階層樹狀檢視控制項，支援節點展開/收合、層級縮排、延遲載入 (Lazy Loading) 與選取變更
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "compat/Compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/input/RoutedEvent.hpp"

namespace cppui {
namespace controls {

class TreeView;

/// <summary>
/// 樹狀節點項目 (TreeViewItem)
/// </summary>
class TreeViewItem {
public:
    using ExpandingHandler = std::function<void(TreeViewItem&)>;

    /// <summary>
    /// 建構預設空白樹狀節點。
    /// </summary>
    TreeViewItem();

    /// <summary>
    /// 建構具備指定標題文字之樹狀節點。
    /// </summary>
    /// <param name="text">節點標題文字。</param>
    explicit TreeViewItem(compat::string_view text);

    /// <summary>
    /// 解構子。
    /// </summary>
    ~TreeViewItem() = default;

    /// <summary>
    /// 取得節點文字。
    /// </summary>
    /// <returns>文字視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetText() const noexcept { return m_text; }

    /// <summary>
    /// 設定節點文字。
    /// </summary>
    /// <param name="text">欲設定之文字。</param>
    void SetText(compat::string_view text);

    /// <summary>
    /// 查詢節點是否處於展開狀態。
    /// </summary>
    /// <returns>若展開傳回 true。</returns>
    COMPAT_NODISCARD bool IsExpanded() const noexcept { return m_isExpanded; }

    /// <summary>
    /// 設定節點展開或收合狀態。
    /// </summary>
    /// <param name="expanded">true 為展開，false 為收合。</param>
    void SetExpanded(bool expanded);

    /// <summary>
    /// 查詢節點是否擁有子項目。
    /// </summary>
    /// <returns>若擁有子項目傳回 true。</returns>
    COMPAT_NODISCARD bool HasChildren() const noexcept { return !m_items.empty(); }

    /// <summary>
    /// 取得子節點清單。
    /// </summary>
    /// <returns>子節點智慧指標清單。</returns>
    COMPAT_NODISCARD const std::vector<std::shared_ptr<TreeViewItem>>& GetItems() const noexcept { return m_items; }

    /// <summary>
    /// 於當前節點下新增文字子項目。
    /// </summary>
    /// <param name="text">子項目標題文字。</param>
    /// <returns>新建立之子項目指標。</returns>
    std::shared_ptr<TreeViewItem> AddItem(compat::string_view text);

    /// <summary>
    /// 於當前節點下掛載子節點。
    /// </summary>
    /// <param name="item">欲新增之子節點實例。</param>
    /// <exception cref="std::invalid_argument">當 item 為空時擲出。</exception>
    void AddItem(std::shared_ptr<TreeViewItem> item);

    /// <summary>
    /// 移除指定之子節點。
    /// </summary>
    /// <param name="item">欲移除之子節點。</param>
    void RemoveItem(const std::shared_ptr<TreeViewItem>& item);

    /// <summary>
    /// 清除所有子節點。
    /// </summary>
    void ClearItems();

    /// <summary>
    /// 取得父節點指標 (若為根節點則傳回 nullptr)。
    /// </summary>
    /// <returns>父節點指標。</returns>
    COMPAT_NODISCARD TreeViewItem* GetParent() const noexcept { return m_parent; }

    /// <summary>
    /// 設定父節點指標。
    /// </summary>
    /// <param name="parent">父節點指標。</param>
    void SetParent(TreeViewItem* parent) noexcept { m_parent = parent; }

    /// <summary>
    /// 設定展開回呼處理常式 (用於延遲載入 Lazy Loading)。
    /// </summary>
    /// <param name="handler">回呼常式。</param>
    void SetExpandingHandler(ExpandingHandler handler) { m_expandingHandler = std::move(handler); }

    /// <summary>
    /// 觸發展開處理回呼。
    /// </summary>
    void TriggerExpanding();

    /// <summary>
    /// 查詢是否已配置展開回呼。
    /// </summary>
    /// <returns>若已設定傳回 true。</returns>
    COMPAT_NODISCARD bool HasExpandingHandler() const noexcept { return static_cast<bool>(m_expandingHandler); }

private:
    std::string m_text;
    bool m_isExpanded = false;
    bool m_expandingTriggered = false;
    TreeViewItem* m_parent = nullptr;
    std::vector<std::shared_ptr<TreeViewItem>> m_items;
    ExpandingHandler m_expandingHandler;
};

/// <summary>
/// 扁平化可見節點結構，用於視區運算與點擊命中測試。
/// </summary>
struct TreeViewFlatNode {
    TreeViewItem* item = nullptr;
    uint32_t level = 0;
};

/// <summary>
/// 樹狀檢視控制項 (TreeView)
/// 支援階層樹狀節點、展開與收合箭頭、縮排、延遲載入 (Lazy Loading) 與選取項目變更。
/// </summary>
class TreeView : public Control {
public:
    using SelectionChangedHandler = std::function<void(TreeViewItem*)>;

    /// <summary>
    /// 建構預設樹狀檢視控制項。
    /// </summary>
    TreeView();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~TreeView() override = default;

    /// <summary>
    /// 取得根層級項目清單。
    /// </summary>
    /// <returns>根節點智慧指標清單。</returns>
    COMPAT_NODISCARD const std::vector<std::shared_ptr<TreeViewItem>>& GetItems() const noexcept { return m_rootItems; }

    /// <summary>
    /// 於根層級新增文字項目。
    /// </summary>
    /// <param name="text">項目標題文字。</param>
    /// <returns>新建立之根節點指標。</returns>
    std::shared_ptr<TreeViewItem> AddRootItem(compat::string_view text);

    /// <summary>
    /// 於根層級掛載節點。
    /// </summary>
    /// <param name="item">欲新增之根節點實例。</param>
    /// <exception cref="std::invalid_argument">當 item 為空時擲出。</exception>
    void AddRootItem(std::shared_ptr<TreeViewItem> item);

    /// <summary>
    /// 移除指定之根節點。
    /// </summary>
    /// <param name="item">欲移除之根節點。</param>
    void RemoveRootItem(const std::shared_ptr<TreeViewItem>& item);

    /// <summary>
    /// 清除所有根節點與選取狀態。
    /// </summary>
    void ClearItems();

    /// <summary>
    /// 取得當前選定之節點 (若無選取則為 nullptr)。
    /// </summary>
    /// <returns>選取節點指標。</returns>
    COMPAT_NODISCARD TreeViewItem* GetSelectedItem() const noexcept { return m_selectedItem; }

    /// <summary>
    /// 設定當前選定之節點。
    /// </summary>
    /// <param name="item">目標節點指標。</param>
    void SetSelectedItem(TreeViewItem* item);

    /// <summary>
    /// 取得單一項目高度 (像素)。
    /// </summary>
    /// <returns>項目高度。</returns>
    COMPAT_NODISCARD float GetItemHeight() const noexcept { return m_itemHeight; }

    /// <summary>
    /// 設定單一項目高度 (像素)。
    /// </summary>
    /// <param name="height">高度數值，必須大於 0.0f。</param>
    /// <exception cref="std::invalid_argument">當 height 小於或等於 0.0f 時擲出。</exception>
    void SetItemHeight(float height);

    /// <summary>
    /// 取得每層階層之縮排寬度 (像素)。
    /// </summary>
    /// <returns>縮排寬度。</returns>
    COMPAT_NODISCARD float GetIndentWidth() const noexcept { return m_indentWidth; }

    /// <summary>
    /// 設定每層階層之縮排寬度 (像素)。
    /// </summary>
    /// <param name="indent">縮排數值，必須大於等於 0.0f。</param>
    /// <exception cref="std::invalid_argument">當 indent 小於 0.0f 時擲出。</exception>
    void SetIndentWidth(float indent);

    /// <summary>
    /// 取得垂直滾動偏移量 (像素)。
    /// </summary>
    /// <returns>滾動偏移值。</returns>
    COMPAT_NODISCARD float GetScrollOffset() const noexcept { return m_scrollOffset; }

    /// <summary>
    /// 設定垂直滾動偏移量。
    /// </summary>
    /// <param name="offset">滾動偏移值。</param>
    void SetScrollOffset(float offset);

    /// <summary>
    /// 註冊選取項目變更事件處理常式。
    /// </summary>
    /// <param name="handler">回呼常式。</param>
    void OnSelectedItemChanged(SelectionChangedHandler handler);

    /// <summary>
    /// 取得所有目前在視覺上可見（展開）的扁平化節點清單。
    /// </summary>
    /// <returns>扁平化節點陣列。</returns>
    COMPAT_NODISCARD std::vector<TreeViewFlatNode> GetFlattenedVisibleItems() const;

    /// <summary>
    /// 排版測量階段，計算樹狀檢視期望尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 繪製樹狀背景、階層箭頭與項目文字。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣。
    /// </summary>
    /// <param name="theme">主題引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    void InitializeHandlers();
    void FlattenItemRecursive(TreeViewItem* item, uint32_t level, std::vector<TreeViewFlatNode>& out) const;

    std::vector<std::shared_ptr<TreeViewItem>> m_rootItems;
    TreeViewItem* m_selectedItem = nullptr;
    float m_itemHeight = 24.0f;
    float m_indentWidth = 20.0f;
    float m_scrollOffset = 0.0f;
    core::Color m_selectionColor;
    core::Color m_selectionTextColor;
    SelectionChangedHandler m_selectionHandler;
};

} // namespace controls
} // namespace cppui
