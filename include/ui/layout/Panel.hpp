#pragma once

#include <vector>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/UIElement.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 多子節點版面容器基底類別 (Panel)
/// 提供子元件集合管理，並與底層 Visual Tree 維護同步。
/// </summary>
class Panel : public core::UIElement {
public:
    Panel() = default;
    ~Panel() override = default;

    void AddChild(std::shared_ptr<core::UIElement> child);
    void RemoveChild(const std::shared_ptr<core::UIElement>& child);
    void ClearChildren();

    COMPAT_NODISCARD size_t GetChildrenCount() const noexcept { return m_children.size(); }
    COMPAT_NODISCARD core::UIElement* GetChild(size_t index) const;
    COMPAT_NODISCARD const std::vector<std::shared_ptr<core::UIElement>>& GetChildren() const noexcept { return m_children; }

protected:
    std::vector<std::shared_ptr<core::UIElement>> m_children;
};

} // namespace layout
} // namespace cppui
