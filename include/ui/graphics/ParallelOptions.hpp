/*
 * File: ParallelOptions.hpp
 * Purpose: Defines execution modes, thread counts, and window slicing options for image processing and convolution.
 */

#pragma once

#include <cstdint>
#include <thread>
#include <algorithm>

namespace cppui {
namespace graphics {

/// <summary>
/// 影像處理與卷積運算之執行緒模式列舉 (ExecutionMode)
/// </summary>
enum class ExecutionMode : uint8_t {
    SingleThread = 0, // 單執行緒循序執行 (保證零執行緒建立與切換開銷)
    MultiThread  = 1  // 多執行緒並行執行
};

/// <summary>
/// 影像運算並行化與視窗分塊配置參數結構 (ParallelOptions)
/// 完全由呼叫端明確指定，不進行系統黑盒子隱式干預。
/// </summary>
struct ParallelOptions {
    ExecutionMode mode{ ExecutionMode::SingleThread }; // 執行模式
    uint32_t      threadCount{ 1 };                    // 工作執行緒數量 (0 代表自動採用硬體核心數)
    uint32_t      sliceHeight{ 0 };                    // 列切分視窗大小 (0 代表高度均分，>0 代表自訂固定大小區塊)

    /// <summary>
    /// 建構標準單執行緒循序執行配置。
    /// </summary>
    /// <returns>已配置之 ParallelOptions 物件。</returns>
    static ParallelOptions Sequential() noexcept {
        ParallelOptions opt;
        opt.mode = ExecutionMode::SingleThread;
        opt.threadCount = 1;
        opt.sliceHeight = 0;
        return opt;
    }

    /// <summary>
    /// 建構多執行緒並行配置，可自訂執行緒數與視窗切分列高。
    /// </summary>
    /// <param name="threads">工作執行緒數 (傳入 0 代表自動依硬體並行核心數設定)。</param>
    /// <param name="chunkRows">視窗切分列高 (傳入 0 代表由高度均分，大於 0 代表以固定列高切割 Work Chunk)。</param>
    /// <returns>已配置之 ParallelOptions 物件。</returns>
    static ParallelOptions Parallel(uint32_t threads = 0, uint32_t chunkRows = 0) noexcept {
        ParallelOptions opt;
        opt.mode = ExecutionMode::MultiThread;
        uint32_t hw = static_cast<uint32_t>(std::thread::hardware_concurrency());
        opt.threadCount = (threads == 0) ? (std::max)(1u, hw) : threads;
        opt.sliceHeight = chunkRows;
        return opt;
    }
};

} // namespace graphics
} // namespace cppui
