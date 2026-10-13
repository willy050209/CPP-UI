/*
 * File: MenuItem.hpp
 * Purpose: Declares MenuItem control for menus and context menus with submenu support.
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "compat/compat.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/mvvm/ICommand.hpp"

namespace cppui {
namespace controls {

class Popup;

/// <summary>
/// 選單項目控制項 (MenuItem)
/// 支援標題、快速鍵提示、ICommand 命令綁定、點擊事件與巢狀次級選單 (Submenu)。
/// </summary>
class MenuItem : public ContentControl {
public:
    using ClickHandler = std::function<void(MenuItem&)>;

    /// <summary>
    /// 預設建構子。
    /// </summary>
    MenuItem();

    /// <summary>
    /// 建構具備標題文字之選單項目。
    /// </summary>
    /// <param name="header">選單標題字串視圖。</param>
    explicit MenuItem(compat::string_view header);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~MenuItem() override;

    /// <summary>
    /// 取得選單項目標題文字視圖。
    /// </summary>
    /// <returns>標題字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetHeader() const noexcept { return m_header; }

    /// <summary>
    /// 設定選單項目標題文字。
    /// </summary>
    /// <param name="header">標題字串視圖。</param>
    void SetHeader(compat::string_view header);

    /// <summary>
    /// 取得快捷鍵手勢提示文字視圖 (如 "Ctrl+C")。
    /// </summary>
    /// <returns>快捷鍵提示字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetInputGestureText() const noexcept { return m_inputGestureText; }

    /// <summary>
    /// 設定快捷鍵手勢提示文字。
    /// </summary>
    /// <param name="text">提示字串視圖。</param>
    void SetInputGestureText(compat::string_view text);

    /// <summary>
    /// 取得關聯之 MVVM 命令介面指標。
    /// </summary>
    /// <returns>ICommand 智慧指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<mvvm::ICommand> GetCommand() const noexcept { return m_command; }

    /// <summary>
    /// 設定關聯之 MVVM 命令。
    /// </summary>
    /// <param name="command">ICommand 智慧指標。</param>
    void SetCommand(std::shared_ptr<mvvm::ICommand> command);

    /// <summary>
    /// 查詢是否包含次級選單項目。
    /// </summary>
    /// <returns>若包含次級選單傳回 true。</returns>
    COMPAT_NODISCARD bool HasSubmenu() const noexcept { return !m_submenuItems.empty(); }

    /// <summary>
    /// 新增次級選單子項目。
    /// </summary>
    /// <param name="item">次級 MenuItem 智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 item 為 nullptr 時擲出。</exception>
    void AddMenuItem(std::shared_ptr<MenuItem> item);

    /// <summary>
    /// 移除次級選單子項目。
    /// </summary>
    /// <param name="item">待移除之 MenuItem 智慧指標。</param>
    void RemoveMenuItem(const std::shared_ptr<MenuItem>& item);

    /// <summary>
    /// 取得次級選單項目陣列清單。
    /// </summary>
    /// <returns>MenuItem 陣列引用。</returns>
    COMPAT_NODISCARD const std::vector<std::shared_ptr<MenuItem>>& GetSubmenuItems() const noexcept {
        return m_submenuItems;
    }

    /// <summary>
    /// 查詢當前是否處於游標反白突顯狀態。
    /// </summary>
    /// <returns>若反白傳回 true。</returns>
    COMPAT_NODISCARD bool IsHighlighted() const noexcept { return m_isHighlighted; }

    /// <summary>
    /// 設定游標反白狀態。
    /// </summary>
    /// <param name="value">true 反白，false 取消。</param>
    void SetHighlighted(bool value);

    /// <summary>
    /// 展開次級選單浮層。
    /// </summary>
    void OpenSubmenu();

    /// <summary>
    /// 收合次級選單浮層。
    /// </summary>
    void CloseSubmenu();

    /// <summary>
    /// 註冊選單點擊事件回呼。
    /// </summary>
    /// <param name="handler">點擊回呼委派。</param>
    void AddClickHandler(ClickHandler handler);

    /// <summary>
    /// 觸發點擊操作，執行 Click 事件與關聯 Command。
    /// </summary>
    void TriggerClick();

protected:
    /// <summary>
    /// 繪製選單項目底色、文字、快捷鍵與箭頭。
    /// </summary>
    /// <param name="dc">繪製上下文引用。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 測量選單寬高尺寸。
    /// </summary>
    /// <param name="availableSize">可用尺寸約束。</param>
    /// <returns>期望尺寸大小。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

private:
    std::string m_header;
    std::string m_inputGestureText;
    std::shared_ptr<mvvm::ICommand> m_command;
    bool m_isHighlighted = false;

    std::vector<std::shared_ptr<MenuItem>> m_submenuItems;
    std::shared_ptr<Popup> m_submenuPopup;
    std::vector<ClickHandler> m_clickHandlers;
};

} // namespace controls
} // namespace cppui
