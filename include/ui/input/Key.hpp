/**
 * @file Key.hpp
 * @brief 鍵盤按鍵碼強型別列舉 (Key) 與輔助分類工具函式
 */

#pragma once

#include <cstdint>
#include "compat/Compat.hpp"

namespace cppui {
namespace input {

// 防禦性清除 X11 等第三方 C API 標頭可能帶來的巨集污染
#ifdef None
#undef None
#endif

/// <summary>
/// 鍵盤按鍵碼強型別列舉 (Key)
/// 底層型別為 uint32_t，數值對齊工業標準虛擬鍵碼 (Virtual-Key Codes)。
/// </summary>
enum class Key : uint32_t {
    None = 0,

    // 控制與導航
    Cancel      = 0x03,
    Backspace   = 0x08,
    Back        = Backspace,
    Tab         = 0x09,
    Clear       = 0x0C,
    Return      = 0x0D,
    Enter       = Return,
    Shift       = 0x10,
    Control     = 0x11,
    Ctrl        = Control,
    Menu        = 0x12,
    Alt         = Menu,
    Pause       = 0x13,
    CapsLock    = 0x14,
    Capital     = CapsLock,
    Escape      = 0x1B,
    Esc         = Escape,
    Space       = 0x20,
    PageUp      = 0x21,
    Prior       = PageUp,
    PageDown    = 0x22,
    Next        = PageDown,
    End         = 0x23,
    Home        = 0x24,
    Left        = 0x25,
    Up          = 0x26,
    Right       = 0x27,
    Down        = 0x28,
    Select      = 0x29,
    Print       = 0x2A,
    Execute     = 0x2B,
    PrintScreen = 0x2C,
    Snapshot    = PrintScreen,
    Insert      = 0x2D,
    Delete      = 0x2E,
    Del         = Delete,
    Help        = 0x2F,

    // 數字鍵 (主鍵盤 0 - 9)
    D0 = 0x30, Num0 = D0,
    D1 = 0x31, Num1 = D1,
    D2 = 0x32, Num2 = D2,
    D3 = 0x33, Num3 = D3,
    D4 = 0x34, Num4 = D4,
    D5 = 0x35, Num5 = D5,
    D6 = 0x36, Num6 = D6,
    D7 = 0x37, Num7 = D7,
    D8 = 0x38, Num8 = D8,
    D9 = 0x39, Num9 = D9,

    // 字母鍵 (A - Z)
    A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46,
    G = 0x47, H = 0x48, I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C,
    M = 0x4D, N = 0x4E, O = 0x4F, P = 0x50, Q = 0x51, R = 0x52,
    S = 0x53, T = 0x54, U = 0x55, V = 0x56, W = 0x57, X = 0x58,
    Y = 0x59, Z = 0x5A,

    // 系統鍵 (Windows / Meta / Apps)
    LWin = 0x5B, LMeta = LWin,
    RWin = 0x5C, RMeta = RWin,
    Apps = 0x5D, ContextMenu = Apps,
    Sleep = 0x5F,

    // 數字鍵盤 (NumPad 0 - 9 與運算符)
    NumPad0 = 0x60, NumPad1 = 0x61, NumPad2 = 0x62, NumPad3 = 0x63,
    NumPad4 = 0x64, NumPad5 = 0x65, NumPad6 = 0x66, NumPad7 = 0x67,
    NumPad8 = 0x68, NumPad9 = 0x69,
    Multiply = 0x6A,
    Add = 0x6B,
    Separator = 0x6C,
    Subtract = 0x6D,
    Decimal = 0x6E,
    Divide = 0x6F,

    // 功能鍵 (F1 - F24)
    F1 = 0x70, F2 = 0x71, F3 = 0x72, F4 = 0x73,
    F5 = 0x74, F6 = 0x75, F7 = 0x76, F8 = 0x77,
    F9 = 0x78, F10 = 0x79, F11 = 0x7A, F12 = 0x7B,
    F13 = 0x7C, F14 = 0x7D, F15 = 0x7E, F16 = 0x7F,
    F17 = 0x80, F18 = 0x81, F19 = 0x82, F20 = 0x83,
    F21 = 0x84, F22 = 0x85, F23 = 0x86, F24 = 0x87,

    // 鎖定鍵
    NumLock = 0x90,
    Scroll = 0x91, ScrollLock = Scroll,

    // 獨立左右修飾鍵
    LShift = 0xA0, RShift = 0xA1,
    LControl = 0xA2, LCtrl = LControl,
    RControl = 0xA3, RCtrl = RControl,
    LMenu = 0xA4, LAlt = LMenu,
    RMenu = 0xA5, RAlt = RMenu,

