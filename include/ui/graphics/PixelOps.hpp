/*
 * File: PixelOps.hpp
 * Purpose: High-performance sequential and parallel pixel traversal algorithms with user-controlled threading.
 */

#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <atomic>
#include <type_traits>
#include "compat/compat.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/ParallelOptions.hpp"
#include "ui/core/Geometry.hpp"

namespace cppui {
namespace graphics {

namespace detail {

template <typename F, typename P>
COMPAT_ALWAYS_INLINE static void InvokePixelOp(F&& f, uint32_t x, uint32_t y, P& pixel) {
    if constexpr (std::is_invocable_v<F, P&>) {
        f(pixel);
    } else if constexpr (std::is_invocable_v<F, uint32_t, uint32_t, P&>) {
        f(x, y, pixel);
    } else if constexpr (std::is_invocable_v<F, uint32_t, uint32_t, core::Color&>) {
        core::Color c = core::Color::FromArgb(pixel);
        f(x, y, c);
        if constexpr (!std::is_const_v<P>) {
            pixel = c.ToArgb();
        }
    } else if constexpr (std::is_invocable_v<F, uint32_t, uint32_t, const core::Color&>) {
        core::Color c = core::Color::FromArgb(pixel);
        f(x, y, c);
    } else {
        static_assert(std::is_invocable_v<F, P&>, "Incompatible functor signature for PixelOps");
    }
}

template <typename Pred, typename P>
COMPAT_ALWAYS_INLINE static bool InvokePred(Pred&& pred, uint32_t x, uint32_t y, const P& pixel) {
    if constexpr (std::is_invocable_v<Pred, const P&>) {
        return pred(pixel);
    } else if constexpr (std::is_invocable_v<Pred, uint32_t, uint32_t, const P&>) {
        return pred(x, y, pixel);
    } else if constexpr (std::is_invocable_v<Pred, uint32_t, uint32_t, const core::Color&>) {
        return pred(x, y, core::Color::FromArgb(pixel));
    } else {
        static_assert(std::is_invocable_v<Pred, const P&>, "Incompatible predicate signature for PixelOps");
        return false;
    }
}

template <typename P, typename F>
static void ExecuteRowRange(P* pixels, uint32_t width, uint32_t y0, uint32_t y1, F&& func) {
    for (uint32_t y = y0; y < y1; ++y) {
        size_t rowOffset = static_cast<size_t>(y) * width;
        for (uint32_t x = 0; x < width; ++x) {
            InvokePixelOp(func, x, y, pixels[rowOffset + x]);
        }
    }
}

template <typename P, typename Pred, typename F>
static void ExecuteRowRangeIf(P* pixels, uint32_t width, uint32_t y0, uint32_t y1, Pred&& pred, F&& func) {
    for (uint32_t y = y0; y < y1; ++y) {
        size_t rowOffset = static_cast<size_t>(y) * width;
        for (uint32_t x = 0; x < width; ++x) {
            if (InvokePred(pred, x, y, pixels[rowOffset + x])) {
                InvokePixelOp(func, x, y, pixels[rowOffset + x]);
            }
        }
    }
}

} // namespace detail

/// <summary>
/// 高效能像素遍歷與條件運算器 (PixelOps)
/// 提供零開銷的多型函式物件調用，並全面由 ParallelOptions 決定單/多執行緒與視窗分塊排程。
/// </summary>
class PixelOps {
public:
    /// <summary>
    /// 高速遍歷可變點陣圖形所有像素點並套用操作方法。
    /// </summary>
    /// <typeparam name="Func">操作函式或 Lambda 運算式。</typeparam>
    /// <param name="bmp">目標點陣圖實例。</param>
    /// <param name="func">像素操作函數。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    template <typename Func>
    static void ForEach(Bitmap& bmp, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential()) {
        const uint32_t width = bmp.GetWidth();
        const uint32_t height = bmp.GetHeight();
        if (width == 0 || height == 0) return;

        auto span = bmp.GetPixelSpanMut();
        uint32_t* pixels = span.data();

        if (options.mode == ExecutionMode::SingleThread || options.threadCount <= 1 || height <= 1) {
            detail::ExecuteRowRange(pixels, width, 0, height, func);
            return;
        }

        ExecuteParallel(pixels, width, height, options, [&func, pixels, width](uint32_t y0, uint32_t y1) {
            detail::ExecuteRowRange(pixels, width, y0, y1, func);
        });
    }

