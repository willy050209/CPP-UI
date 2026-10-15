/*
 * File: ClipboardX11.hpp
 * Purpose: Declares Linux X11 native clipboard implementation using X11 Selection mechanism.
 */

#pragma once

#if defined(CPPUI_HAS_X11) && CPPUI_HAS_X11

#include <cstdint>
#include <string>
#include <mutex>
#include "compat/Compat.hpp"
#include "ui/pal/IClipboard.hpp"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

// 清除 X11 巨集污染
#ifdef None
#undef None
#endif
#ifdef Status
#undef Status
#endif
#ifdef Bool
#undef Bool
#endif
#ifdef Success
#undef Success
#endif
#ifdef Always
#undef Always
#endif

namespace cppui {
namespace pal {
namespace x11 {

/// <summary>
/// Linux X11 原生剪貼簿實作類別 (ClipboardX11)
/// 實作 IClipboard 介面，使用 X11 Selection (CLIPBOARD/UTF8_STRING) 與系統其他應用程式交換文字，
/// 若於無 DISPLAY 伺服器之無頭環境運行，自動平滑降級為記憶體快取模式。
/// </summary>
class ClipboardX11 : public IClipboard {
public:
    /// <summary>建構子：初始化 X11 連線、Atom 與隱藏輔助視窗。</summary>
    ClipboardX11();

    /// <summary>解構子：釋放隱藏輔助視窗與 X11 連線資源。</summary>
    ~ClipboardX11() override;

    // 禁止拷貝
    ClipboardX11(const ClipboardX11&) = delete;
    ClipboardX11& operator=(const ClipboardX11&) = delete;

    /// <summary>查詢剪貼簿目前是否包含文字。</summary>
    /// <returns>若包含文字傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool ContainsText() const override;

    /// <summary>自 X11 剪貼簿取得 UTF-8 編碼之文字。</summary>
    /// <returns>UTF-8 字串。</returns>
    COMPAT_NODISCARD std::string GetText() const override;

    /// <summary>將純文字設定至 X11 剪貼簿。</summary>
    /// <param name="text">字串視圖。</param>
    void SetText(compat::string_view text) override;

    /// <summary>清空 X11 剪貼簿內容。</summary>
    void Clear() override;

private:
    void EnsureInitialized();
    void ProcessPendingEvents() const;

    mutable std::mutex m_mutex;
    Display*           m_display = nullptr;
    Window             m_window = 0;
    Atom               m_clipboardAtom = 0;
    Atom               m_utf8StringAtom = 0;
    Atom               m_targetsAtom = 0;
    Atom               m_propertyAtom = 0;
    mutable std::string m_localText;
    mutable bool       m_ownsClipboard = false;
    bool               m_isInitialized = false;
};

} // namespace x11
} // namespace pal
} // namespace cppui

#endif // CPPUI_HAS_X11
