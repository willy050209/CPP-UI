/*
 * File: DirectWriteTextEngine.cpp
 * Purpose: Implements the DirectWrite text layout, shaping, and measurement engine for Windows.
 */

#include "DirectWriteTextEngine.hpp"
#include <cstdint>

#ifdef _WIN32

#pragma comment(lib, "dwrite.lib")

namespace cppui {
namespace text {

DirectWriteTextEngine& DirectWriteTextEngine::Get() {
    static DirectWriteTextEngine instance;
    return instance;
}

DirectWriteTextEngine::DirectWriteTextEngine() {
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_factory)
    );
    (void)hr;
}

DirectWriteTextEngine::~DirectWriteTextEngine() {
    for (auto& pair : m_formatCache) {
        if (pair.second) {
            pair.second->Release();
        }
    }
    m_formatCache.clear();

    if (m_factory) {
        m_factory->Release();
        m_factory = nullptr;
    }
}

std::wstring DirectWriteTextEngine::Utf8ToUtf16(compat::string_view utf8Str) {
    if (utf8Str.empty()) return std::wstring();

    int32_t len = MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8Str.data(),
        static_cast<int32_t>(utf8Str.size()),
        nullptr,
        0
    );
    if (len <= 0) return std::wstring();

    std::wstring result(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8Str.data(),
        static_cast<int32_t>(utf8Str.size()),
        &result[0],
        len
    );
    return result;
}

std::string DirectWriteTextEngine::Utf16ToUtf8(const wchar_t* utf16Str, size_t length) {
    if (!utf16Str || length == 0) return std::string();

    int32_t len = WideCharToMultiByte(
        CP_UTF8,
        0,
        utf16Str,
        static_cast<int32_t>(length),
        nullptr,
        0,
        nullptr,
        nullptr
    );
    if (len <= 0) return std::string();

    std::string result(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        utf16Str,
        static_cast<int32_t>(length),
        &result[0],
        len,
        nullptr,
        nullptr
    );
    return result;
}

IDWriteTextFormat* DirectWriteTextEngine::GetOrCreateTextFormat(const FontDescriptor& font) {
    if (!m_factory) return nullptr;

    // 依據字型屬性建構快取識別字串
    std::string key = compat::format("{}_{}_{}_{}_{}",
        font.GetFamily(),
        font.GetSize(),
        static_cast<uint16_t>(font.GetWeight()),
        static_cast<uint32_t>(font.GetStyle()),
        static_cast<uint32_t>(font.GetStretch()));

    auto it = m_formatCache.find(key);
    if (it != m_formatCache.end()) {
        return it->second;
    }

    std::wstring wideFamily = Utf8ToUtf16(font.GetFamily());
    DWRITE_FONT_WEIGHT weight = static_cast<DWRITE_FONT_WEIGHT>(font.GetWeight());
    DWRITE_FONT_STYLE style   = DWRITE_FONT_STYLE_NORMAL;
    if (font.GetStyle() == FontStyle::Italic) {
        style = DWRITE_FONT_STYLE_ITALIC;
    } else if (font.GetStyle() == FontStyle::Oblique) {
        style = DWRITE_FONT_STYLE_OBLIQUE;
    }
    DWRITE_FONT_STRETCH stretch = static_cast<DWRITE_FONT_STRETCH>(font.GetStretch());

    IDWriteTextFormat* pFormat = nullptr;
    HRESULT hr = m_factory->CreateTextFormat(
        wideFamily.c_str(),
        nullptr,
        weight,
        style,
        stretch,
        font.GetSize(),
        L"", // 預設系統地區語系
        &pFormat
    );

    if (SUCCEEDED(hr) && pFormat) {
        m_formatCache[key] = pFormat;
        return pFormat;
    }

    return nullptr;
}

