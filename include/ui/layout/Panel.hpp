/*
 * File: Panel.hpp
 * Purpose: Declares Panel, the base class for layout panels that manage multiple child elements.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/UIElement.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 多子節點版面容器基底類別 (Panel)
/// 提供子元件集合管理，並與底層 Visual Tree 維護同步。
/// </summary>
class Panel : public core::UIElement {
public:
    /// <summary>
    /// 預設建構 Panel 實例。
    /// </summary>
    Panel() = default;

    /// <summary>
    /// 虛擬解構 Panel 實例。
    /// </summary>
    ~Panel() override = default;

    /// <summary>
    /// 新增子元件至版面容器中。
    /// </summary>
    /// <param name="child">待加入之子元件智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 child 為 nullptr 時擲出。</exception>
    void AddChild(std::shared_ptr<core::UIElement> child);

    /// <summary>
    /// 從版面容器中移除指定子元件。
    /// </summary>
    /// <param name="child">待移除之子元件智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 child 為 nullptr 時擲出。</exception>
    void RemoveChild(const std::shared_ptr<core::UIElement>& child);

    /// <summary>
    /// 依原始指標從版面容器中移除指定子元件 (解構安全)。
    /// </summary>
    /// <param name="child">待移除之子元件裸指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 child 為 nullptr 時擲出。</exception>
    void RemoveChild(core::UIElement* child);

    /// <summary>
    /// 清除所有子元件。
    /// </summary>
    void ClearChildren();

    /// <summary>
    /// 取得子元件總數。
    /// </summary>
    /// <returns>子元件數量。</returns>
    COMPAT_NODISCARD size_t GetChildrenCount() const noexcept { return m_children.size(); }

    /// <summary>
    /// 依索引取得子元件指標。
    /// </summary>
    /// <param name="index">子元件索引。</param>
    /// <returns>子元件指標，若超出範圍傳回 nullptr。</returns>
    COMPAT_NODISCARD core::UIElement* GetChild(size_t index) const;

    /// <summary>
    /// 取得所有子元件清單。
    /// </summary>
    /// <returns>子元件 vector 引用。</returns>
    COMPAT_NODISCARD const std::vector<std::shared_ptr<core::UIElement>>& GetChildren() const noexcept { return m_children; }

protected:
    std::vector<std::shared_ptr<core::UIElement>> m_children;
};

} // namespace layout
} // namespace cppui
