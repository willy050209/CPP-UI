/**
 * @file ListBox.hpp
 * @brief 清單方塊控制項，支援項目增刪、單選索引與鍵盤/滑鼠巡覽
 */

#pragma once

#include <vector>
#include <string>
#include <functional>
#include "compat/Compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/input/RoutedEvent.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 清單方塊控制項 (ListBox)
/// 支援文字項目清單維護、單選選取 (SelectedIndex / SelectedItem)、滑鼠點擊與鍵盤導航。
/// </summary>
class ListBox : public Control {
public:
    static const input::RoutedEvent SelectedIndexChangedEvent;

    /// <summary>
    /// 建構預設清單方塊控制項。
    /// </summary>
    ListBox();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~ListBox() override = default;

    /// <summary>
    /// 於清單末端新增一個項目。
    /// </summary>
    /// <param name="item">項目文字視圖。</param>
    void AddItem(compat::string_view item);

    /// <summary>
    /// 於指定索引位置插入一個項目。
    /// </summary>
    /// <param name="index">欲插入之索引位置。</param>
    /// <param name="item">項目文字視圖。</param>
    /// <exception cref="std::out_of_range">當 index 大於項目總數時擲出。</exception>
    void InsertItem(size_t index, compat::string_view item);

    /// <summary>
    /// 移除指定索引位置之項目。
    /// </summary>
    /// <param name="index">欲移除之索引位置。</param>
    /// <exception cref="std::out_of_range">當 index 大於或等於項目總數時擲出。</exception>
    void RemoveItem(size_t index);

    /// <summary>
    /// 清空所有清單項目。
    /// </summary>
    void ClearItems();

    /// <summary>
    /// 取得目前清單之項目總數。
    /// </summary>
    /// <returns>項目數量。</returns>
    COMPAT_NODISCARD size_t GetItemCount() const noexcept { return m_items.size(); }

    /// <summary>
    /// 取得指定索引之項目文字。
    /// </summary>
    /// <param name="index">項目索引。</param>
    /// <returns>項目字串視圖。</returns>
    /// <exception cref="std::out_of_range">當 index 超出範圍時擲出。</exception>
    COMPAT_NODISCARD compat::string_view GetItem(size_t index) const;

    /// <summary>
    /// 取得目前選取項目之索引 (-1 表示無選取)。
    /// </summary>
    /// <returns>選取索引。</returns>
    COMPAT_NODISCARD int32_t GetSelectedIndex() const noexcept { return m_selectedIndex; }

    /// <summary>
    /// 設定目前選取項目之索引 (-1 表示清除選取)。
    /// </summary>
    /// <param name="index">欲選取之索引。</param>
    /// <exception cref="std::out_of_range">當 index 小於 -1 或大於等於項目總數時擲出。</exception>
    void SetSelectedIndex(int32_t index);

    /// <summary>
    /// 取得目前選取之項目文字 (若未選取回傳空字串)。
    /// </summary>
    /// <returns>選取文字視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetSelectedItem() const noexcept;

    /// <summary>
    /// 依文字內容搜尋並選取第一個相符項目 (找不到則設為 -1)。
    /// </summary>
    /// <param name="item">欲選取之項目文字。</param>
    void SetSelectedItem(compat::string_view item);

    /// <summary>
    /// 取得單一項目之顯示高度 (像素)。
    /// </summary>
    /// <returns>高度數值。</returns>
    COMPAT_NODISCARD float GetItemHeight() const noexcept { return m_itemHeight; }

    /// <summary>
    /// 設定單一項目之顯示高度 (像素)。
    /// </summary>
    /// <param name="height">高度數值，必須大於 0.0f。</param>
    /// <exception cref="std::invalid_argument">當 height 小於或等於 0.0f 時擲出。</exception>
    void SetItemHeight(float height);

    /// <summary>
    /// 查詢是否啟用視區虛擬化排版 (Virtualization)。
    /// </summary>
    /// <returns>若啟用傳回 true。</returns>
    COMPAT_NODISCARD bool IsVirtualizationEnabled() const noexcept { return m_virtualizationEnabled; }

    /// <summary>
    /// 設定是否啟用視區虛擬化排版。
    /// </summary>
    /// <param name="enable">true 啟用虛擬化 (支援 10 萬筆數據)，false 完整繪製。</param>
    void SetVirtualizationEnabled(bool enable);

    /// <summary>
    /// 取得當前垂直滾動偏移量 (像素)。
    /// </summary>
    /// <returns>滾動偏移值。</returns>
    COMPAT_NODISCARD float GetScrollOffset() const noexcept { return m_scrollOffset; }

    /// <summary>
    /// 設定垂直滾動偏移量。
    /// </summary>
    /// <param name="offset">滾動偏移值。</param>
    void SetScrollOffset(float offset);

    /// <summary>
    /// 將指定索引之項目滾動至可視區域內。
    /// </summary>
    /// <param name="index">項目索引。</param>
    void ScrollIntoView(int32_t index);

    /// <summary>
    /// 註冊選取索引變更事件處理常式。
    /// </summary>
    /// <param name="handler">回呼常式，接收最新選取索引。</param>
    void OnSelectedIndexChanged(std::function<void(int32_t)> handler);

    /// <summary>
    /// 排版測量階段，計算清單期望尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 繪製清單背景、邊框與各項目文字/選取高亮。
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
    void RaiseSelectedIndexChanged();

    std::vector<std::string> m_items;
    int32_t                  m_selectedIndex = -1;
    float                    m_itemHeight = 24.0f;
    float                    m_scrollOffset = 0.0f;
    bool                     m_virtualizationEnabled = true;
    core::Color              m_selectionColor;
    core::Color              m_selectionTextColor;
};

} // namespace controls
} // namespace cppui
