/**
 * @file Font.hpp
 * @brief 字型規格定義、字型度量 (FontMetrics) 與描述項 (FontDescriptor)
 */

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

    /// <summary>計算單行總高度 (ascent + descent + lineGap)。</summary>
    /// <returns>行高數值。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetLineHeight() const noexcept {
        return ascent + descent + lineGap;
    }
};

/// <summary>
/// 字型描述規格 (FontDescriptor)
/// </summary>
class FontDescriptor {
public:
    /// <summary>預設建構子，建立 Segoe UI 14pt 字型描述項。</summary>
    FontDescriptor()
        : m_family("Segoe UI")
        , m_size(14.0f)
        , m_weight(FontWeight::Normal)
        , m_style(FontStyle::Normal)
        , m_stretch(FontStretch::Normal) {}

    /// <summary>
    /// 建構完整的字型描述項。
    /// </summary>
    /// <param name="family">字型家族名稱視圖。</param>
    /// <param name="size">字型大小。</param>
    /// <param name="weight">字型粗細，預設為 Normal。</param>
    /// <param name="style">字型樣式，預設為 Normal。</param>
    /// <param name="stretch">橫向寬度伸展，預設為 Normal。</param>
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

    /// <summary>取得字型家族名稱。</summary>
    /// <returns>字型家族字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetFamily() const noexcept {
        return compat::string_view(m_family.data(), m_family.size());
    }

    /// <summary>取得字型大小。</summary>
    /// <returns>字型大小像素值。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetSize() const noexcept {
        return m_size;
    }

    /// <summary>取得字型粗細。</summary>
    /// <returns>FontWeight 列舉值。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 FontWeight GetWeight() const noexcept {
        return m_weight;
    }

    /// <summary>取得字型樣式。</summary>
    /// <returns>FontStyle 列舉值。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 FontStyle GetStyle() const noexcept {
        return m_style;
    }

    /// <summary>取得字型寬度伸展度。</summary>
    /// <returns>FontStretch 列舉值。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 FontStretch GetStretch() const noexcept {
        return m_stretch;
    }

    /// <summary>設定字型大小。</summary>
    /// <param name="size">字型大小像素值。</param>
    void SetSize(float size) noexcept { m_size = size; }

    /// <summary>設定字型家族名稱。</summary>
    /// <param name="family">字型家族名稱視圖。</param>
    void SetFamily(compat::string_view family) { m_family.assign(family.data(), family.size()); }

    /// <summary>設定字型粗細。</summary>
    /// <param name="weight">FontWeight 列舉值。</param>
    void SetWeight(FontWeight weight) noexcept { m_weight = weight; }

    /// <summary>設定字型樣式。</summary>
    /// <param name="style">FontStyle 列舉值。</param>
    void SetStyle(FontStyle style) noexcept { m_style = style; }

    /// <summary>設定字型伸展度。</summary>
    /// <param name="stretch">FontStretch 列舉值。</param>
    void SetStretch(FontStretch stretch) noexcept { m_stretch = stretch; }

    /// <summary>複製並產生指定大小之新字型描述項。</summary>
    /// <param name="size">新字型大小。</param>
    /// <returns>新 FontDescriptor 副本。</returns>
    FontDescriptor WithSize(float size) const {
        FontDescriptor copy = *this;
        copy.m_size = size;
        return copy;
    }

    /// <summary>複製並產生指定粗細之新字型描述項。</summary>
    /// <param name="weight">新字型粗細。</param>
    /// <returns>新 FontDescriptor 副本。</returns>
    FontDescriptor WithWeight(FontWeight weight) const {
        FontDescriptor copy = *this;
        copy.m_weight = weight;
        return copy;
    }

    /// <summary>複製並產生指定樣式之新字型描述項。</summary>
    /// <param name="style">新字型樣式。</param>
    /// <returns>新 FontDescriptor 副本。</returns>
    FontDescriptor WithStyle(FontStyle style) const {
        FontDescriptor copy = *this;
        copy.m_style = style;
        return copy;
    }

    /// <summary>相等比較運算子。</summary>
    /// <param name="other">另一個字型描述項。</param>
    /// <returns>若所有屬性相同傳回 true。</returns>
    bool operator==(const FontDescriptor& other) const noexcept {
        return m_size == other.m_size &&
               m_weight == other.m_weight &&
               m_style == other.m_style &&
               m_stretch == other.m_stretch &&
               m_family == other.m_family;
    }

    /// <summary>不相等比較運算子。</summary>
    /// <param name="other">另一個字型描述項。</param>
    /// <returns>若有任何屬性相異傳回 true。</returns>
    bool operator!=(const FontDescriptor& other) const noexcept {
        return !(*this == other);
    }

private:
    std::string m_family;
    float       m_size    = 14.0f;
    FontWeight  m_weight  = FontWeight::Normal;
    FontStyle   m_style   = FontStyle::Normal;
    FontStretch m_stretch = FontStretch::Normal;
};

/// <summary>字型規格類別別名 (Font -> FontDescriptor)。</summary>
using Font = FontDescriptor;

} // namespace text
} // namespace cppui
