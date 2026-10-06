#pragma once

#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"

namespace cppui::rendering {

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
    DisplayListRenderer() = default;
    ~DisplayListRenderer() = default;

    /// <summary>
    /// 重播 DisplayList 指令至目標畫布
    /// </summary>
    RenderStats Render(const DisplayList& displayList, IRenderTarget* target);
};

} // namespace cppui::rendering
