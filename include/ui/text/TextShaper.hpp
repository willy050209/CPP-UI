#pragma once

#include <vector>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/text/Font.hpp"

namespace cppui {
namespace text {

/// <summary>
/// 單一塑形字形資訊 (Shaped Glyph)
/// </summary>
struct ShapedGlyph {
    uint16_t glyphId      = 0;    // OpenType 字形識別碼
    uint32_t clusterIndex = 0;    // 原 UTF-8/UTF-16 字串群集起始索引
    float    xAdvance     = 0.0f; // 橫向推進寬度
    float    yAdvance     = 0.0f; // 縱向推進高度
    float    xOffset      = 0.0f; // 橫向視覺偏移
    float    yOffset      = 0.0f; // 縱向視覺偏移
};

/// <summary>
/// 連續塑形字符切片 (Shaped Run)
/// </summary>
class ShapedRun {
public:
    ShapedRun() = default;

    ShapedRun(const FontDescriptor& font, bool isRtl, float totalWidth, std::vector<ShapedGlyph> glyphs)
        : m_font(font)
        , m_isRtl(isRtl)
        , m_totalWidth(totalWidth)
        , m_glyphs(std::move(glyphs)) {}

    COMPAT_NODISCARD const FontDescriptor& GetFont() const noexcept {
        return m_font;
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsRtl() const noexcept {
        return m_isRtl;
    }

    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetTotalWidth() const noexcept {
        return m_totalWidth;
    }

    COMPAT_NODISCARD compat::span<const ShapedGlyph> GetGlyphs() const noexcept {
        return compat::span<const ShapedGlyph>(m_glyphs.data(), m_glyphs.size());
    }

    COMPAT_NODISCARD size_t GetGlyphCount() const noexcept {
        return m_glyphs.size();
    }

private:
    FontDescriptor           m_font;
    bool                     m_isRtl      = false;
    float                    m_totalWidth = 0.0f;
    std::vector<ShapedGlyph> m_glyphs;
};

/// <summary>
/// 跨平台字形塑形器 (TextShaper)
/// </summary>
class TextShaper {
public:
    TextShaper();
    ~TextShaper();

    // 禁止複製
    TextShaper(const TextShaper&) = delete;
    TextShaper& operator=(const TextShaper&) = delete;

    /// <summary>
    /// 將 UTF-8 字串塑形為帶有幾何度量的 ShapedRun 列表
    /// </summary>
    COMPAT_NODISCARD std::vector<ShapedRun> Shape(compat::string_view text, const FontDescriptor& font) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace text
} // namespace cppui
