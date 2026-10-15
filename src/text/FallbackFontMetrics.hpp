/*
 * File: FallbackFontMetrics.hpp
 * Purpose: Provides unified glyph metrics, UTF-8 codepoint decoding, and scaling for cross-platform fallback rendering.
 */

#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include "compat/Compat.hpp"

namespace cppui {
namespace text {

/// <summary>
/// 跨平台純 C++ 備援字型度量結構
/// </summary>
struct FallbackFontMetrics {
    int32_t scale;
    float   charWidth;
    float   charHeight;
    float   lineHeight;
    float   baseline;
};

/// <summary>
/// 跨平台純 C++ 字型度量與 UTF-8 解碼純函數輔助類別
/// </summary>
class FallbackFontHelper {
public:
    /// <summary>
    /// 依據指定字級大小計算跨平台一致之點陣字型度量數值。
    /// </summary>
    /// <param name="fontSize">字型點陣/像素大小。</param>
    /// <returns>填妥之 FallbackFontMetrics 結構。</returns>
    COMPAT_NODISCARD static FallbackFontMetrics ComputeMetrics(float fontSize) noexcept {
        FallbackFontMetrics m{};
        // 標準 UI 字級 (<= 17px) 採用 scale = 1 (8x8 點陣字型，字寬 8px，契合標準拉丁字母 0.57x 比例)
        // 標題字級 (>= 18px) 採用整數倍率放大
        m.scale = (fontSize >= 18.0f) ? (std::max)(1, static_cast<int32_t>(std::round(fontSize / 10.0f))) : 1;
        m.charWidth = 8.0f * static_cast<float>(m.scale);
        m.charHeight = 8.0f * static_cast<float>(m.scale);
        m.lineHeight = fontSize * 1.25f;
        if (m.lineHeight < m.charHeight) {
            m.lineHeight = m.charHeight;
        }
        m.baseline = fontSize;
        return m;
    }

    /// <summary>
    /// 自 UTF-8 二進位緩衝區解碼單一 Unicode 碼點 (Codepoint)。
    /// </summary>
    /// <param name="ptr">字串二進位指標。</param>
    /// <param name="remaining">剩餘有效位元組數。</param>
    /// <param name="outBytes">傳出此碼點佔用之位元組數。</param>
    /// <returns>解碼後之 32-bit Unicode 碼點數值。</returns>
    COMPAT_NODISCARD static uint32_t DecodeUtf8(const char* ptr, size_t remaining, size_t& outBytes) noexcept {
        if (!ptr || remaining == 0) {
            outBytes = 0;
            return 0;
        }
        uint8_t c0 = static_cast<uint8_t>(ptr[0]);
        if (c0 < 0x80) {
            outBytes = 1;
            return c0;
        }
        if ((c0 & 0xE0) == 0xC0 && remaining >= 2) {
            outBytes = 2;
            return ((static_cast<uint32_t>(c0 & 0x1F)) << 6) | (static_cast<uint32_t>(static_cast<uint8_t>(ptr[1]) & 0x3F));
        }
        if ((c0 & 0xF0) == 0xE0 && remaining >= 3) {
            outBytes = 3;
            return ((static_cast<uint32_t>(c0 & 0x0F)) << 12) |
                   ((static_cast<uint32_t>(static_cast<uint8_t>(ptr[1]) & 0x3F)) << 6) |
                   (static_cast<uint32_t>(static_cast<uint8_t>(ptr[2]) & 0x3F));
        }
        if ((c0 & 0xF8) == 0xF0 && remaining >= 4) {
            outBytes = 4;
            return ((static_cast<uint32_t>(c0 & 0x07)) << 18) |
                   ((static_cast<uint32_t>(static_cast<uint8_t>(ptr[1]) & 0x3F)) << 12) |
                   ((static_cast<uint32_t>(static_cast<uint8_t>(ptr[2]) & 0x3F)) << 6) |
                   (static_cast<uint32_t>(static_cast<uint8_t>(ptr[3]) & 0x3F));
        }
        outBytes = 1;
        return c0;
    }

    /// <summary>
    /// 判斷特定 Unicode 碼點是否為全形字元或 CJK 表意字元 (佔用雙倍欄寬)。
    /// </summary>
    /// <param name="cp">32-bit Unicode 碼點。</param>
    /// <returns>若為全形或 CJK 字元傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD static bool IsFullwidth(uint32_t cp) noexcept {
        return (cp >= 0x4E00 && cp <= 0x9FFF) ||
               (cp >= 0x3400 && cp <= 0x4DBF) ||
               (cp >= 0x20000 && cp <= 0x2A6DF) ||
               (cp >= 0x3000 && cp <= 0x303F) || // CJK 符號與標點
               (cp >= 0x3040 && cp <= 0x30FF) || // 日文假名
               (cp >= 0xAC00 && cp <= 0xD7AF) || // 韓文音節
               (cp >= 0xFF00 && cp <= 0xFFEF);   // 全形 ASCII 與標點
    }
};

} // namespace text
} // namespace cppui
