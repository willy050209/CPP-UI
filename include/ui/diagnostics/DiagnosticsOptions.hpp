/**
 * @file DiagnosticsOptions.hpp
 * @brief 全域執行期診斷選項結構宣告
 */

#pragma once

#include <cstdint>
#include "compat/compat.hpp"

namespace cppui {
namespace diagnostics {

/// <summary>
/// 全域執行期診斷選項。
/// </summary>
struct DiagnosticsOptions {
    bool     enableDirtyRectOverlay = false;    // 啟用局部重繪高亮閃爍
    bool     showFpsCounter = false;           // 啟用幀率與繪圖指令統計
    bool     enableVisualTreeInspector = true; // 啟用 F12 Visual Tree Inspector
    uint32_t overlayDurationMs = 250;          // 重繪高亮淡出時間 (毫秒)
};

} // namespace diagnostics
} // namespace cppui
