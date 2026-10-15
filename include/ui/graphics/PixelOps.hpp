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
#include "compat/Compat.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/ParallelOptions.hpp"
#include "ui/core/Geometry.hpp"

namespace cppui {
namespace graphics {

namespace detail {

template <size_t N>
struct priority_tag : priority_tag<N - 1> {};

template <>
struct priority_tag<0> {};

template <typename Dest>
COMPAT_ALWAYS_INLINE static typename std::enable_if<!std::is_const<Dest>::value>::type
AssignColorToPixel(Dest& dest, uint32_t val) {
    dest = val;
}

template <typename Dest>
COMPAT_ALWAYS_INLINE static typename std::enable_if<std::is_const<Dest>::value>::type
AssignColorToPixel(Dest&, uint32_t) {
    // 唯讀目的端：空操作
}

// InvokePixelOp 重載鏈 (由優先權由高至低比對)
template <typename F, typename P>
COMPAT_ALWAYS_INLINE static auto InvokePixelOpImpl(F&& f, uint32_t /*x*/, uint32_t /*y*/, P& pixel, priority_tag<4>)
    -> decltype(f(pixel), void()) {
    f(pixel);
}

template <typename F, typename P>
COMPAT_ALWAYS_INLINE static auto InvokePixelOpImpl(F&& f, uint32_t x, uint32_t y, P& pixel, priority_tag<3>)
    -> decltype(f(x, y, pixel), void()) {
    f(x, y, pixel);
}

template <typename F, typename P>
COMPAT_ALWAYS_INLINE static auto InvokePixelOpImpl(F&& f, uint32_t x, uint32_t y, P& pixel, priority_tag<2>)
    -> decltype(f(x, y, std::declval<core::Color&>()), void()) {
    core::Color c = core::Color::FromArgb(pixel);
    f(x, y, c);
    AssignColorToPixel(pixel, c.ToArgb());
}

template <typename F, typename P>
COMPAT_ALWAYS_INLINE static auto InvokePixelOpImpl(F&& f, uint32_t x, uint32_t y, P& pixel, priority_tag<1>)
    -> decltype(f(x, y, std::declval<const core::Color&>()), void()) {
    core::Color c = core::Color::FromArgb(pixel);
    f(x, y, c);
}

template <typename F, typename P>
COMPAT_ALWAYS_INLINE static void InvokePixelOp(F&& f, uint32_t x, uint32_t y, P& pixel) {
    InvokePixelOpImpl(std::forward<F>(f), x, y, pixel, priority_tag<4>{});
}

// InvokePred 重載鏈
template <typename Pred, typename P>
COMPAT_ALWAYS_INLINE static auto InvokePredImpl(Pred&& pred, uint32_t /*x*/, uint32_t /*y*/, const P& pixel, priority_tag<3>)
    -> decltype(bool(pred(pixel))) {
    return static_cast<bool>(pred(pixel));
}

template <typename Pred, typename P>
COMPAT_ALWAYS_INLINE static auto InvokePredImpl(Pred&& pred, uint32_t x, uint32_t y, const P& pixel, priority_tag<2>)
    -> decltype(bool(pred(x, y, pixel))) {
    return static_cast<bool>(pred(x, y, pixel));
}

template <typename Pred, typename P>
COMPAT_ALWAYS_INLINE static auto InvokePredImpl(Pred&& pred, uint32_t x, uint32_t y, const P& pixel, priority_tag<1>)
    -> decltype(bool(pred(x, y, core::Color::FromArgb(pixel)))) {
    return static_cast<bool>(pred(x, y, core::Color::FromArgb(pixel)));
}

template <typename Pred, typename P>
COMPAT_ALWAYS_INLINE static bool InvokePred(Pred&& pred, uint32_t x, uint32_t y, const P& pixel) {
    return InvokePredImpl(std::forward<Pred>(pred), x, y, pixel, priority_tag<3>{});
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
        if (span.size() < static_cast<size_t>(static_cast<uint64_t>(width) * height)) return;
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
        if (span.size() < static_cast<size_t>(static_cast<uint64_t>(width) * height)) return;
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
        if (span.size() < static_cast<size_t>(static_cast<uint64_t>(width) * height)) return;
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
        if (span.size() < static_cast<size_t>(static_cast<uint64_t>(width) * height)) return;
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
