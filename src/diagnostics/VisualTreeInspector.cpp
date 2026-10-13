/**
 * @file VisualTreeInspector.cpp
 * @brief 視覺樹診斷器與 F12 DevTools 控制項檢查工具實作
 */

#include "ui/diagnostics/VisualTreeInspector.hpp"
#include "ui/diagnostics/DirtyRectTracker.hpp"
#include "ui/text/Font.hpp"
#include <cstdio>
#include <algorithm>
#include <typeinfo>

namespace cppui {
namespace diagnostics {

VisualTreeInspector& VisualTreeInspector::Get() noexcept {
    static VisualTreeInspector s_instance;
    return s_instance;
}

VisualTreeInspector::VisualTreeInspector() {
    input::InputManager::Get().SetInspectorHook(this);
}

VisualTreeInspector::~VisualTreeInspector() {
    if (input::InputManager::Get().GetInspectorHook() == this) {
        input::InputManager::Get().SetInspectorHook(nullptr);
    }
}

void VisualTreeInspector::OnMouseMove(core::UIElement* hitUI) {
    SetHoveredElement(hitUI);
}

void VisualTreeInspector::OnMouseDown(core::UIElement* hitUI) {
    InspectElement(hitUI);
    EnablePickerMode(false);
}

void VisualTreeInspector::OnF12Pressed(core::UIElement& root) {
    if (DirtyRectTracker::Get().GetOptions().enableVisualTreeInspector) {
        Toggle();
        root.InvalidateVisual();
    }
}

void VisualTreeInspector::SetVisible(bool visible) {
    m_visible = visible;
    if (!m_visible) {
        m_pickerMode = false;
        m_hoveredElement = nullptr;
    }
}

void VisualTreeInspector::Toggle() {
    SetVisible(!m_visible);
}

void VisualTreeInspector::EnablePickerMode(bool enabled) {
    m_pickerMode = enabled;
    if (!enabled) {
        m_hoveredElement = nullptr;
    }
}

void VisualTreeInspector::SetHoveredElement(core::UIElement* element) {
    m_hoveredElement = element;
}

void VisualTreeInspector::InspectElement(core::UIElement* element) {
    m_inspectedElement = element;
}

void VisualTreeInspector::TraverseVisualTree(core::UIElement* node, uint32_t depth, std::vector<VisualNodeInfo>& out) const {
    if (!node) return;

    VisualNodeInfo info;
    info.element = node;
    info.typeName = typeid(*node).name();
    info.bounds = node->GetVisualBounds();
    info.depth = depth;
    out.push_back(info);

    size_t childCount = node->GetVisualChildrenCount();
    for (size_t i = 0; i < childCount; ++i) {
        auto* childVisual = node->GetVisualChild(i);
        auto* childElement = dynamic_cast<core::UIElement*>(childVisual);
        if (childElement) {
            TraverseVisualTree(childElement, depth + 1, out);
        }
    }
}

std::vector<VisualNodeInfo> VisualTreeInspector::BuildVisualTree(core::UIElement* root) const {
    std::vector<VisualNodeInfo> tree;
    if (root) {
        TraverseVisualTree(root, 0, tree);
    }
    return tree;
}

void VisualTreeInspector::RenderOverlay(rendering::DrawingContext& dc, float windowWidth, float windowHeight) {
    if (!m_visible && !m_pickerMode) return;
    if (windowWidth <= 0.0f || windowHeight <= 0.0f) return;

    text::Font hudFont("Segoe UI", 12.0f);
    text::Font headerFont("Segoe UI", 13.0f, text::FontWeight::Bold);

    // 1. 若處於元素吸管拾取模式，繪製懸停元素的高亮邊框與度量浮標
    if (m_pickerMode && m_hoveredElement) {
        core::Rect bounds = m_hoveredElement->GetVisualBounds();
        if (bounds.width > 0.0f && bounds.height > 0.0f) {
            dc.DrawRectangle(bounds, core::Color(0, 160, 240, 45));
            dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.Right(), bounds.y), core::Color(0, 160, 240, 230), 2.0f);
            dc.DrawLine(core::Point(bounds.x, bounds.Bottom()), core::Point(bounds.Right(), bounds.Bottom()), core::Color(0, 160, 240, 230), 2.0f);
            dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.x, bounds.Bottom()), core::Color(0, 160, 240, 230), 2.0f);
            dc.DrawLine(core::Point(bounds.Right(), bounds.y), core::Point(bounds.Right(), bounds.Bottom()), core::Color(0, 160, 240, 230), 2.0f);

