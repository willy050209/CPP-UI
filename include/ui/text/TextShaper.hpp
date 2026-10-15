/*
 * File: TextShaper.hpp
 * Purpose: Declares the TextShaper and ShapedRun classes for OpenType glyph shaping,
 *          metric calculations, advances, and cluster mapping.
 */

#pragma once

#include <vector>
#include <memory>
#include "compat/Compat.hpp"
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
/// 封裝單一字型、單一書寫方向下的連續字形度量與排版數據。
/// </summary>
class ShapedRun {
public:
    /// <summary>
    /// 預設建構 ShapedRun。
    /// </summary>
    ShapedRun() = default;

    /// <summary>
    /// 建構帶有字型、流向與字形清單的 ShapedRun。
    /// </summary>
    /// <param name="font">使用的字型描述符。</param>
    /// <param name="isRtl">是否為由右至左書寫。</param>
    /// <param name="totalWidth">總排版推進寬度。</param>
    /// <param name="glyphs">已塑形字形陣列。</param>
    ShapedRun(const FontDescriptor& font, bool isRtl, float totalWidth, std::vector<ShapedGlyph> glyphs)
        : m_font(font)
        , m_isRtl(isRtl)
        , m_totalWidth(totalWidth)
        , m_glyphs(std::move(glyphs)) {}

    /// <summary>
    /// 取得塑形切片所屬之字型描述符。
    /// </summary>
    /// <returns>字型描述符引用。</returns>
    COMPAT_NODISCARD const FontDescriptor& GetFont() const noexcept {
        return m_font;
    }

    /// <summary>
    /// 檢查是否為由右至左書寫。
    /// </summary>
    /// <returns>若為 RTL 傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsRtl() const noexcept {
        return m_isRtl;
    }

    /// <summary>
    /// 取得此切片在排版上的總橫向寬度。
    /// </summary>
    /// <returns>總推進寬度。</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 float GetTotalWidth() const noexcept {
        return m_totalWidth;
    }

    /// <summary>
    /// 取得塑形字形陣列之唯讀 span 視圖。
    /// </summary>
    /// <returns>ShapedGlyph 唯讀 span。</returns>
    COMPAT_NODISCARD compat::span<const ShapedGlyph> GetGlyphs() const noexcept {
        return compat::span<const ShapedGlyph>(m_glyphs.data(), m_glyphs.size());
    }

    /// <summary>
    /// 取得包含之字形總數。
    /// </summary>
    /// <returns>字形數量。</returns>
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
/// 負責調用平台底層 (如 DirectWrite / HarfBuzz / CoreText) 進行字符映射與幾何度量計算。
/// </summary>
class TextShaper {
public:
    /// <summary>
    /// 建構字形塑形器實例。
    /// </summary>
    TextShaper();

    /// <summary>
    /// 解構字形塑形器實例。
    /// </summary>
    ~TextShaper();

    // 禁止複製
    TextShaper(const TextShaper&) = delete;
    TextShaper& operator=(const TextShaper&) = delete;

    /// <summary>
    /// 將 UTF-8 字串塑形為帶有幾何度量的 ShapedRun 列表。
    /// </summary>
    /// <param name="text">待塑形之文字字串視圖。</param>
    /// <param name="font">排版所採用的字型描述符。</param>
    /// <returns>塑形完成之 ShapedRun 向量清單。</returns>
    COMPAT_NODISCARD std::vector<ShapedRun> Shape(compat::string_view text, const FontDescriptor& font) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace text
} // namespace cppui
