/*
 * File: VirtualizingStackPanel.cpp
 * Purpose: Implements VirtualizingStackPanel layout panel.
 */

#include "ui/layout/VirtualizingStackPanel.hpp"
#include <cmath>
#include <algorithm>

namespace cppui {
namespace layout {

VirtualizingStackPanel::VirtualizingStackPanel(Orientation orientation)
    : m_orientation(orientation) {}

void VirtualizingStackPanel::SetOrientation(Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        InvalidateMeasure();
        InvalidateArrange();
    }
}

void VirtualizingStackPanel::SetItemCount(size_t count) {
    if (m_itemCount != count) {
        m_itemCount = count;
        InvalidateMeasure();
        InvalidateArrange();
    }
}

void VirtualizingStackPanel::SetItemExtent(float extent) {
    if (extent > 0.0f && m_itemExtent != extent) {
        m_itemExtent = extent;
        InvalidateMeasure();
        InvalidateArrange();
    }
}

void VirtualizingStackPanel::SetContainerGenerator(ContainerGenerator generator) {
    m_generator = std::move(generator);
    ResetContainers();
}

void VirtualizingStackPanel::SetContainerRecycler(ContainerRecycler recycler) {
    m_recycler = std::move(recycler);
}

void VirtualizingStackPanel::UpdateViewport(float scrollOffset, float viewportSize) {
    m_scrollOffset = (std::max)(0.0f, scrollOffset);
    m_viewportSize = (std::max)(0.0f, viewportSize);
    RealizeContainers();
    InvalidateMeasure();
    InvalidateArrange();
}

void VirtualizingStackPanel::ResetContainers() {
    ClearChildren();
    m_activeContainers.clear();
    m_recycledPool.clear();
    InvalidateMeasure();
    InvalidateArrange();
}

void VirtualizingStackPanel::RealizeContainers() {
    if (m_itemCount == 0 || m_itemExtent <= 0.0f) {
        for (auto& pair : m_activeContainers) {
            RemoveChild(pair.second);
            m_recycledPool.push_back(pair.second);
        }
        m_activeContainers.clear();
        m_firstVisibleIndex = 0;
        m_lastVisibleIndex = 0;
        return;
    }

    float effectiveVp = (m_viewportSize > 0.0f) ? m_viewportSize : 600.0f;
    float maxOffset = (std::max)(0.0f, static_cast<float>(m_itemCount) * m_itemExtent - effectiveVp);
    float clampedOffset = compat::clamp(m_scrollOffset, 0.0f, maxOffset);

    int64_t rawFirst = static_cast<int64_t>(clampedOffset / m_itemExtent);
    int64_t buffer = 2; // 上下多緩衝 2 個項目，滾動時零閃爍
    size_t first = static_cast<size_t>((std::max)(int64_t{0}, rawFirst - buffer));
    int64_t visibleItems = static_cast<int64_t>(std::ceil(effectiveVp / m_itemExtent)) + 1;
    size_t last = static_cast<size_t>((std::min)(static_cast<int64_t>(m_itemCount - 1), rawFirst + visibleItems + buffer));

    m_firstVisibleIndex = first;
    m_lastVisibleIndex = last;

    // 1. 回收超出當前視區的舊容器
    std::vector<size_t> toRemove;
    for (auto& pair : m_activeContainers) {
        size_t idx = pair.first;
        if (idx < first || idx > last) {
            toRemove.push_back(idx);
        }
    }
    for (size_t idx : toRemove) {
        auto container = m_activeContainers[idx];
        m_activeContainers.erase(idx);
        RemoveChild(container);
        m_recycledPool.push_back(std::move(container));
    }

    // 2. 為新進入可見範圍的索引產生或重用容器
    for (size_t idx = first; idx <= last; ++idx) {
        if (m_activeContainers.find(idx) != m_activeContainers.end()) {
            continue; // 已存在於視區
        }

        std::shared_ptr<core::UIElement> container;
        if (!m_recycledPool.empty()) {
            container = m_recycledPool.back();
            m_recycledPool.pop_back();
            if (m_recycler) {
                m_recycler(idx, container.get());
            }
        } else if (m_generator) {
            container = m_generator(idx);
        }

        if (container) {
            AddChild(container);
            m_activeContainers[idx] = std::move(container);
        }
    }
}

core::Size VirtualizingStackPanel::MeasureOverride(core::Size availableSize) {
    RealizeContainers();

    if (m_orientation == Orientation::Vertical) {
        core::Size itemConstraint(availableSize.width, m_itemExtent);
        for (auto& pair : m_activeContainers) {
            if (pair.second) {
                pair.second->Measure(itemConstraint);
            }
        }
        float totalH = static_cast<float>(m_itemCount) * m_itemExtent;
        return core::Size((availableSize.width > 0.0f && availableSize.width < 1e9f) ? availableSize.width : 120.0f, (std::max)(24.0f, totalH));
    } else {
        core::Size itemConstraint(m_itemExtent, availableSize.height);
        for (auto& pair : m_activeContainers) {
            if (pair.second) {
                pair.second->Measure(itemConstraint);
            }
        }
        float totalW = static_cast<float>(m_itemCount) * m_itemExtent;
        return core::Size((std::max)(24.0f, totalW), (availableSize.height > 0.0f && availableSize.height < 1e9f) ? availableSize.height : 30.0f);
    }
}

void VirtualizingStackPanel::ArrangeOverride(core::Size finalSize) {
    for (auto& pair : m_activeContainers) {
        size_t idx = pair.first;
        auto& container = pair.second;
        if (!container) continue;

        float pos = static_cast<float>(idx) * m_itemExtent;
        if (m_orientation == Orientation::Vertical) {
            container->Arrange(core::Rect(0.0f, pos, finalSize.width, m_itemExtent));
        } else {
            container->Arrange(core::Rect(pos, 0.0f, m_itemExtent, finalSize.height));
        }
    }
}

} // namespace layout
} // namespace cppui
