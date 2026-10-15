/*
 * File: LayoutManager.cpp
 * Purpose: Implements centralized layout coordinator with oscillation loop defense.
 */

#include "ui/layout/LayoutManager.hpp"

namespace cppui {
namespace layout {

LayoutManager& LayoutManager::Get() {
    static LayoutManager instance;
    return instance;
}

void LayoutManager::UpdateLayout(core::UIElement& root, core::Size rootSize) {
    uint32_t iteration = 0;
    do {
        const bool wasMeasureDirty = !root.IsMeasureValid();
        root.Measure(rootSize);
        if (wasMeasureDirty) {
            m_stats.totalMeasures++;
        }

        const bool wasArrangeDirty = !root.IsArrangeValid();
        root.Arrange(core::Rect(0.0f, 0.0f, rootSize.width, rootSize.height));
        if (wasArrangeDirty) {
            m_stats.totalArranges++;
        }
        ++iteration;
    } while ((!root.IsMeasureValid() || !root.IsArrangeValid()) && iteration < m_maxIterations);

    if (iteration >= m_maxIterations && (!root.IsMeasureValid() || !root.IsArrangeValid())) {
        m_stats.totalOscillations++;
        compat::println("Warning: Layout oscillation detected in root element after reaching iteration limit ({})", m_maxIterations);
    }
}

} // namespace layout
} // namespace cppui
