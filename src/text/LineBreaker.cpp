#include "ui/text/LineBreaker.hpp"
#include <cstdint>

namespace cppui {
namespace text {

namespace {

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

// 判定是否為 CJK 表意字元 (允許字元間斷行)
bool IsCjkIdeograph(uint32_t cp) noexcept {
    return (cp >= 0x4E00 && cp <= 0x9FFF) ||
           (cp >= 0x3400 && cp <= 0x4DBF) ||
           (cp >= 0x20000 && cp <= 0x2A6DF) ||
           (cp >= 0x3040 && cp <= 0x30FF) || // 日文假名
           (cp >= 0xAC00 && cp <= 0xD7AF);   // 韓文音節
}

// 判定是否為行首禁止標點符號 (不能出現在行首，如 ，。、？！）
bool IsLineStartProhibited(uint32_t cp) noexcept {
    switch (cp) {
        case ',': case '.': case '!': case '?': case ':': case ';':
        case ')': case ']': case '}': case '>':
        case 0x3001: // 、
        case 0x3002: // 。
        case 0xFF0C: // ，
        case 0xFF0E: // ．
        case 0xFF01: // ！
        case 0xFF1F: // ？
        case 0xFF1A: // ：
        case 0xFF1B: // ；
        case 0x300D: // 」
        case 0x300F: // 』
        case 0x3011: // 】
        case 0xFF09: // ）
            return true;
        default:
            return false;
    }
}

// 判定是否為行尾禁止標點符號 (不能出現在行尾，如（「『【)
bool IsLineEndProhibited(uint32_t cp) noexcept {
    switch (cp) {
        case '(': case '[': case '{': case '<':
        case 0x300C: // 「
        case 0x300E: // 『
        case 0x3010: // 【
        case 0xFF08: // （
            return true;
        default:
            return false;
    }
}

} // anonymous namespace

struct LineBreaker::Impl {
    std::vector<LineBreak> FindBreaks(compat::string_view text) const {
        std::vector<LineBreak> breaks;
        if (text.empty()) return breaks;

        size_t pos = 0;
        uint32_t prevCp = 0;

        while (pos < text.size()) {
            size_t bytes = 0;
            uint32_t cp = DecodeUtf8(text.data() + pos, text.size() - pos, bytes);
            if (bytes == 0) break;

            BreakOpportunity opp = BreakOpportunity::None;

            if (cp == '\n') {
                opp = BreakOpportunity::MustBreak;
            } else if (prevCp == '\n') {
                opp = BreakOpportunity::CanBreak;
            } else if (prevCp == ' ' || prevCp == '\t') {
                // 空格後方允許折行
                opp = BreakOpportunity::CanBreak;
            } else if (IsCjkIdeograph(prevCp) && IsCjkIdeograph(cp)) {
                // 兩個中文字元之間允許折行
                opp = BreakOpportunity::CanBreak;
            } else if (IsCjkIdeograph(prevCp) && !IsLineStartProhibited(cp)) {
                // 中文字元之後，且下一個字元非行首禁止標點
                opp = BreakOpportunity::CanBreak;
            } else if (IsCjkIdeograph(cp) && !IsLineEndProhibited(prevCp)) {
                // 中文字元之前，且前一個字元非行尾禁止標點
                opp = BreakOpportunity::CanBreak;
            }

            // 特殊保護：若當前字元為行首禁止標點，則前置斷行機會強制為 None
            if (IsLineStartProhibited(cp)) {
                opp = BreakOpportunity::None;
            }
            // 若前一個字元為行尾禁止標點，則後置斷行機會強制為 None
            if (IsLineEndProhibited(prevCp)) {
                opp = BreakOpportunity::None;
            }

            if (pos > 0) {
                LineBreak lb;
                lb.position = pos;
                lb.opportunity = opp;
                breaks.push_back(lb);
            }

            prevCp = cp;
            pos += bytes;
        }

        // 字串結尾必能斷行
        LineBreak endBreak;
        endBreak.position = text.size();
        endBreak.opportunity = BreakOpportunity::CanBreak;
        breaks.push_back(endBreak);

        return breaks;
    }
};

LineBreaker::LineBreaker() : m_impl(new Impl()) {}
LineBreaker::~LineBreaker() = default;

std::vector<LineBreak> LineBreaker::FindBreaks(compat::string_view text) const {
    if (m_impl) {
        return m_impl->FindBreaks(text);
    }
    return {};
}

} // namespace text
} // namespace cppui
