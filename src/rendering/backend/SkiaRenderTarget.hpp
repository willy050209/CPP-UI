#pragma once

#include "ui/rendering/backend/IRenderContext.hpp"

#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include <vector>
#include <utility>

namespace cppui {
namespace rendering {

/// <summary>
/// 基於 Google Skia 向量畫布之渲染目標 (SkiaRenderTarget)
/// 封裝 SkSurface 與 SkCanvas，提供高速向量圖元繪製與像素讀取
/// </summary>
class SkiaRenderTarget : public IRenderTarget {
public:
    explicit SkiaRenderTarget(sk_sp<SkSurface> surface)
        : m_surface(std::move(surface)) {
        if (m_surface) {
            m_canvas = m_surface->getCanvas();
            m_width = static_cast<uint32_t>(m_surface->width());
            m_height = static_cast<uint32_t>(m_surface->height());
            m_pixels.resize(static_cast<size_t>(m_width) * m_height, 0);
        }
    }

    uint32_t GetWidth() const noexcept override { return m_width; }
    uint32_t GetHeight() const noexcept override { return m_height; }

    void Clear(core::Color color) override {
        if (m_canvas) {
            m_canvas->clear(SkColorSetARGB(color.a, color.r, color.g, color.b));
        }
    }

    void FillRect(const core::Rect& rect, core::Color color) override {
        if (m_canvas) {
            SkPaint paint;
            paint.setColor(SkColorSetARGB(color.a, color.r, color.g, color.b));
            paint.setStyle(SkPaint::kFill_Style);
            paint.setAntiAlias(true);
            m_canvas->drawRect(SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height), paint);
        }
    }

    uint32_t* GetPixelBuffer() noexcept override {
        SyncPixels();
        return m_pixels.data();
    }

    void* GetSkCanvas() noexcept override {
        return static_cast<void*>(m_canvas);
    }

    SkCanvas* GetCanvas() const noexcept {
        return m_canvas;
    }

    sk_sp<SkSurface> GetSurface() const noexcept {
        return m_surface;
    }

    void SyncPixels() {
        if (m_surface && !m_pixels.empty()) {
            SkImageInfo info = SkImageInfo::Make(m_width, m_height, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
            m_surface->readPixels(info, m_pixels.data(), static_cast<size_t>(m_width) * sizeof(uint32_t), 0, 0);
        }
    }

private:
    sk_sp<SkSurface> m_surface;
    SkCanvas* m_canvas = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<uint32_t> m_pixels;
};

} // namespace rendering
} // namespace cppui

#endif // CPPUI_HAS_SKIA
