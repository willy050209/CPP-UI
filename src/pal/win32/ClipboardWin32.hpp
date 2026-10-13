/*
 * File: ClipboardWin32.hpp
 * Purpose: Declares Win32 native clipboard implementation using Win32 Clipboard API.
 */

#pragma once

#include "ui/pal/IClipboard.hpp"

namespace cppui {
namespace pal {

/// <summary>
/// Win32 原生剪貼簿實作 (ClipboardWin32)
/// 封裝 Win32 Clipboard API 與 UTF-8/UTF-16 雙向轉換。
/// </summary>
class ClipboardWin32 : public IClipboard {
public:
    /// <summary>預設建構子。</summary>
    ClipboardWin32() = default;

    /// <summary>虛擬解構子。</summary>
    ~ClipboardWin32() override = default;

    /// <summary>查詢剪貼簿是否包含文字 (CF_UNICODETEXT)。</summary>
    /// <returns>若包含文字傳回 true。</returns>
    COMPAT_NODISCARD bool ContainsText() const override;

    /// <summary>自 Win32 剪貼簿讀取 Unicode 文字並轉為 UTF-8。</summary>
    /// <returns>UTF-8 字串。</returns>
    COMPAT_NODISCARD std::string GetText() const override;

    /// <summary>將 UTF-8 文字轉為 Unicode 並寫入 Win32 剪貼簿。</summary>
    /// <param name="text">字串視圖。</param>
    void SetText(compat::string_view text) override;

    /// <summary>清空 Win32 剪貼簿。</summary>
    void Clear() override;
};

} // namespace pal
} // namespace cppui
