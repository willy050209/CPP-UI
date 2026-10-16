/**
 * @file SnapEngine.hpp
 * @brief 智慧吸附純函數計算引擎 (Snap-to-Grid & Snap-to-Sibling)
 */

#pragma once

#include <vector>
#include <cmath>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/designer/surface/SurfaceTypes.hpp"

namespace cppui {
namespace designer {
namespace surface {

/// <summary>
/// 吸附計算結果結構 (SnapResult)
/// </summary>
struct SnapResult {
    core::Point snappedPos;             // 修正後之目標元件左上角座標
    std::vector<SnapLine> activeGuides; // 觸發並需在畫布繪製之導引線清單
    bool snappedX = false;
    bool snappedY = false;
};

/// <summary>
/// 智慧吸附計算引擎 (SnapEngine)
/// 遵循純函數原則，無任何內部狀態，專注於幾何對齊吸附計算。
/// </summary>
class CPPUI_API SnapEngine {
public:
    /// <summary>
    /// 計算針對網格點之吸附數值。
    /// </summary>
    /// <param name="val">原始數值。</param>
    /// <param name="gridSize">網格間距。</param>
    /// <param name="threshold">吸附臨界閥值。</param>
    /// <returns>若在閥值內回傳吸附後數值，否則回傳原值。</returns>
    COMPAT_NODISCARD static float SnapToGrid(float val, float gridSize, float threshold) noexcept;

    /// <summary>
    /// 針對拖曳目標矩形與兄弟元件清單執行智慧邊緣/中心線與網格吸附計算。
    /// </summary>
    /// <param name="targetBounds">目標元件當前邊界矩形 (DIP)。</param>
    /// <param name="siblingBounds">其他兄弟元件邊界矩形清單。</param>
    /// <param name="options">畫布表面選項組態。</param>
    /// <returns>吸附結果物件 (包含修正後位置與導引線)。</returns>
    COMPAT_NODISCARD static SnapResult CalculateSnap(
        const core::Rect& targetBounds,
        compat::span<const core::Rect> siblingBounds,
        const SurfaceOptions& options
    );
};

} // namespace surface
} // namespace designer
} // namespace cppui
