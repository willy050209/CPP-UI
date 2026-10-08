#pragma once

#include "compat/compat.hpp"
#include <cstdint>

namespace cppui {
namespace app {

/// <summary>
/// 應用程式圖形與渲染實作平台列舉 (PlatformBackend)
/// </summary>
enum class PlatformBackend : uint8_t {
    Win32,  // 預設 Windows GDI / 軟體光柵緩衝區後端
    Skia    // Google Skia 向量畫布渲染後端
};

} // namespace app
} // namespace cppui
