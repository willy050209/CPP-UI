/**
 * @file AdornerOverlay.hpp
 * @brief 設計表面飾條層 (八向控制框、智慧對齊導引線、框選框與插入指示線) 繪製器
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/designer/surface/SurfaceTypes.hpp"

namespace cppui {
namespace designer {
namespace surface {

/// <summary>
/// 飾條繪製器 (AdornerOverlay)
/// 負責將選取框、Handle 方塊、吸附導引線與框選框錄製至 DrawingContext。
/// </summary>
class CPPUI_API AdornerOverlay {
public:
    /// <summary>
    /// 繪製畫布背景網格 (Grid Dots)。
    /// </summary>
    /// <param name="dc">繪圖錄製上下文。</param>
    /// <param name="surfaceSize">畫布總體尺寸。</param>
    /// <param name="gridSize">網格間距 (DIP)。</param>
    /// <param name="gridColor">網格色彩。</param>
    static void RenderGrid(
        rendering::DrawingContext& dc,
        const core::Size& surfaceSize,
        float gridSize,
        core::Color gridColor
    );

    /// <summary>
    /// 繪製單一或群組選取框與八向 Resize 控制點。
    /// </summary>
    /// <param name="dc">繪圖錄製上下文。</param>
    /// <param name="bounds">選取邊界矩形。</param>
    /// <param name="options">表面選項組態。</param>
    /// <param name="showHandles">是否繪製八向控制柄 (多選時副選項可設為 false)。</param>
    static void RenderSelection(
        rendering::DrawingContext& dc,
        const core::Rect& bounds,
        const SurfaceOptions& options,
        bool showHandles = true
    );

    /// <summary>
    /// 繪製當前啟動之智慧吸附導引線。
    /// </summary>
    /// <param name="dc">繪圖錄製上下文。</param>
    /// <param name="guides">吸附導引線清單。</param>
    static void RenderSnapGuides(
        rendering::DrawingContext& dc,
        compat::span<const SnapLine> guides
    );

    /// <summary>
    /// 繪製滑鼠拉出之矩形框選框 (Rubber-band selection box)。
    /// </summary>
    /// <param name="dc">繪圖錄製上下文。</param>
    /// <param name="rect">框選矩形。</param>
    /// <param name="borderColor">邊框色彩。</param>
    /// <param name="fillColor">填色色彩。</param>
    static void RenderRubberBand(
        rendering::DrawingContext& dc,
        const core::Rect& rect,
        core::Color borderColor,
        core::Color fillColor
    );

    /// <summary>
    /// 繪製容器插入點之導引線 (Drop Insertion Indicator)。
    /// </summary>
    /// <param name="dc">繪圖錄製上下文。</param>
    /// <param name="p1">插入指示線起點。</param>
    /// <param name="p2">插入指示線終點。</param>
    /// <param name="color">指示線色彩。</param>
    static void RenderInsertionIndicator(
        rendering::DrawingContext& dc,
        core::Point p1,
        core::Point p2,
        core::Color color
    );
};

} // namespace surface
} // namespace designer
} // namespace cppui
