/**
 * @file DesignerHitTest.cpp
 * @brief 設計期節點與八向選取控制點命中測試器實作
 */

#include "ui/designer/surface/DesignerHitTest.hpp"

namespace cppui {
namespace designer {
namespace surface {

HandleType DesignerHitTest::HitTestHandle(
    const core::Rect& bounds,
    core::Point point,
    float handleSize,
    float hitTolerance
) {
    if (bounds.IsEmpty()) {
        return HandleType::None;
    }

    const HandleType allHandles[] = {
        HandleType::TopLeft,
        HandleType::TopCenter,
        HandleType::TopRight,
        HandleType::MiddleLeft,
        HandleType::MiddleRight,
        HandleType::BottomLeft,
        HandleType::BottomCenter,
        HandleType::BottomRight
    };

    for (HandleType h : allHandles) {
        core::Rect r = GetHandleRect(bounds, h, handleSize);
        core::Rect expanded(
            r.x - hitTolerance,
            r.y - hitTolerance,
            r.width + 2.0f * hitTolerance,
            r.height + 2.0f * hitTolerance
        );

        if (expanded.Contains(point)) {
            return h;
        }
    }

    return HandleType::None;
}

WidgetNode* DesignerHitTest::HitTestNode(
    WidgetNode* root,
    const std::map<std::string, core::Rect>& boundsMap,
    core::Point point
) {
    if (!root) {
        return nullptr;
    }

    // 由後往前 (最上層子元素優先檢驗)
    for (size_t i = root->GetChildCount(); i > 0; --i) {
        WidgetNode* child = root->GetChild(i - 1);
        if (WidgetNode* hit = HitTestNode(child, boundsMap, point)) {
            return hit;
        }
    }

    // 檢驗自身邊界
    core::Rect selfBounds;
    auto it = boundsMap.find(root->GetUuid());
    if (it != boundsMap.end()) {
        selfBounds = it->second;
    } else {
        selfBounds = EstimateNodeBounds(*root);
    }

    if (selfBounds.Contains(point)) {
        return root;
    }

    return nullptr;
}

const WidgetNode* DesignerHitTest::HitTestNode(
    const WidgetNode* root,
    const std::map<std::string, core::Rect>& boundsMap,
    core::Point point
) {
    return HitTestNode(const_cast<WidgetNode*>(root), boundsMap, point);
}

core::Rect DesignerHitTest::EstimateNodeBounds(const WidgetNode& node) {
    float x = 0.0f;
    float y = 0.0f;
    float w = 100.0f;
    float h = 30.0f;

    auto wProp = node.GetProperty("Width");
    if (wProp && wProp->Is<double>()) {
        w = static_cast<float>(wProp->Get<double>().value());
    }

    auto hProp = node.GetProperty("Height");
    if (hProp && hProp->Is<double>()) {
        h = static_cast<float>(hProp->Get<double>().value());
    }

    auto leftAtt = node.GetAttachedProperty("Canvas.Left");
    if (leftAtt && leftAtt->Is<double>()) {
        x = static_cast<float>(leftAtt->Get<double>().value());
    }

    auto topAtt = node.GetAttachedProperty("Canvas.Top");
    if (topAtt && topAtt->Is<double>()) {
        y = static_cast<float>(topAtt->Get<double>().value());
    }

    return core::Rect(x, y, w, h);
}

} // namespace surface
} // namespace designer
} // namespace cppui
