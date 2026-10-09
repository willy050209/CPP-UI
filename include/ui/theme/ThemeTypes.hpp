/*
 * File: ThemeTypes.hpp
 * Purpose: Declares core theme modes and enumerations for the CPPUI styling system.
 */

#pragma once

#include <cstdint>

namespace cppui {
namespace theme {

/// <summary>
/// 主題運作模式列舉 (ThemeMode)
/// </summary>
enum class ThemeMode : uint32_t {
    Dark = 0,         // 暗色模式 (現代專業、高對比暗色調)
    Light = 1,        // 亮色模式 (清新簡潔、低眩光亮色調)
    HighContrast = 2, // 高對比模式 (無障礙輔助專用)
    Custom = 3        // 使用者完全自訂模式
};

} // namespace theme
} // namespace cppui