FontMetrics DirectWriteTextEngine::GetFontMetrics(const FontDescriptor& font) {
    FontMetrics fm;
    IDWriteTextFormat* pFormat = GetOrCreateTextFormat(font);
    if (!pFormat || !m_factory) {
        // 退回預設估算度量 (與 FallbackFontHelper 規範對齊)
        fm.ascent = std::round(font.GetSize() * 0.8f);
        fm.descent = std::round(font.GetSize() * 0.2f);
        fm.lineGap = (std::max)(0.0f, std::ceil(font.GetSize() * 1.25f) - (fm.ascent + fm.descent));
        fm.capHeight = fm.ascent * 0.7f;
        fm.xHeight = fm.ascent * 0.5f;
        return fm;
    }

    // 透過 CreateTextLayout 查詢即時度量資訊
    const wchar_t dummy[] = L"Mg";
    IDWriteTextLayout* pLayout = nullptr;
    HRESULT hr = m_factory->CreateTextLayout(dummy, 2, pFormat, 100.0f, 100.0f, &pLayout);
    if (SUCCEEDED(hr) && pLayout) {
        DWRITE_LINE_METRICS lineMetrics;
        UINT32 actualCount = 0;
        pLayout->GetLineMetrics(&lineMetrics, 1, &actualCount);
        if (actualCount > 0) {
            fm.ascent = lineMetrics.baseline;
            fm.descent = lineMetrics.height - lineMetrics.baseline;
            fm.lineGap = (std::max)(0.0f, std::ceil(lineMetrics.height) - lineMetrics.height);
            fm.capHeight = lineMetrics.baseline * 0.7f;
            fm.xHeight = lineMetrics.baseline * 0.5f;
        }
        pLayout->Release();
    }
    return fm;
}

IDWriteTextLayout* DirectWriteTextEngine::CreateNativeTextLayout(
    compat::string_view text,
    const FontDescriptor& font,
    float maxWidth,
    float maxHeight,
    TextAlignment alignment,
    TextWrapping wrapping,
    TextTrimming trimming
) {
    if (!m_factory) return nullptr;

    IDWriteTextFormat* pFormat = GetOrCreateTextFormat(font);
    if (!pFormat) return nullptr;

    std::wstring wideText = Utf8ToUtf16(text);
    IDWriteTextLayout* pLayout = nullptr;

    HRESULT hr = m_factory->CreateTextLayout(
        wideText.c_str(),
        static_cast<UINT32>(wideText.size()),
        pFormat,
        (maxWidth > 0.0f) ? maxWidth : 10000.0f,
        (maxHeight > 0.0f) ? maxHeight : 10000.0f,
        &pLayout
    );

    if (FAILED(hr) || !pLayout) {
        return nullptr;
    }

    // 設定文字水平對齊
    switch (alignment) {
        case TextAlignment::Left:
            pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            break;
        case TextAlignment::Center:
            pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            break;
        case TextAlignment::Right:
            pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
            break;
        case TextAlignment::Justify:
            pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
            break;
    }

    // 設定自動折行模式
    switch (wrapping) {
        case TextWrapping::NoWrap:
            pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            break;
        case TextWrapping::WordWrap:
            pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
            break;
        case TextWrapping::CharacterWrap:
            pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_CHARACTER);
            break;
    }

    // 設定省略號 (Trimming)
    if (trimming != TextTrimming::None) {
        DWRITE_TRIMMING trimmingOptions = {};
        trimmingOptions.granularity = (trimming == TextTrimming::CharacterEllipsis)
                                    ? DWRITE_TRIMMING_GRANULARITY_CHARACTER
                                    : DWRITE_TRIMMING_GRANULARITY_WORD;
        IDWriteInlineObject* ellipsisSign = nullptr;
        m_factory->CreateEllipsisTrimmingSign(pFormat, &ellipsisSign);
        pLayout->SetTrimming(&trimmingOptions, ellipsisSign);
        if (ellipsisSign) {
            ellipsisSign->Release();
        }
    }

    return pLayout;
}

} // namespace text
} // namespace cppui

#endif // _WIN32
