#pragma once

#include <cstdint>
#include "compat/compat.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 水平對齊模式
/// </summary>
enum class HorizontalAlignment : uint8_t {
    Left = 0,
    Center = 1,
    Right = 2,
    Stretch = 3
};

/// <summary>
/// 垂直對齊模式
/// </summary>
enum class VerticalAlignment : uint8_t {
    Top = 0,
    Center = 1,
    Bottom = 2,
    Stretch = 3
};

/// <summary>
/// 元件可見性狀態
/// </summary>
enum class Visibility : uint8_t {
    /// <summary>正常顯示並參與排版</summary>
    Visible = 0,
    /// <summary>隱藏不渲染，但仍保留尺寸空間參與排版</summary>
    Hidden = 1,
    /// <summary>完全摺疊隱藏，不佔用任何排版空間 (DesiredSize = 0, 0)</summary>
    Collapsed = 2
};

/// <summary>
/// 排列方向 (用於 StackPanel 等線性容器)
/// </summary>
enum class Orientation : uint8_t {
    Horizontal = 0,
    Vertical = 1
};

} // namespace layout
} // namespace cppui
