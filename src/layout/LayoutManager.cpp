#include "ui/layout/LayoutManager.hpp"

namespace cppui {
namespace layout {

LayoutManager& LayoutManager::Get() {
    static LayoutManager instance;
    return instance;
}

void LayoutManager::UpdateLayout(core::UIElement& root, core::Size rootSize) {
    bool wasMeasureDirty = !root.IsMeasureValid();
    root.Measure(rootSize);
    if (wasMeasureDirty) {
        m_stats.totalMeasures++;
    }

    bool wasArrangeDirty = !root.IsArrangeValid();
    root.Arrange(core::Rect(0.0f, 0.0f, rootSize.width, rootSize.height));
    if (wasArrangeDirty) {
        m_stats.totalArranges++;
    }
}

} // namespace layout
} // namespace cppui
