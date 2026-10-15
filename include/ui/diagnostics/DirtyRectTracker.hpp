/**
 * @file DirtyRectTracker.hpp
 * @brief 局部重繪區域追蹤器與 FPS 統計疊加層宣告
 */

#pragma once

#include <vector>
#include <chrono>
#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/diagnostics/DiagnosticsOptions.hpp"
#include "ui/rendering/DrawingContext.hpp"

namespace cppui {
namespace diagnostics {

/// <summary>
/// 局部重繪區域追蹤器 (DirtyRectTracker)
/// 追蹤所有無效化邊界並在幀繪製末端疊加半透明閃爍高亮與 FPS 統計。
/// </summary>
class DirtyRectTracker {
public:
    /// <summary>
    /// 取得 DirtyRectTracker 單例。
    /// </summary>
    /// <returns>單例引用。</returns>
    static DirtyRectTracker& Get() noexcept;

    /// <summary>
    /// 設定全域診斷選項。
    /// </summary>
    /// <param name="options">診斷選項。</param>
    void SetOptions(const DiagnosticsOptions& options);

    /// <summary>
    /// 取得全域診斷選項。
    /// </summary>
    /// <returns>診斷選項常數引用。</returns>
    COMPAT_NODISCARD const DiagnosticsOptions& GetOptions() const noexcept { return m_options; }

    /// <summary>
    /// 記錄重繪區域。
    /// </summary>
    /// <param name="rect">無效化矩形區域。</param>
    void RecordDirtyRect(const core::Rect& rect);

    /// <summary>
    /// 清除所有歷史記錄之重繪區域。
    /// </summary>
    void ClearDirtyRects();

    /// <summary>
    /// 取得記錄的重繪區域數量。
    /// </summary>
    /// <returns>矩形數量。</returns>
    COMPAT_NODISCARD size_t GetDirtyRectCount() const noexcept;

    /// <summary>
    /// 於繪圖上下文渲染診斷覆蓋層 (含重繪高亮框與 FPS 計數器)。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    /// <param name="windowWidth">視窗寬度。</param>
    /// <param name="windowHeight">視窗高度。</param>
    void RenderOverlay(rendering::DrawingContext& dc, float windowWidth, float windowHeight);

    /// <summary>
    /// 更新幀率統計。
    /// </summary>
    /// <param name="frameDeltaSeconds">單幀經過秒數。</param>
    void UpdateFps(float frameDeltaSeconds);

    /// <summary>
    /// 取得當前每秒幀數 (FPS)。
    /// </summary>
    /// <returns>目前 FPS 估算值。</returns>
    COMPAT_NODISCARD float GetCurrentFps() const noexcept { return m_currentFps; }

private:
    DirtyRectTracker();
    ~DirtyRectTracker() = default;

    DirtyRectTracker(const DirtyRectTracker&) = delete;
    DirtyRectTracker& operator=(const DirtyRectTracker&) = delete;

    struct TimedRect {
        core::Rect rect;
        std::chrono::steady_clock::time_point timestamp;
    };

    DiagnosticsOptions m_options;
    std::vector<TimedRect> m_dirtyRects;
    float m_currentFps = 60.0f;
    float m_fpsAccumulator = 0.0f;
    uint32_t m_frameCount = 0;
};

} // namespace diagnostics
} // namespace cppui
