/**
 * @file PixelParityTestUtils.hpp
 * @brief 跨渲染後端像素相似度度量工具 (MSE, PSNR, SSIM)，提供純函數圖像一致性驗證
 */

#pragma once

#include <cstdint>
#include <cmath>
#include <stdexcept>
#include "compat/Compat.hpp"

namespace cppui {
namespace test {

/// <summary>
/// 像素一致性比較結果結構
/// </summary>
struct ParityResult {
    double mse  = 0.0;
    double psnr = 0.0;
    double ssim = 0.0;
    bool passed = false;
};

/// <summary>
/// 跨後端像素比較與訊噪比/結構相似度純函數工具類別
/// </summary>
class PixelParityTestUtils {
public:
    /// <summary>
    /// 計算兩組像素緩衝區之均方誤差 (Mean Squared Error)。
    /// </summary>
    /// <param name="bufA">第一組 32-bit (ARGB/BGRA) 像素緩衝區視圖。</param>
    /// <param name="bufB">第二組 32-bit (ARGB/BGRA) 像素緩衝區視圖。</param>
    /// <param name="width">畫布寬度 (像素)。</param>
    /// <param name="height">畫布高度 (像素)。</param>
    /// <returns>MSE 均方誤差浮點數值。</returns>
    /// <exception cref="std::invalid_argument">當寬高為 0 或緩衝區大小不足時擲出。</exception>
    COMPAT_NODISCARD static double ComputeMSE(
        compat::span<const uint32_t> bufA,
        compat::span<const uint32_t> bufB,
        uint32_t width,
        uint32_t height
    ) {
        if (width == 0 || height == 0) {
            throw std::invalid_argument("PixelParityTestUtils::ComputeMSE: Dimensions must be greater than zero.");
        }
        const size_t totalPixels = static_cast<size_t>(width) * height;
        if (bufA.size() < totalPixels || bufB.size() < totalPixels) {
            throw std::invalid_argument("PixelParityTestUtils::ComputeMSE: Buffer size smaller than width * height.");
        }

        double sumSq = 0.0;
        for (size_t i = 0; i < totalPixels; ++i) {
            uint32_t pA = bufA[i];
            uint32_t pB = bufB[i];

            int32_t rA = static_cast<int32_t>((pA >> 16) & 0xFF);
            int32_t gA = static_cast<int32_t>((pA >> 8)  & 0xFF);
            int32_t bA = static_cast<int32_t>(pA & 0xFF);

            int32_t rB = static_cast<int32_t>((pB >> 16) & 0xFF);
            int32_t gB = static_cast<int32_t>((pB >> 8)  & 0xFF);
            int32_t bB = static_cast<int32_t>(pB & 0xFF);

            int32_t dr = rA - rB;
            int32_t dg = gA - gB;
            int32_t db = bA - bB;

            sumSq += static_cast<double>(dr * dr + dg * dg + db * db);
        }

        return sumSq / (3.0 * static_cast<double>(totalPixels));
    }

    /// <summary>
    /// 計算兩組像素緩衝區之峰值訊噪比 (Peak Signal-to-Noise Ratio, in dB)。
    /// 若兩緩衝區像素位元級完全一致 (MSE == 0)，回傳 100.0 dB 代表完美一致。
    /// </summary>
    /// <param name="bufA">第一組像素緩衝區視圖。</param>
    /// <param name="bufB">第二組像素緩衝區視圖。</param>
    /// <param name="width">畫布寬度 (像素)。</param>
    /// <param name="height">畫布高度 (像素)。</param>
    /// <returns>PSNR 數值 (分貝 dB)。</returns>
    COMPAT_NODISCARD static double ComputePSNR(
        compat::span<const uint32_t> bufA,
        compat::span<const uint32_t> bufB,
        uint32_t width,
        uint32_t height
    ) {
        double mse = ComputeMSE(bufA, bufB, width, height);
        if (mse <= 1e-9) {
            return 100.0; // 位元完全一致，傳回理論高值
        }
        return 10.0 * std::log10((255.0 * 255.0) / mse);
    }

    /// <summary>
    /// 計算兩組像素緩衝區之結構相似度 (Structural Similarity Index, SSIM in [0, 1])。
    /// 依據 ITU-R BT.601 亮度加權公式與 8x8 局部窗口滑動運算。
    /// </summary>
    /// <param name="bufA">第一組像素緩衝區視圖。</param>
    /// <param name="bufB">第二組像素緩衝區視圖。</param>
    /// <param name="width">畫布寬度 (像素)。</param>
    /// <param name="height">畫布高度 (像素)。</param>
    /// <returns>SSIM 相似度數值，範圍介於 0.0 (完全不同) 至 1.0 (完全相同)。</returns>
    COMPAT_NODISCARD static double ComputeSSIM(
        compat::span<const uint32_t> bufA,
        compat::span<const uint32_t> bufB,
        uint32_t width,
        uint32_t height
    ) {
        if (width == 0 || height == 0) {
            throw std::invalid_argument("PixelParityTestUtils::ComputeSSIM: Dimensions must be greater than zero.");
        }
        const size_t totalPixels = static_cast<size_t>(width) * height;
        if (bufA.size() < totalPixels || bufB.size() < totalPixels) {
            throw std::invalid_argument("PixelParityTestUtils::ComputeSSIM: Buffer size smaller than width * height.");
        }

        // 常數定義 (對齊標準 SSIM 規範: C1 = (K1*L)^2, C2 = (K2*L)^2, 其中 K1=0.01, K2=0.03, L=255)
        constexpr double c1 = (0.01 * 255.0) * (0.01 * 255.0); // 6.5025
        constexpr double c2 = (0.03 * 255.0) * (0.03 * 255.0); // 58.5225

        constexpr uint32_t kWindowSize = 8;
        if (width < kWindowSize || height < kWindowSize) {
            // 影像小於 8x8 時採用全域單窗口計算
            return ComputeWindowSSIM(bufA, bufB, 0, 0, width, height, width, c1, c2);
        }

        double totalSsim = 0.0;
        size_t windowCount = 0;

        for (uint32_t y = 0; y + kWindowSize <= height; y += kWindowSize) {
            for (uint32_t x = 0; x + kWindowSize <= width; x += kWindowSize) {
                totalSsim += ComputeWindowSSIM(bufA, bufB, x, y, kWindowSize, kWindowSize, width, c1, c2);
                windowCount++;
            }
        }

        return (windowCount > 0) ? (totalSsim / static_cast<double>(windowCount)) : 1.0;
    }

