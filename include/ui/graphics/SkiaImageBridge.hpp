/*
 * File: SkiaImageBridge.hpp
 * Purpose: Zero-copy Skia bitmap wrapping and hardware-accelerated image filter interoperability.
 */

#pragma once

#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/graphics/Bitmap.hpp"

namespace cppui {
namespace graphics {

/// <summary>
/// Google Skia 向量加速與影像濾鏡橋接器 (SkiaImageBridge)
/// 提供 Bitmap 與 SkBitmap 之零拷貝記憶體共享，並封裝 SkImageFilters 硬體濾鏡加速。
/// </summary>
class SkiaImageBridge {
public:
    /// <summary>
    /// 檢查當前建置環境是否具備原生 Skia 支援。
    /// </summary>
    /// <returns>若啟用且連結 Skia 傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD static bool IsSkiaAvailable() noexcept;

    /// <summary>
    /// 嘗試使用 Skia 向量著色管線執行零拷貝高斯模糊加速。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="sigmaX">X 方向標準差。</param>
    /// <param name="sigmaY">Y 方向標準差。</param>
    /// <returns>若成功執行傳回 true；若 Skia 未啟用或處理失敗傳回 false (由呼叫端回退至 CPU 引擎)。</returns>
    static bool TryGaussianBlur(Bitmap& bmp, float sigmaX, float sigmaY);
};

} // namespace graphics
} // namespace cppui
