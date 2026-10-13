/**
 * @file KeyEventArgs.hpp
 * @brief 鍵盤按鍵 (KeyEventArgs) 與文字輸入 (TextInputEventArgs) 路由事件參數定義
 */

#pragma once

#include <string>
#include "compat/compat.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"

namespace cppui {
namespace input {

/// <summary>
/// 鍵盤按鍵事件參數 (KeyEventArgs)
/// </summary>
class KeyEventArgs : public RoutedEventArgs {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    KeyEventArgs() = default;

    /// <summary>
    /// 建構完整的按鍵事件參數。
    /// </summary>
    /// <param name="routedEvent">路由事件定義描述項常數引用。</param>
    /// <param name="source">引發事件之來源 UI 元件指標。</param>
    /// <param name="virtualKey">虛擬鍵盤碼。</param>
    /// <param name="scanCode">硬體掃描碼，預設為 0。</param>
    /// <param name="isRepeat">是否為長按重複觸發，預設為 false。</param>
    /// <param name="modifiers">當前按下的鍵盤修飾鍵組合。</param>
    KeyEventArgs(
        const RoutedEvent& routedEvent,
        core::UIElement* source,
        uint32_t virtualKey,
        uint32_t scanCode = 0,
        bool isRepeat = false,
        pal::KeyModifiers modifiers = pal::KeyModifiers::None
    ) noexcept
        : RoutedEventArgs(routedEvent, source)
        , m_virtualKey(virtualKey)
        , m_scanCode(scanCode)
        , m_isRepeat(isRepeat)
        , m_modifiers(modifiers) {}

    /// <summary>取得虛擬鍵碼。</summary>
    /// <returns>虛擬鍵碼整數值。</returns>
    COMPAT_NODISCARD uint32_t GetVirtualKey() const noexcept { return m_virtualKey; }

    /// <summary>取得硬體掃描碼。</summary>
    /// <returns>掃描碼整數值。</returns>
    COMPAT_NODISCARD uint32_t GetScanCode() const noexcept { return m_scanCode; }

    /// <summary>檢查是否為長按重複觸發。</summary>
    /// <returns>若為重複觸發傳回 true。</returns>
    COMPAT_NODISCARD bool IsRepeat() const noexcept { return m_isRepeat; }

    /// <summary>取得鍵盤修飾鍵狀態。</summary>
    /// <returns>KeyModifiers 旗標組合。</returns>
    COMPAT_NODISCARD pal::KeyModifiers GetModifiers() const noexcept { return m_modifiers; }

    /// <summary>檢查是否按下 Control 鍵。</summary>
    /// <returns>若按下傳回 true。</returns>
    COMPAT_NODISCARD bool HasControl() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Control); }

    /// <summary>檢查是否按下 Shift 鍵。</summary>
    /// <returns>若按下傳回 true。</returns>
    COMPAT_NODISCARD bool HasShift() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Shift); }

    /// <summary>檢查是否按下 Alt 鍵。</summary>
    /// <returns>若按下傳回 true。</returns>
    COMPAT_NODISCARD bool HasAlt() const noexcept { return pal::HasModifier(m_modifiers, pal::KeyModifiers::Alt); }

private:
    uint32_t          m_virtualKey = 0;
    uint32_t          m_scanCode = 0;
    bool              m_isRepeat = false;
    pal::KeyModifiers m_modifiers = pal::KeyModifiers::None;
};

/// <summary>
/// 鍵盤文字輸入事件參數 (TextInputEventArgs)
/// </summary>
class TextInputEventArgs : public RoutedEventArgs {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    TextInputEventArgs() = default;

    /// <summary>
    /// 建構文字輸入事件參數。
    /// </summary>
    /// <param name="routedEvent">路由事件定義描述項常數引用。</param>
    /// <param name="source">引發事件之來源 UI 元件指標。</param>
    /// <param name="codepoint">UTF-32 Unicode 碼點。</param>
    /// <param name="text">UTF-8 文字字串視圖。</param>
    TextInputEventArgs(
        const RoutedEvent& routedEvent,
        core::UIElement* source,
        uint32_t codepoint,
        compat::string_view text
    )
        : RoutedEventArgs(routedEvent, source)
        , m_codepoint(codepoint)
        , m_text(text.data(), text.size()) {}

    /// <summary>取得 Unicode 碼點。</summary>
    /// <returns>Unicode 碼點數值。</returns>
    COMPAT_NODISCARD uint32_t GetCodepoint() const noexcept { return m_codepoint; }

    /// <summary>取得輸入文字字串視圖。</summary>
    /// <returns>文字字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetText() const noexcept { return compat::string_view(m_text.data(), m_text.size()); }

private:
    uint32_t    m_codepoint = 0;
    std::string m_text;
};

/// <summary>
/// IME 輸入法組字/定稿事件參數 (ImeCompositionEventArgs)
/// </summary>
class ImeCompositionEventArgs : public RoutedEventArgs {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    ImeCompositionEventArgs() = default;

    /// <summary>
    /// 建構 IME 組字事件參數。
    /// </summary>
    /// <param name="routedEvent">路由事件定義描述項常數引用。</param>
    /// <param name="source">引發事件之來源 UI 元件指標。</param>
    /// <param name="compositionText">組字或定稿之 UTF-8 文字視圖。</param>
    /// <param name="cursorPosition">組字串內之游標位置。</param>
    /// <param name="isCommitted">是否為已定稿確認文字 (true 代表定稿，false 代表組字中)。</param>
    ImeCompositionEventArgs(
        const RoutedEvent& routedEvent,
        core::UIElement* source,
        compat::string_view compositionText,
        int32_t cursorPosition,
        bool isCommitted
    )
        : RoutedEventArgs(routedEvent, source)
        , m_compositionText(compositionText.data(), compositionText.size())
        , m_cursorPosition(cursorPosition)
        , m_isCommitted(isCommitted) {}

    /// <summary>取得組字或定稿之文字視圖。</summary>
    /// <returns>文字字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetCompositionText() const noexcept {
        return compat::string_view(m_compositionText.data(), m_compositionText.size());
    }

    /// <summary>取得組字游標字元偏移位置。</summary>
    /// <returns>游標字元索引。</returns>
    COMPAT_NODISCARD int32_t GetCursorPosition() const noexcept {
        return m_cursorPosition;
    }

    /// <summary>檢查該組字文字是否已定稿確認。</summary>
    /// <returns>若已確認定稿傳回 true，否則為組字中傳回 false。</returns>
    COMPAT_NODISCARD bool IsCommitted() const noexcept {
        return m_isCommitted;
    }

private:
    std::string m_compositionText;
    int32_t     m_cursorPosition = 0;
    bool        m_isCommitted = false;
};

} // namespace input
} // namespace cppui

