/*
 * File: ContextMenu.hpp
 * Purpose: Declares ContextMenu popup control and ContextMenuService.
 */

#pragma once

#include <memory>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/controls/Popup.hpp"
#include "ui/controls/MenuItem.hpp"

namespace cppui {
namespace layout {
class StackPanel;
} // namespace layout

namespace controls {

/// <summary>
/// 右鍵上下文選單控制項 (ContextMenu)
/// 繼承自 Popup，承載直向排列之 MenuItem 清單，支援智慧翻轉與外部點擊關閉。
/// </summary>
class ContextMenu : public Popup {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    ContextMenu();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~ContextMenu() override = default;

    /// <summary>
    /// 新增選單項目。
    /// </summary>
    /// <param name="item">MenuItem 智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 item 為 nullptr 時擲出。</exception>
    void AddMenuItem(std::shared_ptr<MenuItem> item);

    /// <summary>
    /// 移除指定選單項目。
    /// </summary>
    /// <param name="item">待移除之 MenuItem 智慧指標。</param>
    void RemoveMenuItem(const std::shared_ptr<MenuItem>& item);

    /// <summary>
    /// 清空所有選單項目。
    /// </summary>
    void ClearMenuItems();

    /// <summary>
    /// 取得選單項目總數。
    /// </summary>
    /// <returns>項目數量。</returns>
    COMPAT_NODISCARD size_t GetMenuItemCount() const noexcept;

    /// <summary>
    /// 依索引取得選單項目。
    /// </summary>
    /// <param name="index">項目索引。</param>
    /// <returns>MenuItem 智慧指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<MenuItem> GetMenuItem(size_t index) const;

    /// <summary>
    /// 於指定目標元件之指定相對座標處彈出上下文選單。
    /// </summary>
    /// <param name="target">錨定目標 UI 元件指標。</param>
    /// <param name="relativePos">相對於目標元件之彈出座標點。</param>
    void Show(core::UIElement* target, core::Point relativePos);

    /// <summary>
    /// 關閉上下文選單。
    /// </summary>
    void Close();

private:
    std::shared_ptr<layout::StackPanel> m_panel;
    std::vector<std::shared_ptr<MenuItem>> m_items;
};

/// <summary>
/// 上下文選單附屬服務管理器 (ContextMenuService)
/// 為任意 UIElement 提供右鍵點擊自動彈出 ContextMenu 之綁定。
/// </summary>
class ContextMenuService {
public:
    /// <summary>
    /// 設定元件之右鍵上下文選單。
    /// </summary>
    /// <param name="element">目標 UI 元件指標。</param>
    /// <param name="menu">ContextMenu 智慧指標。</param>
    static void SetContextMenu(core::UIElement* element, std::shared_ptr<ContextMenu> menu);

    /// <summary>
    /// 取得元件之右鍵上下文選單。
    /// </summary>
    /// <param name="element">目標 UI 元件指標。</param>
    /// <returns>ContextMenu 智慧指標，若無則傳回 nullptr。</returns>
    static std::shared_ptr<ContextMenu> GetContextMenu(core::UIElement* element);
};

} // namespace controls
} // namespace cppui
