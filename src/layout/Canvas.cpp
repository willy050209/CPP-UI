#include "ui/layout/Canvas.hpp"
#include <algorithm>

namespace cppui {
namespace layout {

std::unordered_map<const core::UIElement*, float> Canvas::s_leftProperties;
std::unordered_map<const core::UIElement*, float> Canvas::s_topProperties;

void Canvas::SetLeft(core::UIElement& element, float left) {
    s_leftProperties[&element] = left;
    element.InvalidateArrange();
}

float Canvas::GetLeft(const core::UIElement& element) {
    auto it = s_leftProperties.find(&element);
    if (it != s_leftProperties.end()) {
        return it->second;
    }
    return 0.0f;
}

void Canvas::SetTop(core::UIElement& element, float top) {
    s_topProperties[&element] = top;
    element.InvalidateArrange();
}

float Canvas::GetTop(const core::UIElement& element) {
    auto it = s_topProperties.find(&element);
    if (it != s_topProperties.end()) {
        return it->second;
    }
    return 0.0f;
}

void Canvas::SetLeft(const std::shared_ptr<core::UIElement>& element, float left) {
    if (element) {
        SetLeft(*element, left);
    }
}

float Canvas::GetLeft(const std::shared_ptr<core::UIElement>& element) {
    if (element) {
        return GetLeft(*element);
    }
    return 0.0f;
}

void Canvas::SetTop(const std::shared_ptr<core::UIElement>& element, float top) {
    if (element) {
        SetTop(*element, top);
    }
}

float Canvas::GetTop(const std::shared_ptr<core::UIElement>& element) {
    if (element) {
        return GetTop(*element);
    }
    return 0.0f;
}

core::Size Canvas::MeasureOverride(core::Size availableSize) {
    core::Size childConstraint(1000000.0f, 1000000.0f);
    for (const auto& child : m_children) {
        if (child && child->GetVisibility() != Visibility::Collapsed) {
            child->Measure(childConstraint);
        }
    }
    return core::Size(0.0f, 0.0f);
}

void Canvas::ArrangeOverride(core::Size /*finalSize*/) {
    for (const auto& child : m_children) {
        if (!child || child->GetVisibility() == Visibility::Collapsed) {
            continue;
        }
        float left = GetLeft(*child);
        float top = GetTop(*child);
        core::Size desired = child->GetDesiredSize();
        child->Arrange(core::Rect(left, top, desired.width, desired.height));
    }
}

} // namespace layout
} // namespace cppui
