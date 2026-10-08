/*
 * File: BiDiAnalyzer.cpp
 * Purpose: Implements bidirectional text analysis and Unicode script segmenting.
 */

#include "ui/text/BiDiAnalyzer.hpp"
#include <cstdint>

namespace cppui {
namespace text {

namespace {

// 判定 Unicode CodePoint 是否為強 RTL 字元 (希伯來文、阿拉伯文、敘利亞文等)
bool IsRtlCodePoint(uint32_t cp) noexcept {
    // 希伯來文: 0x0590 ~ 0x05FF
    if (cp >= 0x0590 && cp <= 0x05FF) return true;
    // 阿拉伯文: 0x0600 ~ 0x08FF
    if (cp >= 0x0600 && cp <= 0x08FF) return true;
    // 阿拉伯文展示形式 A & B: 0xFB50 ~ 0xFDFF, 0xFE70 ~ 0xFEFF
    if (cp >= 0xFB50 && cp <= 0xFDFF) return true;
    if (cp >= 0xFE70 && cp <= 0xFEFF) return true;
    return false;
}

// 判定是否為強 LTR 字元 (拉丁、希臘、西里爾、中日韓等)
bool IsLtrCodePoint(uint32_t cp) noexcept {
    if ((cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z')) return true;
    // 擴充拉丁、希臘、西里爾
    if (cp >= 0x00C0 && cp <= 0x02AF) return true;
    if (cp >= 0x0370 && cp <= 0x052F) return true;
    // CJK 統一表意文字
    if (cp >= 0x4E00 && cp <= 0x9FFF) return true;
    if (cp >= 0x3400 && cp <= 0x4DBF) return true;
    // 日文平假名、片假名、韓文音節
    if (cp >= 0x3040 && cp <= 0x30FF) return true;
    if (cp >= 0xAC00 && cp <= 0xD7AF) return true;
    return false;
}

// 解碼單一 UTF-8 字元
uint32_t DecodeUtf8(const char* ptr, size_t remaining, size_t& outBytes) {
    if (remaining == 0) {
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
        return ((c0 & 0x1F) << 6) | (static_cast<uint8_t>(ptr[1]) & 0x3F);
    }
    if ((c0 & 0xF0) == 0xE0 && remaining >= 3) {
        outBytes = 3;
        return ((c0 & 0x0F) << 12) | ((static_cast<uint8_t>(ptr[1]) & 0x3F) << 6) | (static_cast<uint8_t>(ptr[2]) & 0x3F);
    }
    if ((c0 & 0xF8) == 0xF0 && remaining >= 4) {
        outBytes = 4;
        return ((c0 & 0x07) << 18) | ((static_cast<uint8_t>(ptr[1]) & 0x3F) << 12) | ((static_cast<uint8_t>(ptr[2]) & 0x3F) << 6) | (static_cast<uint8_t>(ptr[3]) & 0x3F);
    }
    outBytes = 1;
    return c0;
}

} // anonymous namespace

struct BiDiAnalyzer::Impl {
    std::vector<BiDiRun> Analyze(compat::string_view text, BiDiDirection defaultDir) const {
        std::vector<BiDiRun> runs;
        if (text.empty()) return runs;

        bool inRtl = (defaultDir == BiDiDirection::RightToLeft);
        size_t runStart = 0;
        size_t pos = 0;

        while (pos < text.size()) {
            size_t bytes = 0;
            uint32_t cp = DecodeUtf8(text.data() + pos, text.size() - pos, bytes);
            if (bytes == 0) break;

            bool isRtlChar = IsRtlCodePoint(cp);
            bool isLtrChar = IsLtrCodePoint(cp);

            if (isRtlChar && !inRtl) {
                // 由 LTR 切換為 RTL
                if (pos > runStart) {
                    BiDiRun r;
                    r.startIndex = runStart;
                    r.length = pos - runStart;
                    r.embeddingLevel = 0;
                    r.isRtl = false;
                    runs.push_back(r);
                }
                runStart = pos;
                inRtl = true;
            } else if (isLtrChar && inRtl) {
                // 由 RTL 切換為 LTR
                if (pos > runStart) {
                    BiDiRun r;
                    r.startIndex = runStart;
                    r.length = pos - runStart;
                    r.embeddingLevel = 1;
                    r.isRtl = true;
                    runs.push_back(r);
                }
                runStart = pos;
                inRtl = false;
            }

            pos += bytes;
        }

        if (pos > runStart) {
            BiDiRun r;
            r.startIndex = runStart;
            r.length = pos - runStart;
            r.embeddingLevel = inRtl ? 1 : 0;
            r.isRtl = inRtl;
            runs.push_back(r);
        }

        return runs;
    }

    BiDiDirection DetectBaseDirection(compat::string_view text) const {
        bool hasLtr = false;
        bool hasRtl = false;
        size_t pos = 0;

        while (pos < text.size()) {
            size_t bytes = 0;
            uint32_t cp = DecodeUtf8(text.data() + pos, text.size() - pos, bytes);
            if (bytes == 0) break;

            if (IsRtlCodePoint(cp)) hasRtl = true;
            else if (IsLtrCodePoint(cp)) hasLtr = true;

            if (hasLtr && hasRtl) return BiDiDirection::Mixed;
            pos += bytes;
        }

        if (hasRtl) return BiDiDirection::RightToLeft;
        return BiDiDirection::LeftToRight;
    }
};

BiDiAnalyzer::BiDiAnalyzer() : m_impl(std::make_unique<Impl>()) {}
BiDiAnalyzer::~BiDiAnalyzer() = default;

std::vector<BiDiRun> BiDiAnalyzer::Analyze(compat::string_view text, BiDiDirection defaultDir) const {
    if (m_impl) {
        return m_impl->Analyze(text, defaultDir);
    }
    return {};
}

BiDiDirection BiDiAnalyzer::DetectBaseDirection(compat::string_view text) const {
    if (m_impl) {
        return m_impl->DetectBaseDirection(text);
    }
    return BiDiDirection::LeftToRight;
}

} // namespace text
} // namespace cppui
