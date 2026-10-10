/*
 * File: Convolution.hpp
 * Purpose: Separable and 2D convolution kernel definitions and execution engine with fixed-point arithmetic.
 */

#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "compat/compat.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/ParallelOptions.hpp"

namespace cppui {
namespace graphics {

/// <summary>
/// 卷積邊界外像素處理模式列舉 (BorderMode)
/// </summary>
enum class BorderMode : uint8_t {
    Replicate = 0, // 重複邊緣像素 (Clamp 至邊界)
    Reflect   = 1, // 鏡射邊緣像素 (Mirror)
    Constant  = 2  // 常數邊緣像素 (預設全透明/黑色)
};

/// <summary>
/// 1D 定點數可分離卷積核 (SeparableKernel)
/// 水平與垂直 1D 核拆解，滿足 K = v * h^T。
/// </summary>
struct SeparableKernel {
    std::vector<int32_t> hWeights; // 水平定點數整數權重
    std::vector<int32_t> vWeights; // 垂直定點數整數權重
    int32_t hDivisor{ 1 };         // 水平標準化除數 (必須 > 0)
    int32_t vDivisor{ 1 };         // 垂直標準化除數 (必須 > 0)
    int32_t radiusX{ 0 };          // 水平半徑 (核長度為 2 * radiusX + 1)
    int32_t radiusY{ 0 };          // 垂直半徑 (核長度為 2 * radiusY + 1)
};

/// <summary>
/// 2D 通用定點數卷積核 (Kernel2D)
/// </summary>
struct Kernel2D {
    int32_t width{ 0 };            // 核寬度
    int32_t height{ 0 };           // 核高度
    int32_t anchorX{ 0 };          // 錨點 X 座標 (預設 width / 2)
    int32_t anchorY{ 0 };          // 錨點 Y 座標 (預設 height / 2)
    std::vector<int32_t> weights;  // 權重陣列 (大小為 width * height)
    int32_t divisor{ 1 };          // 標準化除數 (必須 != 0)
    int32_t offset{ 0 };           // 偏置常數 (加至結果)
};

/// <summary>
/// 高效能定點數卷積運算引擎 (ConvolutionEngine)
/// 整合演算法拆解 (可分離核)、定點整數算術、消除分支邊界分割、Row-major 快取優化與視窗分塊平行化。
/// </summary>
class ConvolutionEngine {
public:
    /// <summary>
    /// 執行可分離二維卷積 (水平 1D + 垂直 1D 分塊無分支運算)。
    /// </summary>
    /// <param name="src">來源點陣影像。</param>
    /// <param name="dst">目標點陣影像 (尺寸必須與來源一致)。</param>
    /// <param name="kernel">可分離定點數卷積核。</param>
    /// <param name="borderMode">邊界處理模式。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    /// <exception cref="std::invalid_argument">若來源或目標尺寸不符或參數非法擲出例外。</exception>
    static void ConvolveSeparable(
        const Bitmap& src,
        Bitmap& dst,
        const SeparableKernel& kernel,
        BorderMode borderMode = BorderMode::Replicate,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 執行通用二維矩陣卷積 (定點整數乘加與無分支邊界分區)。
    /// </summary>
    /// <param name="src">來源點陣影像。</param>
    /// <param name="dst">目標點陣影像 (尺寸必須與來源一致)。</param>
    /// <param name="kernel">通用二維定點數卷積核。</param>
    /// <param name="borderMode">邊界處理模式。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    /// <exception cref="std::invalid_argument">若來源或目標尺寸不符或參數非法擲出例外。</exception>
    static void Convolve2D(
        const Bitmap& src,
        Bitmap& dst,
        const Kernel2D& kernel,
        BorderMode borderMode = BorderMode::Replicate,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 建立標準 1D 高斯定點數可分離卷積核。
    /// </summary>
    /// <param name="sigmaX">水平標準差 (必須 > 0)。</param>
    /// <param name="sigmaY">垂直標準差 (<= 0 則等於 sigmaX)。</param>
    /// <returns>SeparableKernel 結構實例。</returns>
    COMPAT_NODISCARD static SeparableKernel CreateGaussianKernel(float sigmaX, float sigmaY = 0.0f);

    /// <summary>
    /// 建立標準 1D 方框模糊 (Box Blur) 定點數可分離卷積核。
    /// </summary>
    /// <param name="radiusX">水平半徑 (必須 >= 0)。</param>
    /// <param name="radiusY">垂直半徑 (若小於 0 則等於 radiusX)。</param>
    /// <returns>SeparableKernel 結構實例。</returns>
    COMPAT_NODISCARD static SeparableKernel CreateBoxKernel(int32_t radiusX, int32_t radiusY = -1);

    /// <summary>建立 Sobel X 方向梯度可分離卷積核。</summary>
    COMPAT_NODISCARD static SeparableKernel CreateSobelKernelX();

    /// <summary>建立 Sobel Y 方向梯度可分離卷積核。</summary>
    COMPAT_NODISCARD static SeparableKernel CreateSobelKernelY();

    /// <summary>建立 Prewitt X 方向梯度可分離卷積核。</summary>
    COMPAT_NODISCARD static SeparableKernel CreatePrewittKernelX();

    /// <summary>建立 Prewitt Y 方向梯度可分離卷積核。</summary>
    COMPAT_NODISCARD static SeparableKernel CreatePrewittKernelY();
};

} // namespace graphics
} // namespace cppui