    // 瀏覽器與多媒體鍵
    BrowserBack = 0xA6,
    BrowserForward = 0xA7,
    BrowserRefresh = 0xA8,
    BrowserStop = 0xA9,
    BrowserSearch = 0xAA,
    BrowserFavorites = 0xAB,
    BrowserHome = 0xAC,
    VolumeMute = 0xAD,
    VolumeDown = 0xAE,
    VolumeUp = 0xAF,
    MediaNextTrack = 0xB0,
    MediaPreviousTrack = 0xB1,
    MediaStop = 0xB2,
    MediaPlayPause = 0xB3,

    // 標點符號與 OEM 鍵
    OemSemicolon    = 0xBA,
    OemPlus         = 0xBB,
    OemComma        = 0xBC,
    OemMinus        = 0xBD,
    OemPeriod       = 0xBE,
    OemQuestion     = 0xBF,
    OemTilde        = 0xC0,
    OemOpenBrackets = 0xDB,
    OemPipe         = 0xDC,
    OemCloseBrackets= 0xDD,
    OemQuotes       = 0xDE
};

/// <summary>
/// 將原始整數虛擬鍵碼轉換為強型別 Key 列舉。
/// </summary>
/// <param name="vk">整數虛擬鍵碼。</param>
/// <returns>對應之 Key 列舉值。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 Key FromVirtualKey(uint32_t vk) noexcept {
    return static_cast<Key>(vk);
}

/// <summary>
/// 將強型別 Key 列舉轉換為原始整數虛擬鍵碼。
/// </summary>
/// <param name="key">Key 列舉值。</param>
/// <returns>對應之整數虛擬鍵碼。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 uint32_t ToVirtualKey(Key key) noexcept {
    return static_cast<uint32_t>(key);
}

// 防禦性清除 X11 Xutil.h 鍵盤判定巨集污染
#ifdef IsModifierKey
#undef IsModifierKey
#endif
#ifdef IsFunctionKey
#undef IsFunctionKey
#endif
#ifdef IsKeypadKey
#undef IsKeypadKey
#endif
#ifdef IsPrivateKeypadKey
#undef IsPrivateKeypadKey
#endif
#ifdef IsCursorKey
#undef IsCursorKey
#endif
#ifdef IsPFKey
#undef IsPFKey
#endif
#ifdef IsMiscFunctionKey
#undef IsMiscFunctionKey
#endif

/// <summary>
/// 判斷指定之按鍵是否為修飾鍵 (Shift, Control, Alt, Windows)。
/// </summary>
/// <param name="key">待檢驗按鍵。</param>
/// <returns>若為修飾鍵傳回 true，否則傳回 false。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsModifierKey(Key key) noexcept {
    switch (key) {
        case Key::Shift:
        case Key::Control:
        case Key::Menu:
        case Key::LShift:
        case Key::RShift:
        case Key::LControl:
        case Key::RControl:
        case Key::LMenu:
        case Key::RMenu:
        case Key::LWin:
        case Key::RWin:
            return true;
        default:
            return false;
    }
}

/// <summary>
/// 判斷指定之按鍵是否為方向箭頭鍵 (Up, Down, Left, Right)。
/// </summary>
/// <param name="key">待檢驗按鍵。</param>
/// <returns>若為箭頭鍵傳回 true，否則傳回 false。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsArrowKey(Key key) noexcept {
    switch (key) {
        case Key::Left:
        case Key::Up:
        case Key::Right:
        case Key::Down:
            return true;
        default:
            return false;
    }
}

/// <summary>
/// 判斷指定之按鍵是否為功能鍵 (F1 - F24)。
/// </summary>
/// <param name="key">待檢驗按鍵。</param>
/// <returns>若為功能鍵傳回 true，否則傳回 false。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsFunctionKey(Key key) noexcept {
    uint32_t vk = static_cast<uint32_t>(key);
    return (vk >= static_cast<uint32_t>(Key::F1) && vk <= static_cast<uint32_t>(Key::F24));
}

/// <summary>
/// 判斷指定之按鍵是否為主鍵盤英數字鍵 (A-Z, 0-9)。
/// </summary>
/// <param name="key">待檢驗按鍵。</param>
/// <returns>若為字母或數字鍵傳回 true，否則傳回 false。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool IsAlphaNumeric(Key key) noexcept {
    uint32_t vk = static_cast<uint32_t>(key);
    return (vk >= static_cast<uint32_t>(Key::D0) && vk <= static_cast<uint32_t>(Key::D9)) ||
           (vk >= static_cast<uint32_t>(Key::A) && vk <= static_cast<uint32_t>(Key::Z));
}