            // 標示微型資訊浮標
            char tipBuf[128];
            std::snprintf(tipBuf, sizeof(tipBuf), "%.1f x %.1f", static_cast<double>(bounds.width), static_cast<double>(bounds.height));
            float tipY = (std::max)(4.0f, bounds.y - 20.0f);
            core::Rect tipRect(bounds.x, tipY, 90.0f, 18.0f);
            dc.DrawRoundedRectangle(tipRect, 3.0f, core::Color(0, 120, 215, 220));
            dc.DrawText(tipBuf, core::Point(bounds.x + 6.0f, tipY + 2.0f), hudFont, core::Color(255, 255, 255, 255));
        }
    }

    // 2. 若有選中之元素，繪製藍色固態外框
    if (m_inspectedElement) {
        core::Rect bounds = m_inspectedElement->GetVisualBounds();
        if (bounds.width > 0.0f && bounds.height > 0.0f) {
            dc.DrawRectangle(bounds, core::Color(0, 120, 215, 30));
            dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.Right(), bounds.y), core::Color(0, 120, 215, 255), 2.0f);
            dc.DrawLine(core::Point(bounds.x, bounds.Bottom()), core::Point(bounds.Right(), bounds.Bottom()), core::Color(0, 120, 215, 255), 2.0f);
            dc.DrawLine(core::Point(bounds.x, bounds.y), core::Point(bounds.x, bounds.Bottom()), core::Color(0, 120, 215, 255), 2.0f);
            dc.DrawLine(core::Point(bounds.Right(), bounds.y), core::Point(bounds.Right(), bounds.Bottom()), core::Color(0, 120, 215, 255), 2.0f);
        }
    }

    // 3. 若處於可見模式，繪製 DevTools 診斷面板
    if (m_visible) {
        float panelW = 280.0f;
        float panelH = 160.0f;
        float panelX = (std::max)(10.0f, windowWidth - panelW - 16.0f);
        float panelY = (std::max)(10.0f, windowHeight - panelH - 16.0f);

        core::Rect panelRect(panelX, panelY, panelW, panelH);
        dc.DrawRoundedRectangle(panelRect, 6.0f, core::Color(30, 30, 36, 235));
        dc.DrawLine(core::Point(panelX, panelY + 28.0f), core::Point(panelX + panelW, panelY + 28.0f), core::Color(80, 80, 90, 200), 1.0f);

        // 標題列
        dc.DrawText("DevTools | Visual Tree Inspector [F12]", core::Point(panelX + 10.0f, panelY + 6.0f), headerFont, core::Color(240, 240, 245, 255));

        // 狀態資訊
        float lineY = panelY + 36.0f;
        std::string pickerStatus = m_pickerMode ? "Picker Mode: ACTIVE (Click to select)" : "Picker Mode: OFF (Press F12 to inspect)";
        dc.DrawText(pickerStatus, core::Point(panelX + 10.0f, lineY), hudFont, m_pickerMode ? core::Color(100, 220, 255, 255) : core::Color(180, 180, 180, 255));
        lineY += 22.0f;

        if (m_inspectedElement) {
            std::string typeName = typeid(*m_inspectedElement).name();
            // 縮短型別名稱
            size_t colon = typeName.rfind(':');
            if (colon != std::string::npos) {
                typeName = typeName.substr(colon + 1);
            }
            char elemBuf[128];
            std::snprintf(elemBuf, sizeof(elemBuf), "Selected: %s", typeName.c_str());
            dc.DrawText(elemBuf, core::Point(panelX + 10.0f, lineY), hudFont, core::Color(255, 215, 0, 255));
            lineY += 20.0f;

            core::Rect b = m_inspectedElement->GetVisualBounds();
            char boundBuf[128];
            std::snprintf(boundBuf, sizeof(boundBuf), "Bounds: [%.1f, %.1f] %.1f x %.1f",
                          static_cast<double>(b.x), static_cast<double>(b.y),
                          static_cast<double>(b.width), static_cast<double>(b.height));
            dc.DrawText(boundBuf, core::Point(panelX + 10.0f, lineY), hudFont, core::Color(220, 220, 225, 255));
            lineY += 20.0f;

            core::Size actualSize = m_inspectedElement->GetActualSize();
            char sizeBuf[128];
            std::snprintf(sizeBuf, sizeof(sizeBuf), "ActualSize: %.1f x %.1f",
                          static_cast<double>(actualSize.width), static_cast<double>(actualSize.height));
            dc.DrawText(sizeBuf, core::Point(panelX + 10.0f, lineY), hudFont, core::Color(180, 220, 180, 255));
        } else {
            dc.DrawText("Selected: None (Use picker to select element)", core::Point(panelX + 10.0f, lineY), hudFont, core::Color(160, 160, 170, 255));
        }
    }
}

} // namespace diagnostics
} // namespace cppui