    /// <summary>
    /// 高速遍歷唯讀點陣圖形所有像素點。
    /// </summary>
    /// <typeparam name="Func">讀取函式或 Lambda 運算式。</typeparam>
    /// <param name="bmp">來源點陣圖實例。</param>
    /// <param name="func">像素操作函數。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    template <typename Func>
    static void ForEach(const Bitmap& bmp, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential()) {
        const uint32_t width = bmp.GetWidth();
        const uint32_t height = bmp.GetHeight();
        if (width == 0 || height == 0) return;

        auto span = bmp.GetPixelSpan();
        const uint32_t* pixels = span.data();

        if (options.mode == ExecutionMode::SingleThread || options.threadCount <= 1 || height <= 1) {
            detail::ExecuteRowRange(pixels, width, 0, height, func);
            return;
        }

        ExecuteParallel(pixels, width, height, options, [&func, pixels, width](uint32_t y0, uint32_t y1) {
            detail::ExecuteRowRange(pixels, width, y0, y1, func);
        });
    }

    /// <summary>
    /// 附帶條件篩選之像素遍歷（僅在符合條件時執行操作）。
    /// </summary>
    /// <typeparam name="Pred">判定條件謂詞函式。</typeparam>
    /// <typeparam name="Func">操作函式。</typeparam>
    /// <param name="bmp">目標點陣圖實例。</param>
    /// <param name="pred">條件謂詞。</param>
    /// <param name="func">像素操作函數。</param>
    /// <param name="options">平行運算與視窗切分選項。</param>
    template <typename Pred, typename Func>
    static void ForEachIf(Bitmap& bmp, Pred&& pred, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential()) {
        const uint32_t width = bmp.GetWidth();
        const uint32_t height = bmp.GetHeight();
        if (width == 0 || height == 0) return;

        auto span = bmp.GetPixelSpanMut();
        uint32_t* pixels = span.data();

        if (options.mode == ExecutionMode::SingleThread || options.threadCount <= 1 || height <= 1) {
            detail::ExecuteRowRangeIf(pixels, width, 0, height, pred, func);
            return;
        }

        ExecuteParallel(pixels, width, height, options, [&pred, &func, pixels, width](uint32_t y0, uint32_t y1) {
            detail::ExecuteRowRangeIf(pixels, width, y0, y1, pred, func);
        });
    }

    /// <summary>
    /// 附帶條件篩選之唯讀像素遍歷。
    /// </summary>
    template <typename Pred, typename Func>
    static void ForEachIf(const Bitmap& bmp, Pred&& pred, Func&& func, const ParallelOptions& options = ParallelOptions::Sequential()) {
        const uint32_t width = bmp.GetWidth();
        const uint32_t height = bmp.GetHeight();
        if (width == 0 || height == 0) return;

        auto span = bmp.GetPixelSpan();
        const uint32_t* pixels = span.data();

        if (options.mode == ExecutionMode::SingleThread || options.threadCount <= 1 || height <= 1) {
            detail::ExecuteRowRangeIf(pixels, width, 0, height, pred, func);
            return;
        }

        ExecuteParallel(pixels, width, height, options, [&pred, &func, pixels, width](uint32_t y0, uint32_t y1) {
            detail::ExecuteRowRangeIf(pixels, width, y0, y1, pred, func);
        });
    }

private:
    template <typename P, typename TaskFn>
    static void ExecuteParallel(P* /*pixels*/, uint32_t /*width*/, uint32_t height, const ParallelOptions& options, TaskFn&& task) {
        struct RowChunk { uint32_t y0; uint32_t y1; };
        std::vector<RowChunk> chunks;

        uint32_t chunkRows = options.sliceHeight;
        if (chunkRows == 0) {
            // 均分切塊
            uint32_t rowsPerThread = (height + options.threadCount - 1) / options.threadCount;
            for (uint32_t y = 0; y < height; y += rowsPerThread) {
                chunks.push_back({ y, (std::min)(y + rowsPerThread, height) });
            }
        } else {
            // 使用者自訂視窗列切分 (Window Slicing)
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

        // 當前執行緒亦參與處理
        worker();

        for (auto& th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
};

} // namespace graphics
} // namespace cppui
