/**
 * @file DrawOp.hpp
 * @brief 繪圖指令與操作負載二進位結構定義 (DrawOpType, DrawOpHeader, DrawRectOp 等)
 */

#pragma once

#include <cstdint>
#include <type_traits>
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

    DrawRectOp() noexcept = default;
    DrawRectOp(const core::Rect& r, const core::Color& c) noexcept
        : rect(r), color(c) {}
};

/// <summary>
/// 圓角矩形繪製操作負載
/// </summary>
struct DrawRoundedRectOp {
    core::Rect  rect;
    float       radius = 0.0f;
    core::Color color;

    DrawRoundedRectOp() noexcept = default;
    DrawRoundedRectOp(const core::Rect& r, float rad, const core::Color& c) noexcept
        : rect(r), radius(rad), color(c) {}
};

/// <summary>
/// 線段繪製操作負載
/// </summary>
struct DrawLineOp {
    core::Point p1;
    core::Point p2;
    core::Color color;
    float       strokeWidth = 1.0f;

    DrawLineOp() noexcept = default;
    DrawLineOp(const core::Point& pt1, const core::Point& pt2, const core::Color& c, float width = 1.0f) noexcept
        : p1(pt1), p2(pt2), color(c), strokeWidth(width) {}
};

/// <summary>
/// 剪裁矩形壓棧操作負載
/// </summary>
struct PushClipRectOp {
    core::Rect rect;

    PushClipRectOp() noexcept = default;
    explicit PushClipRectOp(const core::Rect& r) noexcept : rect(r) {}
};

/// <summary>
/// 透明度壓棧操作負載
/// </summary>
struct PushOpacityOp {
    float opacity = 1.0f;

    PushOpacityOp() noexcept = default;
    explicit PushOpacityOp(float op) noexcept : opacity(op) {}
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

    DrawTextOp() noexcept = default;
    DrawTextOp(core::Point orig, core::Color col, float fSize = 14.0f, uint16_t fWeight = 400,
               uint8_t fStyle = 0, uint32_t famLen = 0, uint32_t txtLen = 0) noexcept
        : origin(orig), color(col), fontSize(fSize), fontWeight(fWeight),
          fontStyle(fStyle), reserved(0), familyLength(famLen), textLength(txtLen) {}
};

/// <summary>
/// 多行 TextLayout 排版繪製操作負載
/// </summary>
struct DrawTextLayoutOp {
    core::Point origin;
    core::Color color;
    void*       nativeLayoutHandle = nullptr;

    DrawTextLayoutOp() noexcept = default;
    DrawTextLayoutOp(core::Point orig, core::Color col, void* handle = nullptr) noexcept
        : origin(orig), color(col), nativeLayoutHandle(handle) {}
};

/// <summary>
/// 橢圓描邊繪製操作負載
/// </summary>
struct DrawEllipseOp {
    core::Rect  rect;
    core::Color color;
    float       strokeWidth = 1.0f;

    DrawEllipseOp() noexcept = default;
    DrawEllipseOp(const core::Rect& r, const core::Color& c, float width = 1.0f) noexcept
        : rect(r), color(c), strokeWidth(width) {}
};

/// <summary>
/// 橢圓填滿繪製操作負載
/// </summary>
struct FillEllipseOp {
    core::Rect  rect;
    core::Color color;

    FillEllipseOp() noexcept = default;
    FillEllipseOp(const core::Rect& r, const core::Color& c) noexcept
        : rect(r), color(c) {}
};

/// <summary>
/// 幾何路徑描邊繪製操作負載
/// </summary>
struct DrawPathOp {
    core::Color color;
    float       strokeWidth = 1.0f;
    uint32_t    pointCount  = 0;

    DrawPathOp() noexcept = default;
    DrawPathOp(core::Color col, float width = 1.0f, uint32_t count = 0) noexcept
        : color(col), strokeWidth(width), pointCount(count) {}
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

    FillPathOp() noexcept = default;
};

/// <summary>
/// 點陣影像繪製操作負載
/// </summary>
struct DrawImageOp {
    core::Rect  destRect;
    core::Rect  srcRect;
    const void* bitmapPtr = nullptr;
    float       opacity   = 1.0f;

    DrawImageOp() noexcept = default;
    DrawImageOp(const core::Rect& dest, const core::Rect& src, const void* bmp = nullptr, float op = 1.0f) noexcept
        : destRect(dest), srcRect(src), bitmapPtr(bmp), opacity(op) {}
};

#pragma pack(pop)

// 靜態斷言：驗證所有繪圖指令負載結構皆為平凡解構 (Trivially Destructible)，防範未呼叫解構子造成記憶體/GPU資源洩漏
static_assert(std::is_trivially_destructible<DrawOpHeader>::value, "DrawOpHeader must be trivially destructible");
static_assert(std::is_trivially_destructible<DrawRectOp>::value, "DrawRectOp must be trivially destructible");
static_assert(std::is_trivially_destructible<DrawRoundedRectOp>::value, "DrawRoundedRectOp must be trivially destructible");
static_assert(std::is_trivially_destructible<DrawLineOp>::value, "DrawLineOp must be trivially destructible");
static_assert(std::is_trivially_destructible<PushClipRectOp>::value, "PushClipRectOp must be trivially destructible");
static_assert(std::is_trivially_destructible<PushOpacityOp>::value, "PushOpacityOp must be trivially destructible");
static_assert(std::is_trivially_destructible<DrawTextOp>::value, "DrawTextOp must be trivially destructible");
static_assert(std::is_trivially_destructible<DrawTextLayoutOp>::value, "DrawTextLayoutOp must be trivially destructible");
static_assert(std::is_trivially_destructible<DrawEllipseOp>::value, "DrawEllipseOp must be trivially destructible");
static_assert(std::is_trivially_destructible<FillEllipseOp>::value, "FillEllipseOp must be trivially destructible");
static_assert(std::is_trivially_destructible<DrawPathOp>::value, "DrawPathOp must be trivially destructible");
static_assert(std::is_trivially_destructible<FillPathOp>::value, "FillPathOp must be trivially destructible");
static_assert(std::is_trivially_destructible<DrawImageOp>::value, "DrawImageOp must be trivially destructible");

} // namespace rendering
} // namespace cppui

