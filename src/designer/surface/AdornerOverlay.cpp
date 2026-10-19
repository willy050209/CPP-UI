/**
 * @file AdornerOverlay.cpp
 * @brief 設計表面飾條層繪製器實作
 */

#include "ui/designer/surface/AdornerOverlay.hpp"

namespace cppui {
namespace designer {
namespace surface {

void AdornerOverlay::RenderGrid(
    rendering::DrawingContext& dc,
    const core::Size& surfaceSize,
    float gridSize,
    core::Color gridColor
) {
    if (gridSize <= 2.0f || gridColor.a == 0) {
        return;
    }

    for (float x = gridSize; x < surfaceSize.width; x += gridSize) {
        for (float y = gridSize; y < surfaceSize.height; y += gridSize) {
            dc.DrawRectangle(core::Rect(x, y, 1.0f, 1.0f), gridColor);
        }
    }
}

void AdornerOverlay::RenderSelection(
    rendering::DrawingContext& dc,
    const core::Rect& bounds,
    const SurfaceOptions& options,
    bool showHandles
) {
    if (bounds.IsEmpty()) {
        return;
    }

    // 1. 半透明填充背景
    if (options.selectionFillColor.a > 0) {
        dc.DrawRectangle(bounds, options.selectionFillColor);
    }

    // 2. 邊框四邊線條
    float left = bounds.x;
    float top = bounds.y;
    float right = bounds.x + bounds.width;
    float bottom = bounds.y + bounds.height;

    dc.DrawLine(core::Point(left, top), core::Point(right, top), options.selectionBorderColor, 1.5f);
    dc.DrawLine(core::Point(right, top), core::Point(right, bottom), options.selectionBorderColor, 1.5f);
    dc.DrawLine(core::Point(right, bottom), core::Point(left, bottom), options.selectionBorderColor, 1.5f);
    dc.DrawLine(core::Point(left, bottom), core::Point(left, top), options.selectionBorderColor, 1.5f);

    // 3. 八向控制點方塊
    if (showHandles) {
        const HandleType handles[] = {
            HandleType::TopLeft,
            HandleType::TopCenter,
            HandleType::TopRight,
            HandleType::MiddleLeft,
            HandleType::MiddleRight,
            HandleType::BottomLeft,
            HandleType::BottomCenter,
            HandleType::BottomRight
        };

        for (HandleType h : handles) {
            core::Rect hr = GetHandleRect(bounds, h, options.handleSize);
            dc.DrawRectangle(hr, options.handleColor);

            float hl = hr.x;
            float ht = hr.y;
            float hrgt = hr.x + hr.width;
            float hb = hr.y + hr.height;

            dc.DrawLine(core::Point(hl, ht), core::Point(hrgt, ht), options.handleBorderColor, 1.0f);
            dc.DrawLine(core::Point(hrgt, ht), core::Point(hrgt, hb), options.handleBorderColor, 1.0f);
            dc.DrawLine(core::Point(hrgt, hb), core::Point(hl, hb), options.handleBorderColor, 1.0f);
            dc.DrawLine(core::Point(hl, hb), core::Point(hl, ht), options.handleBorderColor, 1.0f);
        }
    }
}

void AdornerOverlay::RenderSnapGuides(
    rendering::DrawingContext& dc,
    compat::span<const SnapLine> guides
) {
    for (const auto& g : guides) {
        if (g.orientation == SnapOrientation::Vertical) {
            dc.DrawLine(core::Point(g.position, g.start), core::Point(g.position, g.end), g.color, 1.0f);
        } else {
            dc.DrawLine(core::Point(g.start, g.position), core::Point(g.end, g.position), g.color, 1.0f);
        }
    }
}

void AdornerOverlay::RenderRubberBand(
    rendering::DrawingContext& dc,
    const core::Rect& rect,
    core::Color borderColor,
    core::Color fillColor
) {
    if (rect.IsEmpty()) {
        return;
    }

    if (fillColor.a > 0) {
        dc.DrawRectangle(rect, fillColor);
    }

    float l = rect.x;
    float t = rect.y;
    float r = rect.x + rect.width;
    float b = rect.y + rect.height;

    dc.DrawLine(core::Point(l, t), core::Point(r, t), borderColor, 1.0f);
    dc.DrawLine(core::Point(r, t), core::Point(r, b), borderColor, 1.0f);
    dc.DrawLine(core::Point(r, b), core::Point(l, b), borderColor, 1.0f);
    dc.DrawLine(core::Point(l, b), core::Point(l, t), borderColor, 1.0f);
}

void AdornerOverlay::RenderInsertionIndicator(
    rendering::DrawingContext& dc,
    core::Point p1,
    core::Point p2,
    core::Color color
) {
    dc.DrawLine(p1, p2, color, 2.0f);
}

} // namespace surface
} // namespace designer
} // namespace cppui
