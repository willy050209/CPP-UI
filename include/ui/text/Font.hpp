#pragma once

#include <cstdint>
#include <string>
#include "compat/compat.hpp"

namespace cppui {
namespace text {

/// <summary>
/// 字型粗細等級 (對齊 OpenType 與 CSS 規格)
/// </summary>
enum class FontWeight : uint16_t {
    Thin        = 100,
    ExtraLight  = 200,
    Light       = 300,
    SemiLight   = 350,
    Normal      = 400,
    Regular     = 400,
    Medium      = 500,
    SemiBold    = 600,
    Bold        = 700,
    ExtraBold   = 800,
    Black       = 900,
    ExtraBlack  = 950
};

/// <summary>
/// 字型樣式傾斜度
/// </summary>
enum class FontStyle : uint8_t {
    Normal  = 0,
    Oblique = 1,
    Italic  = 2
};

/// <summary>
/// 字型橫向寬度伸展等級
/// </summary>
enum class FontStretch : uint8_t {
    Undefined      = 0,
    UltraCondensed = 1,
    ExtraCondensed = 2,
    Condensed      = 3,
    SemiCondensed  = 4,
    Normal         = 5,
    Medium         = 5,
    SemiExpanded   = 6,
    Expanded       = 7,
    ExtraExpanded  = 8,
    UltraExpanded  = 9
};

/// <summary>
/// 字型度量測量數據 (以 DIP / 浮點數表示)
/// </summary>
struct FontMetrics {
    float ascent             = 0.0f; // 基線以上的高度 (通常為正值)
    float descent            = 0.0f; // 基線以下的高度 (通常為正值)
    float lineGap            = 0.0f; // 行間距
    float capHeight          = 0.0f; // 大寫字母頂部高度
    float xHeight            = 0.0f; // 小寫 x 頂部高度
    float underlinePosition  = 0.0f; // 底線相對於基線之垂直偏移
    float underlineThickness = 1.0f; // 底線筆刷粗細

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetLineHeight() const noexcept {
        return ascent + descent + lineGap;
    }
};

/// <summary>
/// 字型描述規格 (FontDescriptor)
/// </summary>
class FontDescriptor {
public:
    FontDescriptor()
        : m_family("Segoe UI")
        , m_size(14.0f)
        , m_weight(FontWeight::Normal)
        , m_style(FontStyle::Normal)
        , m_stretch(FontStretch::Normal) {}

    FontDescriptor(compat::string_view family,
                   float size,
                   FontWeight weight = FontWeight::Normal,
                   FontStyle style = FontStyle::Normal,
                   FontStretch stretch = FontStretch::Normal)
        : m_family(family.data(), family.size())
        , m_size(size)
        , m_weight(weight)
        , m_style(style)
        , m_stretch(stretch) {}

    COMPAT_NODISCARD compat::string_view GetFamily() const noexcept {
        return compat::string_view(m_family.data(), m_family.size());
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetSize() const noexcept {
        return m_size;
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 FontWeight GetWeight() const noexcept {
        return m_weight;
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 FontStyle GetStyle() const noexcept {
        return m_style;
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 FontStretch GetStretch() const noexcept {
        return m_stretch;
    }

    FontDescriptor WithSize(float size) const {
        FontDescriptor copy = *this;
        copy.m_size = size;
        return copy;
    }

    FontDescriptor WithWeight(FontWeight weight) const {
        FontDescriptor copy = *this;
        copy.m_weight = weight;
        return copy;
    }

    FontDescriptor WithStyle(FontStyle style) const {
        FontDescriptor copy = *this;
        copy.m_style = style;
        return copy;
    }

private:
    std::string m_family;
    float       m_size    = 14.0f;
    FontWeight  m_weight  = FontWeight::Normal;
    FontStyle   m_style   = FontStyle::Normal;
    FontStretch m_stretch = FontStretch::Normal;
};

} // namespace text
} // namespace cppui
