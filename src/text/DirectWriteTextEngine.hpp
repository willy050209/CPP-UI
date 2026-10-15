#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwrite.h>
#endif

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "compat/Compat.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/TextLayout.hpp"

namespace cppui {
namespace text {

#ifdef _WIN32

/// <summary>
/// Windows 原生 DirectWrite 橋接中樞
/// </summary>
class DirectWriteTextEngine {
public:
    static DirectWriteTextEngine& Get();

    DirectWriteTextEngine();
    ~DirectWriteTextEngine();

    DirectWriteTextEngine(const DirectWriteTextEngine&) = delete;
    DirectWriteTextEngine& operator=(const DirectWriteTextEngine&) = delete;

    IDWriteFactory* GetFactory() const noexcept { return m_factory; }

    // 字串編碼轉換輔助
    static std::wstring Utf8ToUtf16(compat::string_view utf8Str);
    static std::string  Utf16ToUtf8(const wchar_t* utf16Str, size_t length);

    // 字型格式取得與快取
    IDWriteTextFormat* GetOrCreateTextFormat(const FontDescriptor& font);

    // 字型度量查詢
    FontMetrics GetFontMetrics(const FontDescriptor& font);

    // 建立底層原生 DirectWrite TextLayout
    IDWriteTextLayout* CreateNativeTextLayout(
        compat::string_view text,
        const FontDescriptor& font,
        float maxWidth,
        float maxHeight,
        TextAlignment alignment,
        TextWrapping wrapping,
        TextTrimming trimming
    );

private:
    IDWriteFactory* m_factory = nullptr;
    std::unordered_map<std::string, IDWriteTextFormat*> m_formatCache;
};

#endif // _WIN32

} // namespace text
} // namespace cppui
