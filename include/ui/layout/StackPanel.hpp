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
    explicit StackPanel(Orientation orientation = Orientation::Vertical)
        : m_orientation(orientation) {}
    ~StackPanel() override = default;

    COMPAT_NODISCARD Orientation GetOrientation() const noexcept { return m_orientation; }
    void SetOrientation(Orientation orientation);

    COMPAT_NODISCARD float GetSpacing() const noexcept { return m_spacing; }
    void SetSpacing(float spacing);

    core::Size MeasureOverride(core::Size availableSize) override;
    void ArrangeOverride(core::Size finalSize) override;

private:
    Orientation m_orientation = Orientation::Vertical;
    float       m_spacing = 0.0f;
};

} // namespace layout
} // namespace cppui
