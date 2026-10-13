/*
 * File: Popup.cpp
 * Purpose: Implements the Popup floating control with smart placement engine.
 */

#include "ui/controls/Popup.hpp"
#include "ui/controls/OverlayLayer.hpp"
#include "ui/input/InputManager.hpp"
#include <algorithm>
#include <stdexcept>

namespace cppui {
namespace controls {

Popup::Popup() {
    SetHitTestVisible(true);
    SetVisible(false);
}

Popup::Popup(std::shared_ptr<core::UIElement> content)
    : ContentControl(std::move(content)) {
    SetHitTestVisible(true);
    SetVisible(false);
}

Popup::~Popup() {
    if (m_isOpen) {
        SetIsOpen(false);
    }
}

void Popup::SetPlacementTarget(core::UIElement* target) {
    m_placementTarget = target;
    if (m_isOpen) {
        UpdatePlacement();
    }
}

void Popup::SetPlacementMode(PlacementMode mode) noexcept {
    m_placementMode = mode;
    if (m_isOpen) {
        UpdatePlacement();
    }
}

void Popup::SetOffset(core::Point offset) noexcept {
    m_offset = offset;
    if (m_isOpen) {
        UpdatePlacement();
    }
}

void Popup::SetHorizontalOffset(float x) noexcept {
    m_offset.x = x;
    if (m_isOpen) {
        UpdatePlacement();
    }
}

void Popup::SetVerticalOffset(float y) noexcept {
    m_offset.y = y;
    if (m_isOpen) {
        UpdatePlacement();
    }
}

void Popup::SetIsOpen(bool value) {
    if (m_isOpen == value) return;

    if (value) {
        m_isOpen = true;
        SetVisible(true);

        OverlayLayer* layer = nullptr;
        if (m_placementTarget) {
            layer = OverlayLayer::GetOverlayLayer(m_placementTarget);
        }
        if (!layer) {
            layer = OverlayLayer::GetOverlayLayer(this);
        }

        if (layer) {
            m_currentOverlayLayer = layer;
            layer->AddFloatingElement(std::static_pointer_cast<core::UIElement>(shared_from_this()));
        }

        UpdatePlacement();
        input::InputManager::Get().RegisterOpenPopup(this);

        OnOpened();
        for (const auto& handler : m_openedHandlers) {
            if (handler) {
                handler(*this);
            }
        }
    } else {
        m_isOpen = false;
        SetVisible(false);

        input::InputManager::Get().UnregisterOpenPopup(this);

        if (m_currentOverlayLayer) {
            m_currentOverlayLayer->RemoveFloatingElement(std::static_pointer_cast<core::UIElement>(shared_from_this()));
            m_currentOverlayLayer = nullptr;
        }

        OnClosed();
        for (const auto& handler : m_closedHandlers) {
            if (handler) {
                handler(*this);
            }
        }
    }
}

void Popup::UpdatePlacement() {
    Measure(core::Size(10000.0f, 10000.0f));
    core::Size size = GetDesiredSize();
    if (size.width <= 0.0f) {
        size.width = (GetWidth() > 0.0f) ? GetWidth() : 120.0f;
    }
    if (size.height <= 0.0f) {
        size.height = (GetHeight() > 0.0f) ? GetHeight() : 36.0f;
    }

    core::Size windowSize(800.0f, 600.0f);
    if (m_currentOverlayLayer && m_currentOverlayLayer->GetRenderBounds().width > 0.0f) {
        windowSize = core::Size(m_currentOverlayLayer->GetRenderBounds().width, m_currentOverlayLayer->GetRenderBounds().height);
    }

    core::Rect targetRect(0.0f, 0.0f, 0.0f, 0.0f);
    if (m_placementTarget) {
        core::Point p = m_placementTarget->PointToScreen(core::Point(0.0f, 0.0f));
        core::Rect tb = m_placementTarget->GetRenderBounds();
        float targetW = tb.width > 0.0f ? tb.width : m_placementTarget->GetWidth();
        float targetH = tb.height > 0.0f ? tb.height : m_placementTarget->GetHeight();
        targetRect = core::Rect(p.x, p.y, targetW, targetH);
    }

    float candidateX = targetRect.x + m_offset.x;
    float candidateY = targetRect.y + m_offset.y;

    switch (m_placementMode) {
        case PlacementMode::Bottom: {
            candidateX = targetRect.x + m_offset.x;
            candidateY = targetRect.y + targetRect.height + m_offset.y;
            // 邊界溢出自動翻轉：若下方超出且上方有足夠空間，翻轉至上方
            if (candidateY + size.height > windowSize.height && targetRect.y - size.height >= 0.0f) {
                candidateY = targetRect.y - size.height - m_offset.y;
            }
            break;
        }

        case PlacementMode::Top: {
            candidateX = targetRect.x + m_offset.x;
            candidateY = targetRect.y - size.height - m_offset.y;
            // 邊界溢出自動翻轉：若上方超出且下方有空間，翻轉至下方
            if (candidateY < 0.0f && targetRect.y + targetRect.height + size.height <= windowSize.height) {
                candidateY = targetRect.y + targetRect.height + m_offset.y;
            }
            break;
        }

        case PlacementMode::Right: {
            candidateX = targetRect.x + targetRect.width + m_offset.x;
            candidateY = targetRect.y + m_offset.y;
            // 邊界溢出自動翻轉：若右方超出且左方有空間，翻轉至左方
            if (candidateX + size.width > windowSize.width && targetRect.x - size.width >= 0.0f) {
                candidateX = targetRect.x - size.width - m_offset.x;
            }
            break;
        }

        case PlacementMode::Left: {
            candidateX = targetRect.x - size.width - m_offset.x;
            candidateY = targetRect.y + m_offset.y;
            // 邊界溢出自動翻轉：若左方超出且右方有空間，翻轉至右方
            if (candidateX < 0.0f && targetRect.x + targetRect.width + size.width <= windowSize.width) {
                candidateX = targetRect.x + targetRect.width + m_offset.x;
            }
            break;
        }

        case PlacementMode::Mouse:
        case PlacementMode::Custom:
        default: {
            candidateX = targetRect.x + m_offset.x;
            candidateY = targetRect.y + m_offset.y;
            break;
        }
    }

    // 邊界限制 Clamp：防止超出視窗畫面
    candidateX = compat::clamp(candidateX, 0.0f, (std::max)(0.0f, windowSize.width - size.width));
    candidateY = compat::clamp(candidateY, 0.0f, (std::max)(0.0f, windowSize.height - size.height));

    SetVisualOffset(core::Point(candidateX, candidateY));
    Arrange(core::Rect(core::Point(candidateX, candidateY), size));
}

void Popup::AddOpenedHandler(PopupEventHandler handler) {
    if (handler) {
        m_openedHandlers.push_back(std::move(handler));
    }
}

void Popup::AddClosedHandler(PopupEventHandler handler) {
    if (handler) {
        m_closedHandlers.push_back(std::move(handler));
    }
}

void Popup::OnOpened() {}

void Popup::OnClosed() {}

} // namespace controls
} // namespace cppui
