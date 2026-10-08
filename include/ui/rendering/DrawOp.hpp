/**
 * @file DrawOp.hpp
 * @brief 繪圖指令與操作負載二進位結構定義 (DrawOpType, DrawOpHeader, DrawRectOp 等)
 */

#pragma once

#include <cstdint>
#include "ui/core/Geometry.hpp"

namespace cppui {
namespace rendering {

#ifdef DrawText
#undef DrawText
#endif

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
    DrawText,
    DrawTextLayout,
    DrawEllipse,
    FillEllipse,
    DrawPath,
    FillPath,
    DrawImage
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

/// <summary>
/// 文字繪製操作負載 (含 UTF-8 字串內容與字型規格)
/// </summary>
struct DrawTextOp {
    core::Point origin;
    core::Color color;
    float       fontSize     = 14.0f;
    uint16_t    fontWeight   = 400;
    uint8_t     fontStyle    = 0;
    uint8_t     reserved     = 0;
    uint32_t    familyLength = 0;
    uint32_t    textLength   = 0;
};

/// <summary>
/// 多行 TextLayout 排版繪製操作負載
/// </summary>
struct DrawTextLayoutOp {
    core::Point origin;
    core::Color color;
    void*       nativeLayoutHandle = nullptr;
};

/// <summary>
/// 橢圓描邊繪製操作負載
/// </summary>
struct DrawEllipseOp {
    core::Rect  rect;
    core::Color color;
    float       strokeWidth = 1.0f;
};

/// <summary>
/// 橢圓填滿繪製操作負載
/// </summary>
struct FillEllipseOp {
    core::Rect  rect;
    core::Color color;
};

/// <summary>
/// 幾何路徑描邊繪製操作負載
/// </summary>
struct DrawPathOp {
    core::Color color;
    float       strokeWidth = 1.0f;
    uint32_t    pointCount  = 0;
};

/// <summary>
/// 幾何路徑填滿繪製操作負載
/// </summary>
struct FillPathOp {
    core::Color color;
    uint8_t     fillMode       = 0;
    uint8_t     hasGradient    = 0;
    uint16_t    reserved       = 0;
    uint32_t    pointCount     = 0;
    core::Rect  bounds;
    core::Point gradStart;
    core::Point gradEnd;
    core::Color gradStartColor;
    core::Color gradEndColor;
};

/// <summary>
/// 點陣影像繪製操作負載
/// </summary>
struct DrawImageOp {
    core::Rect  destRect;
    core::Rect  srcRect;
    const void* bitmapPtr = nullptr;
    float       opacity   = 1.0f;
};

#pragma pack(pop)

} // namespace rendering
} // namespace cppui
