/*
 * File: SkiaImageBridge.cpp
 * Purpose: Implements zero-copy Skia integration and accelerated blur when CPPUI_HAS_SKIA is active.
 */

#include "ui/graphics/SkiaImageBridge.hpp"

#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkImageFilters.h"
#endif

namespace cppui {
namespace graphics {

bool SkiaImageBridge::IsSkiaAvailable() noexcept {
#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    return true;
#else
    return false;
#endif
}

bool SkiaImageBridge::TryGaussianBlur(Bitmap& bmp, float sigmaX, float sigmaY) {
#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    const uint32_t width = bmp.GetWidth();
    const uint32_t height = bmp.GetHeight();
    if (width == 0 || height == 0 || sigmaX <= 0.0f) return false;
    if (sigmaY <= 0.0f) sigmaY = sigmaX;

    auto pixelSpan = bmp.GetPixelSpanMut();
    uint32_t* rawPixels = pixelSpan.data();
    if (!rawPixels) return false;

    // 零拷貝以 SkBitmap 安裝現有連續記憶體緩衝區
    SkImageInfo info = SkImageInfo::Make(static_cast<int>(width), static_cast<int>(height),
                                         kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap srcBitmap;
    if (!srcBitmap.installPixels(info, rawPixels, bmp.GetStride())) {
        return false;
    }

    sk_sp<SkImage> srcImage = srcBitmap.asImage();
    if (!srcImage) return false;

    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    if (!surface) return false;

    SkCanvas* canvas = surface->getCanvas();
    if (!canvas) return false;

    SkPaint paint;
    paint.setImageFilter(SkImageFilters::Blur(sigmaX, sigmaY, nullptr));
    canvas->drawImage(srcImage, 0.0f, 0.0f, SkSamplingOptions(), &paint);

    // 讀回至原始像素緩衝區
    if (!surface->readPixels(info, rawPixels, bmp.GetStride(), 0, 0)) {
        return false;
    }

    return true;
#else
    (void)bmp;
    (void)sigmaX;
    (void)sigmaY;
    return false;
#endif
}

} // namespace graphics
} // namespace cppui
