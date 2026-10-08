/**
 * @file DisplayListRenderer.hpp
 * @brief DisplayList 指令重播器，維護狀態棧並將繪圖指令派發至後端 RenderTarget
 */

#pragma once

#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"

namespace cppui {
namespace rendering {

/// <summary>
/// 渲染統計資訊
/// </summary>
struct RenderStats {
    size_t totalOps = 0;
    size_t renderedOps = 0;
    size_t culledOps = 0;
};

/// <summary>
/// DisplayList 命令重播器 (DisplayListRenderer)
/// 遍歷 Command Stream，維護 Clip Stack 與 Opacity Stack，執行圖元剪裁並分發至 RenderTarget。
/// </summary>
class DisplayListRenderer {
public:
    /// <summary>預設建構子。</summary>
    DisplayListRenderer() = default;

    /// <summary>預設解構子。</summary>
    ~DisplayListRenderer() = default;

    /// <summary>
    /// 重播 DisplayList 指令至目標畫布。
    /// </summary>
    /// <param name="displayList">待重播之 DisplayList 引用。</param>
    /// <param name="target">目標渲染目標介面指標。</param>
    /// <returns>RenderStats 渲染統計結構。</returns>
    RenderStats Render(const DisplayList& displayList, IRenderTarget* target);
};

} // namespace rendering
} // namespace cppui
