#include "SkiaRenderContext.hpp"
#include <cstring>

namespace cppui {
namespace rendering {

SkiaRenderContext::SkiaRenderContext() = default;
SkiaRenderContext::~SkiaRenderContext() {
    Destroy();
}

bool SkiaRenderContext::Init(void* nativeWindowHandle, uint32_t width, uint32_t height) {
    m_nativeWindowHandle = nativeWindowHandle;
    m_width = width;
    m_height = height;

#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    SkImageInfo info = SkImageInfo::Make(
        static_cast<int>(width > 0 ? width : 1),
        static_cast<int>(height > 0 ? height : 1),
        kBGRA_8888_SkColorType,
        kPremul_SkAlphaType
    );
    m_surface = SkSurfaces::Raster(info);
    if (m_surface) {
        m_skiaTarget = std::make_unique<SkiaRenderTarget>(m_surface);
        m_useSkia = true;
        m_fallbackContext.Init(nativeWindowHandle, width, height);
        return true;
    }
#endif

    m_useSkia = false;
    return m_fallbackContext.Init(nativeWindowHandle, width, height);
}

void SkiaRenderContext::Resize(uint32_t width, uint32_t height) {
    if (m_width == width && m_height == height) return;
    m_width = width;
    m_height = height;

#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    if (m_useSkia) {
        SkImageInfo info = SkImageInfo::Make(
            static_cast<int>(width > 0 ? width : 1),
            static_cast<int>(height > 0 ? height : 1),
            kBGRA_8888_SkColorType,
            kPremul_SkAlphaType
        );
        m_surface = SkSurfaces::Raster(info);
        if (m_surface) {
            m_skiaTarget = std::make_unique<SkiaRenderTarget>(m_surface);
        }
    }
#endif

    m_fallbackContext.Resize(width, height);
}

IRenderTarget* SkiaRenderContext::BeginFrame() {
#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    if (m_useSkia && m_skiaTarget) {
        return m_skiaTarget.get();
    }
#endif
    return m_fallbackContext.BeginFrame();
}

void SkiaRenderContext::EndFrame() {
#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    if (m_useSkia && m_skiaTarget) {
        m_skiaTarget->SyncPixels();
        auto* fbTarget = m_fallbackContext.BeginFrame();
        if (fbTarget && fbTarget->GetPixelBuffer() && m_skiaTarget->GetPixelBuffer()) {
            std::memcpy(
                fbTarget->GetPixelBuffer(),
                m_skiaTarget->GetPixelBuffer(),
                static_cast<size_t>(m_width) * m_height * sizeof(uint32_t)
            );
        }
        m_fallbackContext.EndFrame();
        return;
    }
#endif
    m_fallbackContext.EndFrame();
}

void SkiaRenderContext::Destroy() {
#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    m_skiaTarget.reset();
    m_surface.reset();
    m_useSkia = false;
#endif
    m_fallbackContext.Destroy();
    m_width = 0;
    m_height = 0;
    m_nativeWindowHandle = nullptr;
}

} // namespace rendering
} // namespace cppui