/// <summary>
/// 取得按鍵碼之文字名稱視圖。
/// </summary>
/// <param name="key">Key 列舉值。</param>
/// <returns>字串視圖。</returns>
COMPAT_NODISCARD inline compat::string_view KeyToString(Key key) noexcept {
    switch (key) {
        case Key::None:        return "None";
        case Key::Backspace:   return "Backspace";
        case Key::Tab:         return "Tab";
        case Key::Clear:       return "Clear";
        case Key::Return:      return "Return";
        case Key::Shift:       return "Shift";
        case Key::Control:     return "Control";
        case Key::Menu:        return "Alt";
        case Key::Pause:       return "Pause";
        case Key::CapsLock:    return "CapsLock";
        case Key::Escape:      return "Escape";
        case Key::Space:       return "Space";
        case Key::PageUp:      return "PageUp";
        case Key::PageDown:    return "PageDown";
        case Key::End:         return "End";
        case Key::Home:        return "Home";
        case Key::Left:        return "Left";
        case Key::Up:          return "Up";
        case Key::Right:       return "Right";
        case Key::Down:        return "Down";
        case Key::Select:      return "Select";
        case Key::Print:       return "Print";
        case Key::Execute:     return "Execute";
        case Key::PrintScreen: return "PrintScreen";
        case Key::Insert:      return "Insert";
        case Key::Delete:      return "Delete";
        case Key::Help:        return "Help";

        case Key::D0: return "0"; case Key::D1: return "1"; case Key::D2: return "2";
        case Key::D3: return "3"; case Key::D4: return "4"; case Key::D5: return "5";
        case Key::D6: return "6"; case Key::D7: return "7"; case Key::D8: return "8";
        case Key::D9: return "9";

        case Key::A: return "A"; case Key::B: return "B"; case Key::C: return "C";
        case Key::D: return "D"; case Key::E: return "E"; case Key::F: return "F";
        case Key::G: return "G"; case Key::H: return "H"; case Key::I: return "I";
        case Key::J: return "J"; case Key::K: return "K"; case Key::L: return "L";
        case Key::M: return "M"; case Key::N: return "N"; case Key::O: return "O";
        case Key::P: return "P"; case Key::Q: return "Q"; case Key::R: return "R";
        case Key::S: return "S"; case Key::T: return "T"; case Key::U: return "U";
        case Key::V: return "V"; case Key::W: return "W"; case Key::X: return "X";
        case Key::Y: return "Y"; case Key::Z: return "Z";

        case Key::LWin: return "LWin"; case Key::RWin: return "RWin";
        case Key::Apps: return "Apps"; case Key::Sleep: return "Sleep";

        case Key::NumPad0: return "NumPad0"; case Key::NumPad1: return "NumPad1";
        case Key::NumPad2: return "NumPad2"; case Key::NumPad3: return "NumPad3";
        case Key::NumPad4: return "NumPad4"; case Key::NumPad5: return "NumPad5";
        case Key::NumPad6: return "NumPad6"; case Key::NumPad7: return "NumPad7";
        case Key::NumPad8: return "NumPad8"; case Key::NumPad9: return "NumPad9";
        case Key::Multiply: return "Multiply"; case Key::Add: return "Add";
        case Key::Separator: return "Separator"; case Key::Subtract: return "Subtract";
        case Key::Decimal: return "Decimal"; case Key::Divide: return "Divide";

        case Key::F1: return "F1";   case Key::F2: return "F2";   case Key::F3: return "F3";
        case Key::F4: return "F4";   case Key::F5: return "F5";   case Key::F6: return "F6";
        case Key::F7: return "F7";   case Key::F8: return "F8";   case Key::F9: return "F9";
        case Key::F10: return "F10"; case Key::F11: return "F11"; case Key::F12: return "F12";
        case Key::F13: return "F13"; case Key::F14: return "F14"; case Key::F15: return "F15";
        case Key::F16: return "F16"; case Key::F17: return "F17"; case Key::F18: return "F18";
        case Key::F19: return "F19"; case Key::F20: return "F20"; case Key::F21: return "F21";
        case Key::F22: return "F22"; case Key::F23: return "F23"; case Key::F24: return "F24";

        case Key::NumLock:    return "NumLock";
        case Key::Scroll:     return "ScrollLock";
        case Key::LShift:     return "LShift";
        case Key::RShift:     return "RShift";
        case Key::LControl:   return "LControl";
        case Key::RControl:   return "RControl";
        case Key::LMenu:      return "LAlt";
        case Key::RMenu:      return "RAlt";

        case Key::OemSemicolon:     return ";";
        case Key::OemPlus:          return "+";
        case Key::OemComma:         return ",";
        case Key::OemMinus:         return "-";
        case Key::OemPeriod:        return ".";
        case Key::OemQuestion:      return "/";
        case Key::OemTilde:         return "`";
        case Key::OemOpenBrackets:  return "[";
        case Key::OemPipe:          return "\\";
        case Key::OemCloseBrackets: return "]";
        case Key::OemQuotes:        return "'";

        default:
            return "Unknown";
    }
}

} // namespace input
} // namespace cppui
