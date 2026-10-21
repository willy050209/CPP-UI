/*
 * File: ClipboardWin32.cpp
 * Purpose: Implements Win32 native clipboard.
 */

#if defined(_WIN32) || defined(WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "ClipboardWin32.hpp"
#include <vector>

namespace cppui {
namespace pal {

namespace {

class ClipboardScope {
public:
    ClipboardScope() {
        for (int32_t retry = 0; retry < 10; ++retry) {
            if (::OpenClipboard(nullptr) != FALSE) {
                m_opened = true;
                break;
            }
            ::Sleep(10);
        }
    }
    ~ClipboardScope() {
        if (m_opened) {
            ::CloseClipboard();
        }
    }
    COMPAT_NODISCARD bool IsOpened() const noexcept { return m_opened; }

private:
    bool m_opened = false;
};

} // namespace

bool ClipboardWin32::ContainsText() const {
    if (::IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE) {
        return true;
    }
    return !m_fallbackText.empty();
}

std::string ClipboardWin32::GetText() const {
    ClipboardScope scope;
    if (!scope.IsOpened()) {
        return m_fallbackText;
    }

    HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
    if (!hData) {
        return m_fallbackText;
    }

    const wchar_t* wstr = static_cast<const wchar_t*>(::GlobalLock(hData));
    if (!wstr) {
        return m_fallbackText;
    }

    int32_t wlen = static_cast<int32_t>(wcslen(wstr));
    if (wlen == 0) {
        ::GlobalUnlock(hData);
        return m_fallbackText;
    }

    int32_t utf8Len = ::WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0) {
        ::GlobalUnlock(hData);
        return m_fallbackText;
    }

    std::string result(static_cast<size_t>(utf8Len), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, &result[0], utf8Len, nullptr, nullptr);

    ::GlobalUnlock(hData);
    return result;
}

void ClipboardWin32::SetText(compat::string_view text) {
    m_fallbackText = std::string(text.data(), text.size());
    ClipboardScope scope;
    if (!scope.IsOpened()) {
        return;
    }

    ::EmptyClipboard();

    if (text.empty()) return;

    int32_t textLen = static_cast<int32_t>(text.size());
    int32_t wlen = ::MultiByteToWideChar(CP_UTF8, 0, text.data(), textLen, nullptr, 0);
    if (wlen <= 0) return;

    size_t allocBytes = (static_cast<size_t>(wlen) + 1) * sizeof(wchar_t);
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, allocBytes);
    if (!hGlobal) return;

    wchar_t* dest = static_cast<wchar_t*>(::GlobalLock(hGlobal));
    if (!dest) {
        ::GlobalFree(hGlobal);
        return;
    }

    ::MultiByteToWideChar(CP_UTF8, 0, text.data(), textLen, dest, wlen);
    dest[wlen] = L'\0';
    ::GlobalUnlock(hGlobal);

    if (::SetClipboardData(CF_UNICODETEXT, hGlobal) == nullptr) {
        ::GlobalFree(hGlobal);
    }
}

void ClipboardWin32::Clear() {
    m_fallbackText.clear();
    ClipboardScope scope;
    if (!scope.IsOpened()) return;
    ::EmptyClipboard();
}

} // namespace pal
} // namespace cppui

#endif // _WIN32
