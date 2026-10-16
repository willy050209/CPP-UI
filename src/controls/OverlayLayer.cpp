/*
 * File: OverlayLayer.cpp
 * Purpose: Implements the OverlayLayer floating child container.
 */

#include "ui/controls/OverlayLayer.hpp"
#include "ui/app/Form.hpp"
#include <stdexcept>
#include <algorithm>

namespace cppui {
namespace controls {

OverlayLayer::OverlayLayer() {
    SetHitTestVisible(true);
}

void OverlayLayer::AddFloatingElement(std::shared_ptr<core::UIElement> element) {
    if (!element) {
        throw std::invalid_argument("OverlayLayer::AddFloatingElement: element cannot be nullptr.");
    }
    AddChild(std::move(element));
}

void OverlayLayer::RemoveFloatingElement(const std::shared_ptr<core::UIElement>& element) {
    if (!element) {
        throw std::invalid_argument("OverlayLayer::RemoveFloatingElement: element cannot be nullptr.");
    }
    RemoveChild(element);
}

void OverlayLayer::RemoveFloatingElement(core::UIElement* element) {
    if (!element) {
        throw std::invalid_argument("OverlayLayer::RemoveFloatingElement: element cannot be nullptr.");
    }
    RemoveChild(element);
}

size_t OverlayLayer::GetFloatingElementCount() const noexcept {
    return GetChildrenCount();
}

core::Visual* OverlayLayer::HitTest(core::Point localPoint) {
    if (!m_isVisible || m_opacity <= 0.0f) {
        return nullptr;
    }

    core::Rect localBounds(0.0f, 0.0f, m_visualBounds.width, m_visualBounds.height);
    if (!localBounds.Contains(localPoint)) {
        return nullptr;
    }

    // 依反向 Z-Order 優先測試最上層浮動子元件
    for (size_t i = m_children.size(); i > 0; --i) {
        auto& child = m_children[i - 1];
        if (!child || !child->IsVisible()) continue;

        core::Point childLocal(
            localPoint.x - child->GetVisualOffset().x,
            localPoint.y - child->GetVisualOffset().y
        );
        core::Visual* hit = child->HitTest(childLocal);
        if (hit) {
            return hit;
        }
    }

    // 若未命中任何浮動子元件，回傳 nullptr 以穿透至下方底層內容
    return nullptr;
}

OverlayLayer* OverlayLayer::GetOverlayLayer(core::UIElement* element) {
    if (!element) return nullptr;

    core::Visual* cur = element;
    while (cur) {
        if (auto* overlay = dynamic_cast<OverlayLayer*>(cur)) {
            return overlay;
        }
        if (auto* form = dynamic_cast<app::Form*>(cur)) {
            return form->GetOverlayLayer();
        }
        cur = cur->GetVisualParent();
    }
    return nullptr;
}

core::Size OverlayLayer::MeasureOverride(core::Size availableSize) {
    for (size_t i = 0; i < m_children.size(); ++i) {
        if (m_children[i] && m_children[i]->IsVisible()) {
            m_children[i]->Measure(availableSize);
        }
    }
    return availableSize;
}

void OverlayLayer::ArrangeOverride(core::Size finalSize) {
    (void)finalSize;
    for (size_t i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (child && child->IsVisible()) {
            core::Size desired = child->GetDesiredSize();
            child->Arrange(core::Rect(child->GetVisualOffset(), desired));
        }
    }
}

} // namespace controls
} // namespace cppui
