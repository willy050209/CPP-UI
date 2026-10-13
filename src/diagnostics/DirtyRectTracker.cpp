/**
 * @file DirtyRectTracker.cpp
 * @brief 局部重繪區域追蹤器實作
 */

#include "ui/diagnostics/DirtyRectTracker.hpp"
#include "ui/text/Font.hpp"
#include <cstdio>
#include <algorithm>

namespace cppui {
namespace diagnostics {

DirtyRectTracker& DirtyRectTracker::Get() noexcept {
    static DirtyRectTracker s_instance;
    return s_instance;
}

DirtyRectTracker::DirtyRectTracker() = default;

void DirtyRectTracker::SetOptions(const DiagnosticsOptions& options) {
    m_options = options;
    if (!m_options.enableDirtyRectOverlay) {
        m_dirtyRects.clear();
    }
}

void DirtyRectTracker::RecordDirtyRect(const core::Rect& rect) {
    if (!m_options.enableDirtyRectOverlay) return;
    if (rect.width <= 0.0f || rect.height <= 0.0f) return;

    m_dirtyRects.push_back(TimedRect{ rect, std::chrono::steady_clock::now() });
}

void DirtyRectTracker::ClearDirtyRects() {
    m_dirtyRects.clear();
}

size_t DirtyRectTracker::GetDirtyRectCount() const noexcept {
    return m_dirtyRects.size();
}

void DirtyRectTracker::UpdateFps(float frameDeltaSeconds) {
    if (frameDeltaSeconds <= 0.0f) return;
    m_fpsAccumulator += frameDeltaSeconds;
    m_frameCount++;

    if (m_fpsAccumulator >= 0.5f) {
        m_currentFps = static_cast<float>(m_frameCount) / m_fpsAccumulator;
        m_frameCount = 0;
        m_fpsAccumulator = 0.0f;
    }
}

void DirtyRectTracker::RenderOverlay(rendering::DrawingContext& dc, float windowWidth, float windowHeight) {
    if (windowWidth <= 0.0f || windowHeight <= 0.0f) return;

    auto now = std::chrono::steady_clock::now();

    // 1. 繪製局部重繪區域淡出覆蓋層
    if (m_options.enableDirtyRectOverlay && !m_dirtyRects.empty()) {
        std::vector<TimedRect> surviving;
        surviving.reserve(m_dirtyRects.size());

        for (const auto& tr : m_dirtyRects) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - tr.timestamp).count();
            if (elapsed < static_cast<int64_t>(m_options.overlayDurationMs)) {
                surviving.push_back(tr);

                float alphaRatio = 1.0f - static_cast<float>(elapsed) / static_cast<float>(m_options.overlayDurationMs);
                uint8_t fillAlpha = static_cast<uint8_t>(alphaRatio * 60.0f);
                uint8_t strokeAlpha = static_cast<uint8_t>(alphaRatio * 200.0f);

                dc.DrawRectangle(tr.rect, core::Color(255, 60, 60, fillAlpha));
                dc.DrawLine(core::Point(tr.rect.x, tr.rect.y), core::Point(tr.rect.Right(), tr.rect.y), core::Color(255, 30, 30, strokeAlpha), 1.5f);
                dc.DrawLine(core::Point(tr.rect.x, tr.rect.Bottom()), core::Point(tr.rect.Right(), tr.rect.Bottom()), core::Color(255, 30, 30, strokeAlpha), 1.5f);
                dc.DrawLine(core::Point(tr.rect.x, tr.rect.y), core::Point(tr.rect.x, tr.rect.Bottom()), core::Color(255, 30, 30, strokeAlpha), 1.5f);
                dc.DrawLine(core::Point(tr.rect.Right(), tr.rect.y), core::Point(tr.rect.Right(), tr.rect.Bottom()), core::Color(255, 30, 30, strokeAlpha), 1.5f);
            }
        }
        m_dirtyRects = std::move(surviving);
    }

    // 2. 繪製幀率與效能 HUD 懸浮膠囊
    if (m_options.showFpsCounter) {
        float bannerW = 120.0f;
        float bannerH = 26.0f;
        float bannerX = (std::max)(0.0f, windowWidth - bannerW - 12.0f);
        float bannerY = 12.0f;

        core::Rect bannerRect(bannerX, bannerY, bannerW, bannerH);
        dc.DrawRoundedRectangle(bannerRect, 4.0f, core::Color(0, 0, 0, 190));

        char buf[32];
        std::snprintf(buf, sizeof(buf), "FPS: %.1f", static_cast<double>(m_currentFps));
        text::Font font("Segoe UI", 12.0f);
        dc.DrawText(buf, core::Point(bannerX + 12.0f, bannerY + 5.0f), font, core::Color(80, 240, 120, 255));
    }
}

} // namespace diagnostics
} // namespace cppui
