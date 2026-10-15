/*
 * File: Convolution.cpp
 * Purpose: Implements high-performance separable and 2D convolution with fixed-point arithmetic and window slicing.
 */

#include "ui/graphics/Convolution.hpp"
#include "compat/Compat.hpp"
#include <cmath>
#include <algorithm>
#include <thread>
#include <atomic>
#include <stdexcept>

namespace cppui {
namespace graphics {

namespace {

COMPAT_ALWAYS_INLINE static int32_t ClampCoord(int32_t val, int32_t limit) noexcept {
    return compat::clamp(val, 0, limit - 1);
}

COMPAT_ALWAYS_INLINE static int32_t ReflectCoord(int32_t val, int32_t limit) noexcept {
    if (limit <= 1) return 0;
    while (val < 0 || val >= limit) {
        if (val < 0) {
            val = -val;
        } else if (val >= limit) {
            val = 2 * limit - 2 - val;
        }
    }
    return val;
}

COMPAT_ALWAYS_INLINE static int32_t ResolveCoord(int32_t val, int32_t limit, BorderMode mode) noexcept {
    if (val >= 0 && val < limit) return val;
    switch (mode) {
        case BorderMode::Replicate:
            return ClampCoord(val, limit);
        case BorderMode::Reflect:
            return ReflectCoord(val, limit);
        case BorderMode::Constant:
        default:
            return -1;
    }
}

COMPAT_ALWAYS_INLINE static uint32_t PackARGB(int32_t a, int32_t r, int32_t g, int32_t b) noexcept {
    uint8_t ca = static_cast<uint8_t>(compat::clamp(a, 0, 255));
    uint8_t cr = static_cast<uint8_t>(compat::clamp(r, 0, 255));
    uint8_t cg = static_cast<uint8_t>(compat::clamp(g, 0, 255));
    uint8_t cb = static_cast<uint8_t>(compat::clamp(b, 0, 255));
    return (static_cast<uint32_t>(ca) << 24) |
           (static_cast<uint32_t>(cr) << 16) |
           (static_cast<uint32_t>(cg) << 8)  |
           static_cast<uint32_t>(cb);
}

template <typename TaskFn>
static void DispatchRowSlices(uint32_t height, const ParallelOptions& options, TaskFn&& task) {
    if (height == 0) return;
    if (options.mode == ExecutionMode::SingleThread || options.threadCount <= 1 || height <= 1) {
        task(0, height);
        return;
    }

    struct RowChunk { uint32_t y0; uint32_t y1; };
    std::vector<RowChunk> chunks;

    uint32_t chunkRows = options.sliceHeight;
    if (chunkRows == 0) {
        uint32_t rowsPerThread = (height + options.threadCount - 1) / options.threadCount;
        for (uint32_t y = 0; y < height; y += rowsPerThread) {
            chunks.push_back({ y, (std::min)(y + rowsPerThread, height) });
        }
    } else {
        for (uint32_t y = 0; y < height; y += chunkRows) {
            chunks.push_back({ y, (std::min)(y + chunkRows, height) });
        }
    }

    std::atomic<size_t> chunkIndex{ 0 };
    const size_t totalChunks = chunks.size();
    const uint32_t workerCount = (std::min)(options.threadCount, static_cast<uint32_t>(totalChunks));

    auto worker = [&]() {
        while (true) {
            size_t idx = chunkIndex.fetch_add(1, std::memory_order_relaxed);
            if (idx >= totalChunks) break;
            task(chunks[idx].y0, chunks[idx].y1);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(workerCount - 1);
    for (uint32_t i = 1; i < workerCount; ++i) {
        threads.emplace_back(worker);
    }

    worker();

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

} // anonymous namespace

void ConvolutionEngine::ConvolveSeparable(
    const Bitmap& src,
    Bitmap& dst,
    const SeparableKernel& kernel,
    BorderMode borderMode,
    const ParallelOptions& options
) {
    const uint32_t width = src.GetWidth();
    const uint32_t height = src.GetHeight();

    if (width == 0 || height == 0) {
        throw std::invalid_argument("ConvolutionEngine::ConvolveSeparable: source bitmap has zero dimensions");
    }
    if (width > Bitmap::MAX_DIMENSION || height > Bitmap::MAX_DIMENSION || (static_cast<uint64_t>(width) * height > Bitmap::MAX_PIXELS)) {
        throw std::invalid_argument("ConvolutionEngine::ConvolveSeparable: bitmap dimensions exceed safety limit");
    }
    if (dst.GetWidth() != width || dst.GetHeight() != height) {
        throw std::invalid_argument("ConvolutionEngine::ConvolveSeparable: destination bitmap dimensions must match source");
    }
    if (kernel.hWeights.empty() || kernel.vWeights.empty()) {
        throw std::invalid_argument("ConvolutionEngine::ConvolveSeparable: kernel weights cannot be empty");
    }
    if (kernel.hDivisor <= 0 || kernel.vDivisor <= 0) {
        throw std::invalid_argument("ConvolutionEngine::ConvolveSeparable: divisors must be positive");
    }

    const auto srcSpan = src.GetPixelSpan();
    const uint32_t* srcPixels = srcSpan.data();
    auto dstSpan = dst.GetPixelSpanMut();
    uint32_t* dstPixels = dstSpan.data();

    std::vector<uint32_t> temp(static_cast<size_t>(width) * height);
    uint32_t* tempPixels = temp.data();

    const int32_t radX = kernel.radiusX;
    const int32_t kSizeX = static_cast<int32_t>(kernel.hWeights.size());
    const int32_t* hW = kernel.hWeights.data();
    const int32_t hDiv = kernel.hDivisor;

    // 階段 1：水平 1D 卷積 (src -> temp)，逐列切割無分支加速
    DispatchRowSlices(height, options, [=](uint32_t y0, uint32_t y1) {
        for (uint32_t y = y0; y < y1; ++y) {
            const size_t rowOffset = static_cast<size_t>(y) * width;
            const uint32_t* srcRow = srcPixels + rowOffset;
            uint32_t* tempRow = tempPixels + rowOffset;

            int32_t x = 0;
            const int32_t leftEnd = (std::min)(radX, static_cast<int32_t>(width));

            // 左側邊界區 (包含鏡射/截斷保護)
            for (; x < leftEnd; ++x) {
                int32_t sumA = 0, sumR = 0, sumG = 0, sumB = 0;
                for (int32_t k = 0; k < kSizeX; ++k) {
                    int32_t sx = ResolveCoord(x - radX + k, static_cast<int32_t>(width), borderMode);
                    if (sx >= 0) {
                        uint32_t p = srcRow[sx];
                        int32_t w = hW[k];
                        sumA += w * static_cast<int32_t>((p >> 24) & 0xFF);
                        sumR += w * static_cast<int32_t>((p >> 16) & 0xFF);
                        sumG += w * static_cast<int32_t>((p >> 8) & 0xFF);
                        sumB += w * static_cast<int32_t>(p & 0xFF);
                    }
                }
                tempRow[x] = PackARGB(sumA / hDiv, sumR / hDiv, sumG / hDiv, sumB / hDiv);
            }

            // 中央無分支區 (100% Branch-Free，直接指針步進)
            const int32_t centerEnd = static_cast<int32_t>(width) - radX;
            for (; x < centerEnd; ++x) {
                int32_t sumA = 0, sumR = 0, sumG = 0, sumB = 0;
                const uint32_t* p = srcRow + (x - radX);
                for (int32_t k = 0; k < kSizeX; ++k) {
                    uint32_t px = p[k];
                    int32_t w = hW[k];
                    sumA += w * static_cast<int32_t>((px >> 24) & 0xFF);
                    sumR += w * static_cast<int32_t>((px >> 16) & 0xFF);
                    sumG += w * static_cast<int32_t>((px >> 8) & 0xFF);
                    sumB += w * static_cast<int32_t>(px & 0xFF);
                }
                tempRow[x] = PackARGB(sumA / hDiv, sumR / hDiv, sumG / hDiv, sumB / hDiv);
            }

            // 右側邊界區
            for (; x < static_cast<int32_t>(width); ++x) {
                int32_t sumA = 0, sumR = 0, sumG = 0, sumB = 0;
                for (int32_t k = 0; k < kSizeX; ++k) {
                    int32_t sx = ResolveCoord(x - radX + k, static_cast<int32_t>(width), borderMode);
                    if (sx >= 0) {
                        uint32_t p = srcRow[sx];
                        int32_t w = hW[k];
                        sumA += w * static_cast<int32_t>((p >> 24) & 0xFF);
                        sumR += w * static_cast<int32_t>((p >> 16) & 0xFF);
                        sumG += w * static_cast<int32_t>((p >> 8) & 0xFF);
                        sumB += w * static_cast<int32_t>(p & 0xFF);
                    }
                }
                tempRow[x] = PackARGB(sumA / hDiv, sumR / hDiv, sumG / hDiv, sumB / hDiv);
            }
        }
    });

    const int32_t radY = kernel.radiusY;
    const int32_t kSizeY = static_cast<int32_t>(kernel.vWeights.size());
    const int32_t* vW = kernel.vWeights.data();
    const int32_t vDiv = kernel.vDivisor;

    // 階段 2：垂直 1D 卷積 (temp -> dst)，維持 Row-major 連續存取優化
    DispatchRowSlices(height, options, [=](uint32_t y0, uint32_t y1) {
        for (uint32_t y = y0; y < y1; ++y) {
            uint32_t* dstRow = dstPixels + static_cast<size_t>(y) * width;
            int32_t iy = static_cast<int32_t>(y);

            if (iy >= radY && iy < static_cast<int32_t>(height) - radY) {
                // 垂直中央無分支區
                for (uint32_t x = 0; x < width; ++x) {
                    int32_t sumA = 0, sumR = 0, sumG = 0, sumB = 0;
                    for (int32_t k = 0; k < kSizeY; ++k) {
                        int32_t sy = iy - radY + k;
                        uint32_t px = tempPixels[static_cast<size_t>(sy) * width + x];
                        int32_t w = vW[k];
                        sumA += w * static_cast<int32_t>((px >> 24) & 0xFF);
                        sumR += w * static_cast<int32_t>((px >> 16) & 0xFF);
                        sumG += w * static_cast<int32_t>((px >> 8) & 0xFF);
                        sumB += w * static_cast<int32_t>(px & 0xFF);
                    }
                    dstRow[x] = PackARGB(sumA / vDiv, sumR / vDiv, sumG / vDiv, sumB / vDiv);
                }
            } else {
                // 頂部與底部邊界區
                for (uint32_t x = 0; x < width; ++x) {
                    int32_t sumA = 0, sumR = 0, sumG = 0, sumB = 0;
                    for (int32_t k = 0; k < kSizeY; ++k) {
                        int32_t sy = ResolveCoord(iy - radY + k, static_cast<int32_t>(height), borderMode);
                        if (sy >= 0) {
                            uint32_t px = tempPixels[static_cast<size_t>(sy) * width + x];
                            int32_t w = vW[k];
                            sumA += w * static_cast<int32_t>((px >> 24) & 0xFF);
                            sumR += w * static_cast<int32_t>((px >> 16) & 0xFF);
                            sumG += w * static_cast<int32_t>((px >> 8) & 0xFF);
                            sumB += w * static_cast<int32_t>(px & 0xFF);
                        }
                    }
                    dstRow[x] = PackARGB(sumA / vDiv, sumR / vDiv, sumG / vDiv, sumB / vDiv);
                }
            }
        }
    });
}

void ConvolutionEngine::Convolve2D(
    const Bitmap& src,
    Bitmap& dst,
    const Kernel2D& kernel,
    BorderMode borderMode,
    const ParallelOptions& options
) {
    const uint32_t width = src.GetWidth();
    const uint32_t height = src.GetHeight();

    if (width == 0 || height == 0) {
        throw std::invalid_argument("ConvolutionEngine::Convolve2D: source bitmap has zero dimensions");
    }
    if (width > Bitmap::MAX_DIMENSION || height > Bitmap::MAX_DIMENSION || (static_cast<uint64_t>(width) * height > Bitmap::MAX_PIXELS)) {
        throw std::invalid_argument("ConvolutionEngine::Convolve2D: bitmap dimensions exceed safety limit");
    }
    if (dst.GetWidth() != width || dst.GetHeight() != height) {
        throw std::invalid_argument("ConvolutionEngine::Convolve2D: destination bitmap dimensions must match source");
    }
    if (kernel.width <= 0 || kernel.height <= 0 || kernel.weights.empty()) {
        throw std::invalid_argument("ConvolutionEngine::Convolve2D: invalid kernel geometry or empty weights");
    }
    uint64_t requiredKernelWeights = static_cast<uint64_t>(kernel.width) * kernel.height;
    if (kernel.weights.size() < static_cast<size_t>(requiredKernelWeights)) {
        throw std::invalid_argument("ConvolutionEngine::Convolve2D: kernel weights count does not match kernel dimensions");
    }
    if (kernel.divisor == 0) {
        throw std::invalid_argument("ConvolutionEngine::Convolve2D: kernel divisor cannot be zero");
    }

    const auto srcSpan = src.GetPixelSpan();
    const uint32_t* srcPixels = srcSpan.data();
    auto dstSpan = dst.GetPixelSpanMut();
    uint32_t* dstPixels = dstSpan.data();

    const int32_t kw = kernel.width;
    const int32_t kh = kernel.height;
    const int32_t ax = kernel.anchorX;
    const int32_t ay = kernel.anchorY;
    const int32_t div = kernel.divisor;
    const int32_t offset = kernel.offset;
    const int32_t* kW = kernel.weights.data();

    DispatchRowSlices(height, options, [=](uint32_t y0, uint32_t y1) {
        for (uint32_t y = y0; y < y1; ++y) {
            uint32_t* dstRow = dstPixels + static_cast<size_t>(y) * width;
            int32_t iy = static_cast<int32_t>(y);

            bool isInsideY = (iy >= ay && iy < static_cast<int32_t>(height) - (kh - 1 - ay));

            for (uint32_t x = 0; x < width; ++x) {
                int32_t ix = static_cast<int32_t>(x);
                bool isInsideX = (ix >= ax && ix < static_cast<int32_t>(width) - (kw - 1 - ax));

                int32_t sumA = 0, sumR = 0, sumG = 0, sumB = 0;

                if (isInsideY && isInsideX) {
                    // 完全中央無分支核心
                    size_t wIdx = 0;
                    for (int32_t ky = 0; ky < kh; ++ky) {
                        const uint32_t* rowPtr = srcPixels + static_cast<size_t>(iy - ay + ky) * width + (ix - ax);
                        for (int32_t kx = 0; kx < kw; ++kx) {
                            uint32_t px = rowPtr[kx];
                            int32_t w = kW[wIdx++];
                            sumA += w * static_cast<int32_t>((px >> 24) & 0xFF);
                            sumR += w * static_cast<int32_t>((px >> 16) & 0xFF);
                            sumG += w * static_cast<int32_t>((px >> 8) & 0xFF);
                            sumB += w * static_cast<int32_t>(px & 0xFF);
                        }
                    }
                } else {
                    // 邊緣保護區
                    size_t wIdx = 0;
                    for (int32_t ky = 0; ky < kh; ++ky) {
                        int32_t sy = ResolveCoord(iy - ay + ky, static_cast<int32_t>(height), borderMode);
                        for (int32_t kx = 0; kx < kw; ++kx) {
                            int32_t sx = ResolveCoord(ix - ax + kx, static_cast<int32_t>(width), borderMode);
                            int32_t w = kW[wIdx++];
                            if (sy >= 0 && sx >= 0) {
                                uint32_t px = srcPixels[static_cast<size_t>(sy) * width + sx];
                                sumA += w * static_cast<int32_t>((px >> 24) & 0xFF);
                                sumR += w * static_cast<int32_t>((px >> 16) & 0xFF);
                                sumG += w * static_cast<int32_t>((px >> 8) & 0xFF);
                                sumB += w * static_cast<int32_t>(px & 0xFF);
                            }
                        }
                    }
                }

                dstRow[x] = PackARGB((sumA / div) + offset, (sumR / div) + offset, (sumG / div) + offset, (sumB / div) + offset);
            }
        }
    });
}

SeparableKernel ConvolutionEngine::CreateGaussianKernel(float sigmaX, float sigmaY) {
    if (sigmaX <= 0.0f) {
        throw std::invalid_argument("ConvolutionEngine::CreateGaussianKernel: sigmaX must be > 0");
    }
    if (sigmaY <= 0.0f) {
        sigmaY = sigmaX;
    }

    auto Build1DKernel = [](float sigma, int32_t& outRadius, int32_t& outDivisor) -> std::vector<int32_t> {
        outRadius = (std::max)(1, static_cast<int32_t>(std::ceil(sigma * 3.0f)));
        int32_t kSize = 2 * outRadius + 1;
        std::vector<float> fWeights(kSize);
        float twoSigmaSq = 2.0f * sigma * sigma;
        float sumF = 0.0f;

        for (int32_t i = 0; i < kSize; ++i) {
            float dist = static_cast<float>(i - outRadius);
            float w = std::exp(-(dist * dist) / twoSigmaSq);
            fWeights[i] = w;
            sumF += w;
        }

        // 定點數縮放：放大至 4096 精度
        const float scale = 4096.0f / sumF;
        std::vector<int32_t> iWeights(kSize);
        int32_t sumI = 0;

        for (int32_t i = 0; i < kSize; ++i) {
            int32_t val = (std::max)(1, static_cast<int32_t>(std::round(fWeights[i] * scale)));
            iWeights[i] = val;
            sumI += val;
        }

        outDivisor = sumI;
        return iWeights;
    };

    SeparableKernel k;
    k.hWeights = Build1DKernel(sigmaX, k.radiusX, k.hDivisor);
    k.vWeights = Build1DKernel(sigmaY, k.radiusY, k.vDivisor);
    return k;
}

SeparableKernel ConvolutionEngine::CreateBoxKernel(int32_t radiusX, int32_t radiusY) {
    if (radiusX < 0) {
        throw std::invalid_argument("ConvolutionEngine::CreateBoxKernel: radiusX must be >= 0");
    }
    if (radiusY < 0) {
        radiusY = radiusX;
    }

    SeparableKernel k;
    k.radiusX = radiusX;
    k.radiusY = radiusY;
    k.hWeights.assign(2 * radiusX + 1, 1);
    k.vWeights.assign(2 * radiusY + 1, 1);
    k.hDivisor = 2 * radiusX + 1;
    k.vDivisor = 2 * radiusY + 1;
    return k;
}

SeparableKernel ConvolutionEngine::CreateSobelKernelX() {
    SeparableKernel k;
    k.radiusX = 1;
    k.radiusY = 1;
    k.hWeights = { -1, 0, 1 };
    k.vWeights = { 1, 2, 1 };
    k.hDivisor = 1;
    k.vDivisor = 1;
    return k;
}

SeparableKernel ConvolutionEngine::CreateSobelKernelY() {
    SeparableKernel k;
    k.radiusX = 1;
    k.radiusY = 1;
    k.hWeights = { 1, 2, 1 };
    k.vWeights = { -1, 0, 1 };
    k.hDivisor = 1;
    k.vDivisor = 1;
    return k;
}

SeparableKernel ConvolutionEngine::CreatePrewittKernelX() {
    SeparableKernel k;
    k.radiusX = 1;
    k.radiusY = 1;
    k.hWeights = { -1, 0, 1 };
    k.vWeights = { 1, 1, 1 };
    k.hDivisor = 1;
    k.vDivisor = 1;
    return k;
}

SeparableKernel ConvolutionEngine::CreatePrewittKernelY() {
    SeparableKernel k;
    k.radiusX = 1;
    k.radiusY = 1;
    k.hWeights = { 1, 1, 1 };
    k.vWeights = { -1, 0, 1 };
    k.hDivisor = 1;
    k.vDivisor = 1;
    return k;
}

} // namespace graphics
} // namespace cppui
