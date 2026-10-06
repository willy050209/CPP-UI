#pragma once

#include <cstdint>
#include "ui/core/Geometry.hpp"

namespace cppui::rendering {

/// <summary>
/// 繪圖操作類型列舉
/// </summary>
enum class DrawOpType : uint16_t {
    None = 0,
    Save,
    Restore,
    PushClipRect,
    PopClip,
    PushOpacity,
    PopOpacity,
    DrawRectangle,
    DrawRoundedRectangle,
    DrawLine,
    DrawTextRun // 預留 M3 HarfBuzz/ICU 文字圖元
};

#pragma pack(push, 4)

/// <summary>
/// 繪圖操作二進位標頭 (緊湊 8 字節)
/// </summary>
struct DrawOpHeader {
    DrawOpType type;
    uint16_t   flags;
    uint32_t   size; // 包含 Header 本身與後續 Payload 的總字節長度
};

/// <summary>
/// 矩形繪製操作負載
/// </summary>
struct DrawRectOp {
    core::Rect  rect;
    core::Color color;
};

/// <summary>
/// 圓角矩形繪製操作負載
/// </summary>
struct DrawRoundedRectOp {
    core::Rect  rect;
    float       radius = 0.0f;
    core::Color color;
};

/// <summary>
/// 線段繪製操作負載
/// </summary>
struct DrawLineOp {
    core::Point p1;
    core::Point p2;
    core::Color color;
    float       strokeWidth = 1.0f;
};

/// <summary>
/// 剪裁矩形壓棧操作負載
/// </summary>
struct PushClipRectOp {
    core::Rect rect;
};

/// <summary>
/// 透明度壓棧操作負載
/// </summary>
struct PushOpacityOp {
    float opacity = 1.0f;
};

#pragma pack(pop)

} // namespace cppui::rendering
