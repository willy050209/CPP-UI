/*
 * File: ImageFilters.hpp
 * Purpose: Complete suite of high-performance image processing filters (grayscale, thresholding, blurs, edges).
 */

#pragma once

#include <cstdint>
#include <memory>
#include "compat/Compat.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/ParallelOptions.hpp"

namespace cppui {
namespace graphics {

/// <summary>
/// 灰階轉換演算法標準權重模式列舉 (GrayscaleMethod)
/// </summary>
enum class GrayscaleMethod : uint8_t {
    BT601 = 0,     // ITU-R BT.601 標準: (77R + 150G + 29B) >> 8
    BT709 = 1,     // ITU-R BT.709 標準: (54R + 183G + 19B) >> 8
    Average = 2,   // 算術平均法: (R + G + B) / 3
    Lightness = 3  // 去飽和明度法: (max(R,G,B) + min(R,G,B)) / 2
};

/// <summary>
/// 二值化運算門檻模式列舉 (ThresholdType)
/// </summary>
enum class ThresholdType : uint8_t {
    Binary = 0,         // v >= T ? 255 : 0
    BinaryInverted = 1, // v >= T ? 0 : 255
    Truncate = 2,       // v > T ? T : v
    ToZero = 3          // v >= T ? v : 0
};

/// <summary>
/// 邊緣偵測算子列舉 (EdgeOperator)
/// </summary>
enum class EdgeOperator : uint8_t {
    Sobel = 0,
    Prewitt = 1,
    Scharr = 2,
    Laplacian = 3
};

/// <summary>
/// 高效能影像濾鏡與運算套件 (ImageFilters)
/// 涵蓋色彩空間轉換、自適應二值化、六大模糊平滑演算法與邊緣偵測，全面支援定點數與 ParallelOptions。
/// </summary>
class ImageFilters {
public:
    // -------------------------------------------------------------------------
    // 1. 灰階轉換 (Grayscale Conversion)
    // -------------------------------------------------------------------------

