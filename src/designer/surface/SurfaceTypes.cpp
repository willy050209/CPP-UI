/**
 * @file SurfaceTypes.cpp
 * @brief CPPUI Designer 設計表面基礎型別與幾何 Handle 計算實作
 */

#include "ui/designer/surface/SurfaceTypes.hpp"

namespace cppui {
namespace designer {
namespace surface {

controls::CursorType GetCursorForHandle(HandleType handle) {
    switch (handle) {
        case HandleType::TopLeft:
        case HandleType::BottomRight:
            return controls::CursorType::SizeNWSE;
        case HandleType::TopRight:
        case HandleType::BottomLeft:
            return controls::CursorType::SizeNESW;
        case HandleType::TopCenter:
        case HandleType::BottomCenter:
            return controls::CursorType::SizeNS;
        case HandleType::MiddleLeft:
        case HandleType::MiddleRight:
            return controls::CursorType::SizeWE;
        default:
            return controls::CursorType::Default;
    }
}

core::Rect GetHandleRect(const core::Rect& bounds, HandleType handle, float size) {
    float half = size * 0.5f;
    float cx = bounds.x + bounds.width * 0.5f;
    float cy = bounds.y + bounds.height * 0.5f;

    switch (handle) {
        case HandleType::TopLeft:
            return core::Rect(bounds.x - half, bounds.y - half, size, size);
        case HandleType::TopCenter:
            return core::Rect(cx - half, bounds.y - half, size, size);
        case HandleType::TopRight:
            return core::Rect(bounds.x + bounds.width - half, bounds.y - half, size, size);
        case HandleType::MiddleLeft:
            return core::Rect(bounds.x - half, cy - half, size, size);
        case HandleType::MiddleRight:
            return core::Rect(bounds.x + bounds.width - half, cy - half, size, size);
        case HandleType::BottomLeft:
            return core::Rect(bounds.x - half, bounds.y + bounds.height - half, size, size);
        case HandleType::BottomCenter:
            return core::Rect(cx - half, bounds.y + bounds.height - half, size, size);
        case HandleType::BottomRight:
            return core::Rect(bounds.x + bounds.width - half, bounds.y + bounds.height - half, size, size);
        default:
            return core::Rect(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

} // namespace surface
} // namespace designer
} // namespace cppui