    /// <summary>
    /// 執行跨後端一致性斷言檢查，驗證 PSNR 與 SSIM 是否達標。
    /// </summary>
    /// <param name="bufA">第一組像素緩衝區視圖。</param>
    /// <param name="bufB">第二組像素緩衝區視圖。</param>
    /// <param name="width">畫布寬度 (像素)。</param>
    /// <param name="height">畫布高度 (像素)。</param>
    /// <param name="minPsnr">最低要求 PSNR (dB)。</param>
    /// <param name="minSsim">最低要求 SSIM [0, 1]。</param>
    /// <returns>ParityResult 統計結構。</returns>
    COMPAT_NODISCARD static ParityResult VerifyParity(
        compat::span<const uint32_t> bufA,
        compat::span<const uint32_t> bufB,
        uint32_t width,
        uint32_t height,
        double minPsnr,
        double minSsim
    ) {
        double mse  = ComputeMSE(bufA, bufB, width, height);
        double psnr = ComputePSNR(bufA, bufB, width, height);
        double ssim = ComputeSSIM(bufA, bufB, width, height);

        ParityResult res;
        res.mse = mse;
        res.psnr = psnr;
        res.ssim = ssim;
        res.passed = (psnr >= minPsnr) && (ssim >= minSsim);
        return res;
    }

private:
    /// <summary>計算單一局部窗口之 SSIM。</summary>
    COMPAT_NODISCARD static double ComputeWindowSSIM(
        compat::span<const uint32_t> bufA,
        compat::span<const uint32_t> bufB,
        uint32_t startX,
        uint32_t startY,
        uint32_t winW,
        uint32_t winH,
        uint32_t stride,
        double c1,
        double c2
    ) noexcept {
        const size_t n = static_cast<size_t>(winW) * winH;
        if (n == 0) return 1.0;

        double sumX = 0.0;
        double sumY = 0.0;

        for (uint32_t wy = 0; wy < winH; ++wy) {
            size_t rowOffset = static_cast<size_t>(startY + wy) * stride + startX;
            for (uint32_t wx = 0; wx < winW; ++wx) {
                uint32_t pA = bufA[rowOffset + wx];
                uint32_t pB = bufB[rowOffset + wx];

                double lumaA = 0.299 * static_cast<double>((pA >> 16) & 0xFF) +
                               0.587 * static_cast<double>((pA >> 8)  & 0xFF) +
                               0.114 * static_cast<double>(pA & 0xFF);
                double lumaB = 0.299 * static_cast<double>((pB >> 16) & 0xFF) +
                               0.587 * static_cast<double>((pB >> 8)  & 0xFF) +
                               0.114 * static_cast<double>(pB & 0xFF);

                sumX += lumaA;
                sumY += lumaB;
            }
        }

        double muX = sumX / static_cast<double>(n);
        double muY = sumY / static_cast<double>(n);

        double varX = 0.0;
        double varY = 0.0;
        double covXY = 0.0;

        for (uint32_t wy = 0; wy < winH; ++wy) {
            size_t rowOffset = static_cast<size_t>(startY + wy) * stride + startX;
            for (uint32_t wx = 0; wx < winW; ++wx) {
                uint32_t pA = bufA[rowOffset + wx];
                uint32_t pB = bufB[rowOffset + wx];

                double lumaA = 0.299 * static_cast<double>((pA >> 16) & 0xFF) +
                               0.587 * static_cast<double>((pA >> 8)  & 0xFF) +
                               0.114 * static_cast<double>(pA & 0xFF);
                double lumaB = 0.299 * static_cast<double>((pB >> 16) & 0xFF) +
                               0.587 * static_cast<double>((pB >> 8)  & 0xFF) +
                               0.114 * static_cast<double>(pB & 0xFF);

                double dx = lumaA - muX;
                double dy = lumaB - muY;

                varX  += dx * dx;
                varY  += dy * dy;
                covXY += dx * dy;
            }
        }

        double sigmaX2  = varX  / static_cast<double>(n);
        double sigmaY2  = varY  / static_cast<double>(n);
        double sigmaXY  = covXY / static_cast<double>(n);

        double numerator   = (2.0 * muX * muY + c1) * (2.0 * sigmaXY + c2);
        double denominator = (muX * muX + muY * muY + c1) * (sigmaX2 + sigmaY2 + c2);

        if (denominator <= 0.0) return 1.0;
        return numerator / denominator;
    }
};

} // namespace test
} // namespace cppui
