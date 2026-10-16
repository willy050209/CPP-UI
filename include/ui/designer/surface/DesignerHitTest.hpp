/**
 * @file DesignerHitTest.hpp
 * @brief 設計期節點與八向選取控制點命中測試器
 */

#pragma once

#include <map>
#include <string>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/designer/ast/WidgetNode.hpp"
#include "ui/designer/surface/SurfaceTypes.hpp"

namespace cppui {
namespace designer {
namespace surface {

/// <summary>
/// 設計期命中測試器 (DesignerHitTest)
/// 提供純幾何計算：Handle 控制點命中、多層階層節點命中與邊界運算。
/// </summary>
class CPPUI_API DesignerHitTest {
public:
    /// <summary>
    /// 檢驗目標點是否命中給定元件邊界上的任一 8 向 Handle。
    /// </summary>
    /// <param name="bounds">元件邊界矩形 (畫布座標)。</param>
    /// <param name="point">待檢測點 (畫布座標)。</param>
    /// <param name="handleSize">Handle 方塊大小。</param>
    /// <param name="hitTolerance">命中容差半徑。</param>
    /// <returns>命中的 HandleType，若未命中任何 Handle 回傳 HandleType::None。</returns>
    COMPAT_NODISCARD static HandleType HitTestHandle(
        const core::Rect& bounds,
        core::Point point,
        float handleSize = 6.0f,
        float hitTolerance = 3.0f
    );

    /// <summary>
    /// 由上而下遞迴檢驗命中哪一個 AST WidgetNode。
    /// </summary>
    /// <param name="root">起始走訪之根容器節點。</param>
    /// <param name="boundsMap">節點 UUID 至畫布 Rect 邊界之映射表。</param>
    /// <param name="point">待檢測點 (畫布座標)。</param>
    /// <returns>命中最深層之節點指標，若未命中回傳 nullptr。</returns>
    COMPAT_NODISCARD static WidgetNode* HitTestNode(
        WidgetNode* root,
        const std::map<std::string, core::Rect>& boundsMap,
        core::Point point
    );

    /// <summary>
    /// 常數重載版本：由上而下遞迴檢驗命中哪一個 AST WidgetNode。
    /// </summary>
    COMPAT_NODISCARD static const WidgetNode* HitTestNode(
        const WidgetNode* root,
        const std::map<std::string, core::Rect>& boundsMap,
        core::Point point
    );

    /// <summary>
    /// 依據節點自身屬性 (Width, Height) 與附加屬性 (Canvas.Left, Canvas.Top) 估算基本畫布邊界。
    /// </summary>
    /// <param name="node">目標節點。</param>
    /// <returns>估算的 Rect 邊界。</returns>
    COMPAT_NODISCARD static core::Rect EstimateNodeBounds(const WidgetNode& node);
};

} // namespace surface
} // namespace designer
} // namespace cppui
