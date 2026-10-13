/*
 * File: ComboBox.hpp
 * Purpose: Declares ComboBox dropdown selection control.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "compat/compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/Popup.hpp"

namespace cppui {
namespace layout {
class StackPanel;
} // namespace layout

namespace controls {

/// <summary>
/// 下拉選單控制項 (ComboBox)
/// 組合文字顯示方塊、下拉按鈕與浮動項目清單 Popup，支援智慧翻轉與選取變更通知。
/// </summary>
class ComboBox : public Control {
public:
    using SelectionChangedHandler = std::function<void(ComboBox&, int32_t)>;

    /// <summary>
    /// 預設建構子。
    /// </summary>
    ComboBox();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~ComboBox() override;

    /// <summary>
    /// 新增純文字選項項目。
    /// </summary>
    /// <param name="text">選項文字視圖。</param>
    void AddItem(compat::string_view text);

    /// <summary>
    /// 插入純文字選項項目至指定索引。
    /// </summary>
    /// <param name="index">目標索引。</param>
    /// <param name="text">選項文字視圖。</param>
    void InsertItem(size_t index, compat::string_view text);

    /// <summary>
    /// 移除指定索引之選項項目。
    /// </summary>
    /// <param name="index">欲移除之索引。</param>
    void RemoveItem(size_t index);

    /// <summary>
    /// 清除所有選項項目。
    /// </summary>
    void ClearItems();

    /// <summary>
    /// 取得選項項目總數。
    /// </summary>
    /// <returns>項目數量。</returns>
    COMPAT_NODISCARD size_t GetItemCount() const noexcept { return m_items.size(); }

    /// <summary>
    /// 依索引取得選項項目文字。
    /// </summary>
    /// <param name="index">項目索引。</param>
    /// <returns>選項文字視圖，若越界回傳空視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetItem(size_t index) const;

    /// <summary>
    /// 取得當前選取之索引。
    /// </summary>
    /// <returns>選取索引，未選取傳回 -1。</returns>
    COMPAT_NODISCARD int32_t GetSelectedIndex() const noexcept { return m_selectedIndex; }

    /// <summary>
    /// 設定當前選取之索引。
    /// </summary>
    /// <param name="index">選取索引。</param>
    void SetSelectedIndex(int32_t index);

    /// <summary>
    /// 取得當前選取之選項項目文字。
    /// </summary>
    /// <returns>選取文字視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetSelectedItem() const;

    /// <summary>
    /// 查詢下拉清單是否處於展開狀態。
    /// </summary>
    /// <returns>若已展開傳回 true。</returns>
    COMPAT_NODISCARD bool IsDropDownOpen() const noexcept;

    /// <summary>
    /// 設定下拉清單展開或收合狀態。
    /// </summary>
    /// <param name="open">true 展開，false 收合。</param>
    void SetIsDropDownOpen(bool open);

    /// <summary>
    /// 註冊選取變更事件回呼。
    /// </summary>
    /// <param name="handler">事件委派常式。</param>
    void AddSelectionChangedHandler(SelectionChangedHandler handler);

protected:
    /// <summary>
    /// 繪製 ComboBox 框體、選取文字與下拉箭頭。
    /// </summary>
    /// <param name="dc">繪製上下文引用。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 測量 ComboBox 期望尺寸。
    /// </summary>
    /// <param name="availableSize">可用尺寸約束。</param>
    /// <returns>期望尺寸大小。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

private:
    void RebuildDropdownItems();

    std::vector<std::string> m_items;
    int32_t m_selectedIndex = -1;
    bool m_isHovered = false;

    std::shared_ptr<Popup> m_dropdownPopup;
    std::shared_ptr<layout::StackPanel> m_dropdownPanel;
    std::vector<SelectionChangedHandler> m_selectionHandlers;
};

} // namespace controls
} // namespace cppui
