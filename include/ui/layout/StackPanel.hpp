/**
 * @file StackPanel.hpp
 * @brief 線性堆疊排版容器，支援水平或垂直軸向排列與子元件間距
 */

#pragma once

#include "ui/layout/Panel.hpp"
#include "ui/layout/LayoutTypes.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 線性堆疊排版容器 (StackPanel)
/// 將子元件依照水平或垂直軸線依序排列，支援元件間距 (Spacing)。
/// </summary>
class StackPanel : public Panel {
public:
    /// <summary>
    /// 建構線性堆疊排版容器。
    /// </summary>
    /// <param name="orientation">排列方向，預設為垂直 (Vertical)。</param>
    explicit StackPanel(Orientation orientation = Orientation::Vertical)
        : m_orientation(orientation) {}

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~StackPanel() override = default;

    /// <summary>
    /// 取得當前堆疊排列方向。
    /// </summary>
    /// <returns>Orientation 列舉值。</returns>
    COMPAT_NODISCARD Orientation GetOrientation() const noexcept { return m_orientation; }

    /// <summary>
    /// 設定堆疊排列方向。
    /// </summary>
    /// <param name="orientation">Orientation 列舉值。</param>
    void SetOrientation(Orientation orientation);

    /// <summary>
    /// 取得子元件之間的間距。
    /// </summary>
    /// <returns>間距像素值。</returns>
    COMPAT_NODISCARD float GetSpacing() const noexcept { return m_spacing; }

    /// <summary>
    /// 設定子元件之間的間距。
    /// </summary>
    /// <param name="spacing">間距像素值。</param>
    void SetSpacing(float spacing);

    /// <summary>
    /// 覆寫排版度量，沿主軸累加各子節點尺寸並求副軸最大尺寸。
    /// </summary>
    /// <param name="availableSize">可用尺寸。</param>
    /// <returns>容器期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 覆寫排版排列，沿主軸依序放置各子元件。
    /// </summary>
    /// <param name="finalSize">實際分配尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

private:
    Orientation m_orientation = Orientation::Vertical;
    float       m_spacing = 0.0f;
};

} // namespace layout
} // namespace cppui
