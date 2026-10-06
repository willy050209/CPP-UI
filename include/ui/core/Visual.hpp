#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/rendering/DrawingContext.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 視覺樹最輕量節點基底類別 (Visual)
/// 負責親緣階層維護、局部座標位移 (VisualOffset)、透明度、裁剪與遞迴渲染通道。
/// </summary>
class Visual : public std::enable_shared_from_this<Visual> {
public:
    Visual() = default;
    virtual ~Visual() = default;

    // 禁止拷貝，支援移動
    Visual(const Visual&) = delete;
    Visual& operator=(const Visual&) = delete;
    Visual(Visual&&) noexcept = default;
    Visual& operator=(Visual&&) noexcept = default;

    // --- 親緣關係 ---
    COMPAT_NODISCARD Visual* GetVisualParent() const noexcept { return m_visualParent; }
    COMPAT_NODISCARD size_t GetVisualChildrenCount() const noexcept { return m_visualChildren.size(); }
    COMPAT_NODISCARD Visual* GetVisualChild(size_t index) const;

    void AddVisualChild(std::shared_ptr<Visual> child);
    void RemoveVisualChild(const std::shared_ptr<Visual>& child);
    void ClearVisualChildren();

    // --- 幾何與變換屬性 ---
    COMPAT_NODISCARD Point GetVisualOffset() const noexcept { return m_visualOffset; }
    void SetVisualOffset(Point offset) noexcept { m_visualOffset = offset; }

    COMPAT_NODISCARD Rect GetVisualBounds() const noexcept { return m_visualBounds; }
    void SetVisualBounds(const Rect& bounds) noexcept { m_visualBounds = bounds; }

    COMPAT_NODISCARD float GetOpacity() const noexcept { return m_opacity; }
    void SetOpacity(float opacity) noexcept { m_opacity = compat::clamp(opacity, 0.0f, 1.0f); }

    COMPAT_NODISCARD bool IsVisible() const noexcept { return m_isVisible; }
    void SetVisible(bool visible) noexcept { m_isVisible = visible; }

    COMPAT_NODISCARD bool GetClipToBounds() const noexcept { return m_clipToBounds; }
    void SetClipToBounds(bool clip) noexcept { m_clipToBounds = clip; }

    // --- 座標系變換 ---
    COMPAT_NODISCARD Point PointToClient(Point screenPoint) const;
    COMPAT_NODISCARD Point PointToScreen(Point clientPoint) const;
    COMPAT_NODISCARD Point TransformToAncestor(Point p, const Visual* ancestor) const;
    COMPAT_NODISCARD Point TransformFromAncestor(Point p, const Visual* ancestor) const;

    // --- 命中測試 ---
    COMPAT_NODISCARD virtual Visual* HitTest(Point localPoint);

    // --- 渲染管線 ---
    void Render(rendering::DrawingContext& dc);
    virtual void OnRender(rendering::DrawingContext& dc) { (void)dc; }

protected:
    void SetVisualParent(Visual* parent) noexcept { m_visualParent = parent; }

    Visual*                                m_visualParent = nullptr;
    std::vector<std::shared_ptr<Visual>>   m_visualChildren;
    Point                                  m_visualOffset{ 0.0f, 0.0f };
    Rect                                   m_visualBounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    float                                  m_opacity = 1.0f;
    bool                                   m_isVisible = true;
    bool                                   m_clipToBounds = false;
};

} // namespace core
} // namespace cppui
