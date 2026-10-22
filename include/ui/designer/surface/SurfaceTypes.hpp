/**
 * @file SurfaceTypes.hpp
 * @brief CPPUI Designer 設計表面基礎型別、控制點列舉、吸附導引線與組態選項
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/controls/Control.hpp"

namespace cppui {
namespace designer {
namespace surface {

/// <summary>
/// 八向選取縮放控制點列舉 (HandleType)
/// </summary>
enum class HandleType : uint8_t {
    None = 0,
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

/// <summary>
/// 畫布滑鼠拖曳互動模式列舉 (DragMode)
/// </summary>
enum class DragMode : uint8_t {
    None = 0,
    Move,
    Resize,
    RubberBand,
    Create,
    ScrollH,
    ScrollV
};

/// <summary>
/// 吸附導引線方向列舉 (SnapOrientation)
/// </summary>
enum class SnapOrientation : uint8_t {
    Horizontal, // 水平線 (Y 軸固定)
    Vertical    // 垂直線 (X 軸固定)
};

/// <summary>
/// 智慧吸附導引線描述項 (SnapLine)
/// </summary>
struct SnapLine {
    SnapOrientation orientation = SnapOrientation::Horizontal;
    float position = 0.0f; // 垂直線之 X 座標，或水平線之 Y 座標
    float start = 0.0f;    // 線段起點座標 (DIP)
    float end = 0.0f;      // 線段終點座標 (DIP)
    core::Color color{ 224, 17, 95, 255 }; // 預設洋紅 (Magenta)

    COMPAT_NODISCARD bool operator==(const SnapLine& other) const noexcept {
        return orientation == other.orientation &&
               position == other.position &&
               start == other.start &&
               end == other.end &&
               color == other.color;
    }
};

/// <summary>
/// 畫布表面選項組態 (SurfaceOptions)
/// </summary>
struct SurfaceOptions {
    float gridSize = 8.0f;
    bool snapToGrid = true;
    bool snapToGuides = true;
    float snapThreshold = 6.0f;
    bool showGrid = true;
    core::Color gridColor{ 200, 200, 200, 60 };
    core::Color selectionBorderColor{ 0, 120, 215, 255 };
    core::Color selectionFillColor{ 0, 0, 0, 0 };
    core::Color handleColor{ 255, 255, 255, 255 };
    core::Color handleBorderColor{ 0, 120, 215, 255 };
    float handleSize = 6.0f;
};

/// <summary>
/// 依據八向 HandleType 取得對應之滑鼠游標樣式。
/// </summary>
/// <param name="handle">控制點類型。</param>
/// <returns>CursorType 游標列舉值。</returns>
COMPAT_NODISCARD CPPUI_API controls::CursorType GetCursorForHandle(HandleType handle);

/// <summary>
/// 計算指定控制點在給定元件邊界矩形下的 Handle 幾何小方塊。
/// </summary>
/// <param name="bounds">目標元件邊界矩形。</param>
/// <param name="handle">控制點類型。</param>
/// <param name="size">Handle 方塊邊長 (像素)。</param>
/// <returns>Handle 所在之 Rect 邊界。</returns>
COMPAT_NODISCARD CPPUI_API core::Rect GetHandleRect(const core::Rect& bounds, HandleType handle, float size = 6.0f);

} // namespace surface
} // namespace designer
} // namespace cppui
