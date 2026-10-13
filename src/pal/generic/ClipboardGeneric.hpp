/*
 * File: ClipboardGeneric.hpp
 * Purpose: Declares generic in-memory clipboard implementation for fallback and headless testing.
 */

#pragma once

#include <string>
#include <mutex>
#include "ui/pal/IClipboard.hpp"

namespace cppui {
namespace pal {

/// <summary>
/// 通用記憶體剪貼簿實作 (ClipboardGeneric)
/// 用於無頭測試 (Headless) 或平台剪貼簿未支援時之備援。
/// </summary>
class ClipboardGeneric : public IClipboard {
public:
    /// <summary>預設建構子。</summary>
    ClipboardGeneric() = default;

    /// <summary>虛擬解構子。</summary>
    ~ClipboardGeneric() override = default;

    /// <summary>查詢是否包含文字。</summary>
    /// <returns>若包含文字傳回 true。</returns>
    COMPAT_NODISCARD bool ContainsText() const override;

    /// <summary>取得記憶體中之文字。</summary>
    /// <returns>UTF-8 字串。</returns>
    COMPAT_NODISCARD std::string GetText() const override;

    /// <summary>設定文字至記憶體。</summary>
    /// <param name="text">字串視圖。</param>
    void SetText(compat::string_view text) override;

    /// <summary>清空文字。</summary>
    void Clear() override;

private:
    mutable std::mutex m_mutex;
    std::string m_text;
};

} // namespace pal
} // namespace cppui
