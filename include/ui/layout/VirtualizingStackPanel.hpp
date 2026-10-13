/*
 * File: VirtualizingStackPanel.hpp
 * Purpose: Declares VirtualizingStackPanel layout panel with container recycling and viewport virtualization.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include "compat/compat.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/LayoutTypes.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 虛擬化線性堆疊排版容器 (VirtualizingStackPanel)
/// 僅為當前視區 (Viewport) 內之項目建立與排列 UI 容器，
/// 並透過 Recycling Pool (回收池) 重用離開視區之容器節點，達成 O(1) 記憶體與常數時間複雜度。
/// </summary>
class VirtualizingStackPanel : public Panel {
public:
    using ContainerGenerator = std::function<std::shared_ptr<core::UIElement>(size_t index)>;
    using ContainerRecycler  = std::function<void(size_t index, core::UIElement* container)>;

    /// <summary>
    /// 建構虛擬化堆疊容器。
    /// </summary>
    /// <param name="orientation">排列方向，預設垂直。</param>
    explicit VirtualizingStackPanel(Orientation orientation = Orientation::Vertical);

    /// <summary>虛擬解構子。</summary>
    ~VirtualizingStackPanel() override = default;

    /// <summary>取得排列方向。</summary>
    /// <returns>Orientation 列舉值。</returns>
    COMPAT_NODISCARD Orientation GetOrientation() const noexcept { return m_orientation; }

    /// <summary>設定排列方向。</summary>
    /// <param name="orientation">Orientation 列舉值。</param>
    void SetOrientation(Orientation orientation);

    /// <summary>取得資料項目總數量。</summary>
    /// <returns>項目數量。</returns>
    COMPAT_NODISCARD size_t GetItemCount() const noexcept { return m_itemCount; }

    /// <summary>設定資料項目總數量。</summary>
    /// <param name="count">項目總數。</param>
    void SetItemCount(size_t count);

    /// <summary>取得預設固定項目長度 (垂直為高度，水平為寬度)。</summary>
    /// <returns>長度像素值。</returns>
    COMPAT_NODISCARD float GetItemExtent() const noexcept { return m_itemExtent; }

    /// <summary>設定預設固定項目長度。</summary>
    /// <param name="extent">長度像素值。</param>
    void SetItemExtent(float extent);

    /// <summary>取得項目高度 (GetItemExtent 便捷別名)。</summary>
    COMPAT_NODISCARD float GetItemHeight() const noexcept { return GetItemExtent(); }

    /// <summary>設定項目高度 (SetItemExtent 便捷別名)。</summary>
    void SetItemHeight(float height) { SetItemExtent(height); }

    /// <summary>設定容器產生器 (當回收池無可用節點時呼叫)。</summary>
    /// <param name="generator">產生器回呼委派。</param>
    void SetContainerGenerator(ContainerGenerator generator);

    /// <summary>設定容器重用資料綁定回呼 (當節點被重用綁定新索引時呼叫)。</summary>
    /// <param name="recycler">回收綁定委派。</param>
    void SetContainerRecycler(ContainerRecycler recycler);

    /// <summary>更新當前視區可視範圍與滾動偏移量。</summary>
    /// <param name="scrollOffset">滾動位移 (像素)。</param>
    /// <param name="viewportSize">可見視區尺寸 (像素)。</param>
    void UpdateViewport(float scrollOffset, float viewportSize);

    /// <summary>取得當前回收池中閒置之容器數量。</summary>
    /// <returns>回收池容器數。</returns>
    COMPAT_NODISCARD size_t GetRecycledContainerCount() const noexcept { return m_recycledPool.size(); }

    /// <summary>取得回收池中閒置之容器數量 (GetRecycledContainerCount 便捷別名)。</summary>
    COMPAT_NODISCARD size_t GetRecycledPoolCount() const noexcept { return GetRecycledContainerCount(); }

    /// <summary>取得當前視區內活躍掛載之容器數量。</summary>
    /// <returns>活躍容器數。</returns>
    COMPAT_NODISCARD size_t GetActiveContainerCount() const noexcept { return m_activeContainers.size(); }

    /// <summary>取得當前視區內第一個可見項目的索引。</summary>
    /// <returns>索引值。</returns>
    COMPAT_NODISCARD size_t GetFirstVisibleIndex() const noexcept { return m_firstVisibleIndex; }

    /// <summary>取得當前視區內最後一個可見項目的索引。</summary>
    /// <returns>索引值。</returns>
    COMPAT_NODISCARD size_t GetLastVisibleIndex() const noexcept { return m_lastVisibleIndex; }

    /// <summary>取得總延伸長度 (Extent)。</summary>
    /// <returns>總長度像素值。</returns>
    COMPAT_NODISCARD float GetTotalExtent() const noexcept { return static_cast<float>(m_itemCount) * m_itemExtent; }

    /// <summary>重設並清空所有容器與回收池快取。</summary>
    void ResetContainers();

    /// <summary>清空回收池與活躍容器 (ResetContainers 便捷別名)。</summary>
    void ClearPool() { ResetContainers(); }

protected:
    /// <summary>排版度量階段。</summary>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>排版排列階段。</summary>
    void ArrangeOverride(core::Size finalSize) override;

private:
    void RealizeContainers();

    Orientation m_orientation = Orientation::Vertical;
    size_t m_itemCount = 0;
    float m_itemExtent = 24.0f;
    float m_scrollOffset = 0.0f;
    float m_viewportSize = 0.0f;

    size_t m_firstVisibleIndex = 0;
    size_t m_lastVisibleIndex = 0;

    ContainerGenerator m_generator;
    ContainerRecycler  m_recycler;

    std::vector<std::shared_ptr<core::UIElement>> m_recycledPool;
    std::unordered_map<size_t, std::shared_ptr<core::UIElement>> m_activeContainers;
};

} // namespace layout
} // namespace cppui
