#include "ui/layout/StackPanel.hpp"
#include <algorithm>

namespace cppui {
namespace layout {

void StackPanel::SetOrientation(Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        InvalidateMeasure();
    }
}

void StackPanel::SetSpacing(float spacing) {
    if (m_spacing != spacing) {
        m_spacing = spacing;
        InvalidateMeasure();
    }
}

core::Size StackPanel::MeasureOverride(core::Size availableSize) {
    float totalW = 0.0f;
    float totalH = 0.0f;
    size_t visibleCount = 0;

    bool isVertical = (m_orientation == Orientation::Vertical);
    core::Size childConstraint = isVertical
        ? core::Size(availableSize.width, 1000000.0f)
        : core::Size(1000000.0f, availableSize.height);

    for (auto& child : m_children) {
        if (!child || child->GetVisibility() == Visibility::Collapsed) {
            continue;
        }

        child->Measure(childConstraint);
        core::Size desired = child->GetDesiredSize();

        if (isVertical) {
            totalH += desired.height;
            totalW = std::max(totalW, desired.width);
        } else {
            totalW += desired.width;
            totalH = std::max(totalH, desired.height);
        }

        visibleCount++;
    }

    if (visibleCount > 1) {
        float totalSpacing = static_cast<float>(visibleCount - 1) * m_spacing;
        if (isVertical) {
            totalH += totalSpacing;
        } else {
            totalW += totalSpacing;
        }
    }

    return core::Size(totalW, totalH);
}

void StackPanel::ArrangeOverride(core::Size finalSize) {
    bool isVertical = (m_orientation == Orientation::Vertical);
    float offset = 0.0f;

    for (auto& child : m_children) {
        if (!child || child->GetVisibility() == Visibility::Collapsed) {
            continue;
        }

        core::Size desired = child->GetDesiredSize();

        if (isVertical) {
            core::Rect childRect(0.0f, offset, finalSize.width, desired.height);
            child->Arrange(childRect);
            offset += desired.height + m_spacing;
        } else {
            core::Rect childRect(offset, 0.0f, desired.width, finalSize.height);
            child->Arrange(childRect);
            offset += desired.width + m_spacing;
        }
    }
}

} // namespace layout
} // namespace cppui
