/**
 * @file SnapEngine.cpp
 * @brief 智慧吸附純函數計算引擎實作
 */

#include "ui/designer/surface/SnapEngine.hpp"
#include <algorithm>
#include <limits>

namespace cppui {
namespace designer {
namespace surface {

float SnapEngine::SnapToGrid(float val, float gridSize, float threshold) noexcept {
    if (gridSize <= 0.0f) {
        return val;
    }
    float rounded = std::round(val / gridSize) * gridSize;
    if (std::abs(val - rounded) <= threshold) {
        return rounded;
    }
    return val;
}

SnapResult SnapEngine::CalculateSnap(
    const core::Rect& targetBounds,
    compat::span<const core::Rect> siblingBounds,
    const SurfaceOptions& options
) {
    SnapResult result;
    result.snappedPos = core::Point(targetBounds.x, targetBounds.y);

    if (options.snapToGuides) {
        // --- 水平 X 軸吸附運算 ---
        float bestDeltaX = options.snapThreshold + 1.0f;
        float chosenGuideX = 0.0f;
        float chosenGuideStartY = 0.0f;
        float chosenGuideEndY = 0.0f;
        bool foundGuideX = false;

        float tLeft = targetBounds.x;
        float tCenter = targetBounds.x + targetBounds.width * 0.5f;
        float tRight = targetBounds.x + targetBounds.width;

        for (const auto& sib : siblingBounds) {
            float sLeft = sib.x;
            float sCenter = sib.x + sib.width * 0.5f;
            float sRight = sib.x + sib.width;

            struct TestPair {
                float target;
                float sibling;
                float guidePos;
            };

            const TestPair pairsX[] = {
                { tLeft,   sLeft,   sLeft },   // 左緣對齊
                { tRight,  sRight,  sRight },  // 右緣對齊
                { tCenter, sCenter, sCenter }, // 中心對齊
                { tLeft,   sRight,  sRight },  // 左靠右
                { tRight,  sLeft,   sLeft }    // 右靠左
            };

            for (const auto& p : pairsX) {
                float delta = p.sibling - p.target;
                if (std::abs(delta) <= options.snapThreshold && std::abs(delta) < std::abs(bestDeltaX)) {
                    bestDeltaX = delta;
                    chosenGuideX = p.guidePos;
                    chosenGuideStartY = std::min(targetBounds.y, sib.y) - 10.0f;
                    chosenGuideEndY = std::max(targetBounds.y + targetBounds.height, sib.y + sib.height) + 10.0f;
                    foundGuideX = true;
                }
            }
        }

        if (foundGuideX) {
            result.snappedPos.x += bestDeltaX;
            result.snappedX = true;

            SnapLine line;
            line.orientation = SnapOrientation::Vertical;
            line.position = chosenGuideX;
            line.start = chosenGuideStartY;
            line.end = chosenGuideEndY;
            result.activeGuides.push_back(line);
        }

        // --- 垂直 Y 軸吸附運算 ---
        float bestDeltaY = options.snapThreshold + 1.0f;
        float chosenGuideY = 0.0f;
        float chosenGuideStartX = 0.0f;
        float chosenGuideEndX = 0.0f;
        bool foundGuideY = false;

        float tTop = targetBounds.y;
        float tCenterY = targetBounds.y + targetBounds.height * 0.5f;
        float tBottom = targetBounds.y + targetBounds.height;

        for (const auto& sib : siblingBounds) {
            float sTop = sib.y;
            float sCenterY = sib.y + sib.height * 0.5f;
            float sBottom = sib.y + sib.height;

            struct TestPair {
                float target;
                float sibling;
                float guidePos;
            };

            const TestPair pairsY[] = {
                { tTop,     sTop,     sTop },     // 頂緣對齊
                { tBottom,  sBottom,  sBottom },  // 底緣對齊
                { tCenterY, sCenterY, sCenterY }, // 中心對齊
                { tTop,     sBottom,  sBottom },  // 頂靠底
                { tBottom,  sTop,     sTop }      // 底靠頂
            };

            for (const auto& p : pairsY) {
                float delta = p.sibling - p.target;
                if (std::abs(delta) <= options.snapThreshold && std::abs(delta) < std::abs(bestDeltaY)) {
                    bestDeltaY = delta;
                    chosenGuideY = p.guidePos;
                    chosenGuideStartX = std::min(targetBounds.x, sib.x) - 10.0f;
                    chosenGuideEndX = std::max(targetBounds.x + targetBounds.width, sib.x + sib.width) + 10.0f;
                    foundGuideY = true;
                }
            }
        }

        if (foundGuideY) {
            result.snappedPos.y += bestDeltaY;
            result.snappedY = true;

            SnapLine line;
            line.orientation = SnapOrientation::Horizontal;
            line.position = chosenGuideY;
            line.start = chosenGuideStartX;
            line.end = chosenGuideEndX;
            result.activeGuides.push_back(line);
        }
    }

    // --- 網格吸附 (若未觸發導引線) ---
    if (!result.snappedX && options.snapToGrid) {
        float gridX = SnapToGrid(result.snappedPos.x, options.gridSize, options.snapThreshold);
        if (gridX != result.snappedPos.x) {
            result.snappedPos.x = gridX;
            result.snappedX = true;
        }
    }

    if (!result.snappedY && options.snapToGrid) {
        float gridY = SnapToGrid(result.snappedPos.y, options.gridSize, options.snapThreshold);
        if (gridY != result.snappedPos.y) {
            result.snappedPos.y = gridY;
            result.snappedY = true;
        }
    }

    return result;
}

} // namespace surface
} // namespace designer
} // namespace cppui
