#include "ui/layout/Panel.hpp"

namespace cppui {
namespace layout {

void Panel::AddChild(std::shared_ptr<core::UIElement> child) {
    if (!child || child.get() == this) return;

    AddVisualChild(child);
    m_children.push_back(std::move(child));
    InvalidateMeasure();
}

void Panel::RemoveChild(const std::shared_ptr<core::UIElement>& child) {
    if (!child) return;

    RemoveVisualChild(child);
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == child.get()) {
            m_children.erase(it);
            InvalidateMeasure();
            return;
        }
    }
}

void Panel::ClearChildren() {
    ClearVisualChildren();
    m_children.clear();
    InvalidateMeasure();
}

core::UIElement* Panel::GetChild(size_t index) const {
    if (index >= m_children.size()) {
        return nullptr;
    }
    return m_children[index].get();
}

} // namespace layout
} // namespace cppui