    /// <summary>
    /// 就地將點陣影像轉為灰階 (In-place)。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="method">灰階轉換標準方法。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void Grayscale(
        Bitmap& bmp,
        GrayscaleMethod method = GrayscaleMethod::BT601,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 純函數版本：將來源點陣影像轉為灰階並傳回新 Bitmap 實例副本。
    /// </summary>
    /// <param name="src">來源唯讀點陣影像。</param>
    /// <param name="method">灰階轉換標準方法。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    /// <returns>轉換後之新 Bitmap 共享指標。</returns>
    COMPAT_NODISCARD static std::shared_ptr<Bitmap> ToGrayscale(
        const Bitmap& src,
        GrayscaleMethod method = GrayscaleMethod::BT601,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    // -------------------------------------------------------------------------
    // 2. 二值化 (Thresholding & Binarization)
    // -------------------------------------------------------------------------

    /// <summary>
    /// 依據指定常數閥值執行全域二值化。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="threshold">門檻值 (0~255)。</param>
    /// <param name="type">二值化型態。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void Threshold(
        Bitmap& bmp,
        uint8_t threshold,
        ThresholdType type = ThresholdType::Binary,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 依據大津法 (Otsu's method) 計算最大類間變異數之全圖自動最佳門檻。
    /// </summary>
    /// <param name="bmp">來源點陣影像。</param>
    /// <returns>計算得出之最佳二值化門檻數值 (0~255)。</returns>
    COMPAT_NODISCARD static uint8_t ComputeOtsuThreshold(const Bitmap& bmp);

    /// <summary>
    /// 執行大津法自動二值化 (Otsu Binarization)。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="type">二值化型態。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void OtsuThreshold(
        Bitmap& bmp,
        ThresholdType type = ThresholdType::Binary,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 執行局部自適應二值化 (Adaptive Thresholding)。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="blockSize">局部鄰域視窗大小 (必須為奇數且 >= 3)。</param>
    /// <param name="c">從局部均值扣除之常數偏移量。</param>
    /// <param name="useGaussian">是否以局部高斯加權替代方框均值。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void AdaptiveThreshold(
        Bitmap& bmp,
        int32_t blockSize,
        int32_t c,
        bool useGaussian = false,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    // -------------------------------------------------------------------------
    // 3. 模糊平滑濾鏡 (Blur Filters)
    // -------------------------------------------------------------------------

    /// <summary>
    /// 執行方框模糊 (Box Blur)，內部以可分離 1D 核心高速計算。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="radius">模糊半徑 (像素距離，必須 >= 0)。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void BoxBlur(
        Bitmap& bmp,
        int32_t radius,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 執行均值模糊 (Mean / Average Blur)。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="kernelSize">卷積核尺寸 (必須為奇數且 >= 1)。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void MeanBlur(
        Bitmap& bmp,
        int32_t kernelSize,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 執行高斯模糊 (Gaussian Blur)。支援 Skia 硬體加速優先切換。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="sigmaX">X 方向標準差 (必須 > 0)。</param>
    /// <param name="sigmaY">Y 方向標準差 (<= 0 代表等於 sigmaX)。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    /// <param name="preferSkia">若 Skia 可用時是否優先啟用 Skia SIMD/硬體加速。</param>
    static void GaussianBlur(
        Bitmap& bmp,
        float sigmaX,
        float sigmaY = 0.0f,
        const ParallelOptions& options = ParallelOptions::Sequential(),
        bool preferSkia = true
    );

    /// <summary>
    /// 執行中值模糊 (Median Blur)，有效濾除胡椒鹽雜訊。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="kernelSize">鄰域核尺寸 (3 或 5，必須為奇數)。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void MedianBlur(
        Bitmap& bmp,
        int32_t kernelSize = 3,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 執行動態模糊 (Motion Blur)，模擬線性方向運動造成的拖影。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="length">動態拖影長度 (像素，必須 >= 1)。</param>
    /// <param name="angleDegrees">運動方向角度 (0~360 度)。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void MotionBlur(
        Bitmap& bmp,
        int32_t length,
        float angleDegrees,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 執行雙邊濾波 (Bilateral Filter)，結合空間與色彩距離達成保邊平滑。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="radius">濾波空間半徑 (必須 >= 1)。</param>
    /// <param name="sigmaColor">色彩強度標準差 (必須 > 0)。</param>
    /// <param name="sigmaSpace">空間距離標準差 (必須 > 0)。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void BilateralFilter(
        Bitmap& bmp,
        int32_t radius,
        float sigmaColor,
        float sigmaSpace,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    // -------------------------------------------------------------------------
    // 4. 邊緣偵測 (Edge Detection)
    // -------------------------------------------------------------------------

    /// <summary>
    /// 執行一階或二階微分梯度邊緣偵測 (Sobel, Prewitt, Scharr, Laplacian)。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="op">邊緣算子種類。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void DetectEdges(
        Bitmap& bmp,
        EdgeOperator op = EdgeOperator::Sobel,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );

    /// <summary>
    /// 執行 Canny 多階段邊緣偵測演算法。
    /// 包含高斯濾波、Sobel 梯度強度/方向計算、非極大值抑制 (NMS) 與雙門檻遲滯追蹤。
    /// </summary>
    /// <param name="bmp">目標點陣影像。</param>
    /// <param name="lowThreshold">低門檻值。</param>
    /// <param name="highThreshold">高門檻值 (必須 >= lowThreshold)。</param>
    /// <param name="gaussianSigma">前置平滑之高斯標準差 (預設 1.0f)。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    static void Canny(
        Bitmap& bmp,
        uint8_t lowThreshold,
        uint8_t highThreshold,
        float gaussianSigma = 1.0f,
        const ParallelOptions& options = ParallelOptions::Sequential()
    );
};

} // namespace graphics
} // namespace cppui
