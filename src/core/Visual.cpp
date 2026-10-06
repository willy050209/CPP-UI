#include "ui/core/Visual.hpp"

namespace cppui {
namespace core {

Visual* Visual::GetVisualChild(size_t index) const {
    if (index >= m_visualChildren.size()) {
        return nullptr;
    }
    return m_visualChildren[index].get();
}

void Visual::AddVisualChild(std::shared_ptr<Visual> child) {
    if (!child || child.get() == this) return;

    // 若子節點已有其他父節點，先解綁
    if (child->m_visualParent && child->m_visualParent != this) {
        // 從舊父節點移除
        auto oldParent = child->m_visualParent;
        for (auto it = oldParent->m_visualChildren.begin(); it != oldParent->m_visualChildren.end(); ++it) {
            if (it->get() == child.get()) {
                oldParent->m_visualChildren.erase(it);
                break;
            }
        }
    }

    child->SetVisualParent(this);
    m_visualChildren.push_back(std::move(child));
}

void Visual::RemoveVisualChild(const std::shared_ptr<Visual>& child) {
    if (!child) return;

    for (auto it = m_visualChildren.begin(); it != m_visualChildren.end(); ++it) {
        if (it->get() == child.get()) {
            child->SetVisualParent(nullptr);
            m_visualChildren.erase(it);
            return;
        }
    }
}

void Visual::ClearVisualChildren() {
    for (auto& child : m_visualChildren) {
        if (child) {
            child->SetVisualParent(nullptr);
        }
    }
    m_visualChildren.clear();
}

Point Visual::PointToScreen(Point clientPoint) const {
    Point p = clientPoint;
    const Visual* cur = this;
    while (cur) {
        p.x += cur->m_visualOffset.x;
        p.y += cur->m_visualOffset.y;
        cur = cur->m_visualParent;
    }
    return p;
}

Point Visual::PointToClient(Point screenPoint) const {
    Point rootOffset = PointToScreen(Point(0.0f, 0.0f));
    return Point(screenPoint.x - rootOffset.x, screenPoint.y - rootOffset.y);
}

Point Visual::TransformToAncestor(Point p, const Visual* ancestor) const {
    Point pt = p;
    const Visual* cur = this;
    while (cur && cur != ancestor) {
        pt.x += cur->m_visualOffset.x;
        pt.y += cur->m_visualOffset.y;
        cur = cur->m_visualParent;
    }
    return pt;
}

Point Visual::TransformFromAncestor(Point p, const Visual* ancestor) const {
    Point originInAncestor = TransformToAncestor(Point(0.0f, 0.0f), ancestor);
    return Point(p.x - originInAncestor.x, p.y - originInAncestor.y);
}

Visual* Visual::HitTest(Point localPoint) {
    if (!m_isVisible || m_opacity <= 0.0f) {
        return nullptr;
    }

    // 檢查點是否落在本節點可視範圍內 (以 local bounds 判斷)
    Rect localBounds(0.0f, 0.0f, m_visualBounds.width, m_visualBounds.height);
    if (!localBounds.Contains(localPoint)) {
        return nullptr;
    }

    // 反向 Z-Order 優先搜尋最上層的子元件
    for (auto it = m_visualChildren.rbegin(); it != m_visualChildren.rend(); ++it) {
        auto& child = *it;
        if (!child || !child->IsVisible()) continue;

        Point childLocal(localPoint.x - child->m_visualOffset.x,
                         localPoint.y - child->m_visualOffset.y);
        Visual* hit = child->HitTest(childLocal);
        if (hit) {
            return hit;
        }
    }

    return this;
}

void Visual::Render(rendering::DrawingContext& dc) {
    if (!m_isVisible || m_opacity <= 0.0f) {
        return;
    }

    bool hasOpacity = (m_opacity < 1.0f);
    if (hasOpacity) {
        dc.PushOpacity(m_opacity);
    }

    if (m_clipToBounds) {
        dc.PushClip(Rect(m_visualOffset.x, m_visualOffset.y, m_visualBounds.width, m_visualBounds.height));
    }

    // 呼叫虛擬繪圖自訂點
    OnRender(dc);

    // 遞迴渲染子節點
    for (auto& child : m_visualChildren) {
        if (child && child->IsVisible()) {
            child->Render(dc);
        }
    }

    if (m_clipToBounds) {
        dc.PopClip();
    }

    if (hasOpacity) {
        dc.PopOpacity();
    }
}

} // namespace core
} // namespace cppui
