/*
 * File: RawInputEvent.hpp
 * Purpose: Defines platform raw input event structures and factory helpers.
 */

#pragma once

#include <cstdint>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"

namespace cppui {
namespace pal {

/// <summary>
/// 原始事件類型列舉
/// </summary>
enum class RawEventType : uint16_t {
    None = 0,
    // 滑鼠事件
    MouseMove,
    MouseDown,
    MouseUp,
    MouseWheel,
    MouseEnter,
    MouseLeave,
    // 鍵盤事件
    KeyDown,
    KeyUp,
    TextInput,
    // 視窗生命週期事件
    WindowResize,
    WindowMove,
    WindowClose,
    WindowFocusGained,
    WindowFocusLost,
    DpiChanged
};

/// <summary>
/// 滑鼠按鍵列舉
/// </summary>
enum class RawMouseButton : uint8_t {
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 3,
    XButton1 = 4,
    XButton2 = 5
};

/// <summary>
/// 鍵盤修飾鍵旗標
/// </summary>
enum class KeyModifiers : uint8_t {
    None     = 0,
    Control  = 1 << 0,
    Shift    = 1 << 1,
    Alt      = 1 << 2,
    Meta     = 1 << 3, // Windows 鍵 / Command 鍵
    CapsLock = 1 << 4
};

/// <summary>
/// 按位或運算子：組合兩個鍵盤修飾鍵旗標。
/// </summary>
/// <param name="a">修飾鍵 A。</param>
/// <param name="b">修飾鍵 B。</param>
/// <returns>組合後之修飾鍵旗標。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 inline KeyModifiers operator|(KeyModifiers a, KeyModifiers b) noexcept {
    return static_cast<KeyModifiers>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

/// <summary>
/// 按位與運算子：檢驗鍵盤修飾鍵旗標交集。
/// </summary>
/// <param name="a">修飾鍵 A。</param>
/// <param name="b">修飾鍵 B。</param>
/// <returns>交集修飾鍵旗標。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 inline KeyModifiers operator&(KeyModifiers a, KeyModifiers b) noexcept {
    return static_cast<KeyModifiers>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

/// <summary>
/// 檢驗修飾鍵集合是否包含特定目標修飾鍵。
/// </summary>
/// <param name="mods">修飾鍵集合。</param>
/// <param name="target">目標待檢測修飾鍵。</param>
/// <returns>若包含傳回 true，否則傳回 false。</returns>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 inline bool HasModifier(KeyModifiers mods, KeyModifiers target) noexcept {
    return (mods & target) == target;
}

/// <summary>
/// 原始滑鼠事件資料
/// </summary>
struct RawMouseEventData {
    core::Point  position;         // 相對於視窗客戶區的排版座標 (客戶區像素空間，與 Visual 佈局對齊)
    core::Point  physicalPosition; // 相對於視窗客戶區的物理像素座標 (Physical Pixels)
    core::Point  logicalPosition;  // 相對於視窗客戶區的邏輯座標 (DIP: Device Independent Pixels)
    RawMouseButton button = RawMouseButton::None;
    int32_t      wheelDeltaX = 0; // 滾輪水平滾動量
    int32_t      wheelDeltaY = 0; // 滾輪垂直滾動量
    int32_t      clickCount = 1;  // 單擊 (1), 雙擊 (2)
};

/// <summary>
/// 原始鍵盤事件資料
/// </summary>
struct RawKeyEventData {
    uint32_t virtualKey = 0; // 虛擬鍵碼 (VK_*)
    uint32_t scanCode   = 0; // 平台掃描碼
    bool     isRepeat   = false;
};

/// <summary>
/// 原始文字輸入事件資料 (UTF-32 codepoint 或 UTF-8 character)
/// </summary>
struct RawTextInputData {
    uint32_t codepoint = 0;
};

/// <summary>
/// 原始視窗幾何與 DPI 變更事件資料
/// </summary>
struct RawWindowEventData {
    uint32_t width  = 0; // 客戶區物理像素寬度
    uint32_t height = 0; // 客戶區物理像素高度
    float    dpiScale = 1.0f; // DPI 縮放係數 (如 1.0, 1.25, 1.5, 2.0)
};

/// <summary>
/// 平台原生輸入事件統一結構
/// </summary>
struct RawInputEvent {
    RawEventType  type = RawEventType::None;
    uint64_t      timestampMs = 0; // 時間戳 (毫秒)
    KeyModifiers  modifiers = KeyModifiers::None;

    // 事件詳細內容
    union {
        RawMouseEventData  mouse;
        RawKeyEventData    key;
        RawTextInputData   text;
        RawWindowEventData window;
    };

    /// <summary>
    /// 預設建構原始輸入事件。
    /// </summary>
    RawInputEvent() noexcept : type(RawEventType::None), timestampMs(0), modifiers(KeyModifiers::None) {
        window = {};
    }

    /// <summary>
    /// 建立滑鼠移動事件 (包含客戶區排版、物理像素與邏輯像素座標)。
    /// </summary>
    /// <param name="clientPos">客戶區排版座標點。</param>
    /// <param name="physicalPos">物理像素座標點。</param>
    /// <param name="logicalPos">邏輯 DIP 座標點。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseMove(core::Point clientPos, core::Point physicalPos, core::Point logicalPos, KeyModifiers mods, uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::MouseMove;
        e.modifiers = mods;
        e.timestampMs = ts;
        e.mouse = { clientPos, physicalPos, logicalPos, RawMouseButton::None, 0, 0, 0 };
        return e;
    }

    /// <summary>
    /// 建立滑鼠移動事件 (包含客戶區排版與物理像素座標)。
    /// </summary>
    /// <param name="clientPos">客戶區排版座標點。</param>
    /// <param name="physicalPos">物理像素座標點。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseMove(core::Point clientPos, core::Point physicalPos, KeyModifiers mods, uint64_t ts = 0) noexcept {
        return MakeMouseMove(clientPos, physicalPos, clientPos, mods, ts);
    }

    /// <summary>
    /// 建立單一座標之滑鼠移動事件。
    /// </summary>
    /// <param name="pos">座標點。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseMove(core::Point pos, KeyModifiers mods, uint64_t ts = 0) noexcept {
        return MakeMouseMove(pos, pos, pos, mods, ts);
    }

    /// <summary>
    /// 建立滑鼠按下事件 (包含客戶區排版、物理像素與邏輯像素座標)。
    /// </summary>
    /// <param name="clientPos">客戶區排版座標點。</param>
    /// <param name="physicalPos">物理像素座標點。</param>
    /// <param name="logicalPos">邏輯 DIP 座標點。</param>
    /// <param name="btn">觸發之滑鼠按鍵。</param>
    /// <param name="clicks">點擊次數 (1 代表單擊，2 代表雙擊)。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseDown(core::Point clientPos, core::Point physicalPos, core::Point logicalPos, RawMouseButton btn, int32_t clicks, KeyModifiers mods, uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::MouseDown;
        e.modifiers = mods;
        e.timestampMs = ts;
        e.mouse = { clientPos, physicalPos, logicalPos, btn, 0, 0, clicks };
        return e;
    }

    /// <summary>
    /// 建立滑鼠按下事件 (包含客戶區排版與物理像素座標)。
    /// </summary>
    /// <param name="clientPos">客戶區排版座標點。</param>
    /// <param name="physicalPos">物理像素座標點。</param>
    /// <param name="btn">觸發之滑鼠按鍵。</param>
    /// <param name="clicks">點擊次數 (1 代表單擊，2 代表雙擊)。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseDown(core::Point clientPos, core::Point physicalPos, RawMouseButton btn, int32_t clicks, KeyModifiers mods, uint64_t ts = 0) noexcept {
        return MakeMouseDown(clientPos, physicalPos, clientPos, btn, clicks, mods, ts);
    }

    /// <summary>
    /// 建立單一座標之滑鼠按下事件。
    /// </summary>
    /// <param name="pos">座標點。</param>
    /// <param name="btn">觸發之滑鼠按鍵。</param>
    /// <param name="clicks">點擊次數 (1 代表單擊，2 代表雙擊)。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseDown(core::Point pos, RawMouseButton btn, int32_t clicks, KeyModifiers mods, uint64_t ts = 0) noexcept {
        return MakeMouseDown(pos, pos, pos, btn, clicks, mods, ts);
    }

    /// <summary>
    /// 建立滑鼠釋放事件 (包含客戶區排版、物理像素與邏輯像素座標)。
    /// </summary>
    /// <param name="clientPos">客戶區排版座標點。</param>
    /// <param name="physicalPos">物理像素座標點。</param>
    /// <param name="logicalPos">邏輯 DIP 座標點。</param>
    /// <param name="btn">觸發之滑鼠按鍵。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseUp(core::Point clientPos, core::Point physicalPos, core::Point logicalPos, RawMouseButton btn, KeyModifiers mods, uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::MouseUp;
        e.modifiers = mods;
        e.timestampMs = ts;
        e.mouse = { clientPos, physicalPos, logicalPos, btn, 0, 0, 0 };
        return e;
    }

    /// <summary>
    /// 建立滑鼠釋放事件 (包含客戶區排版與物理像素座標)。
    /// </summary>
    /// <param name="clientPos">客戶區排版座標點。</param>
    /// <param name="physicalPos">物理像素座標點。</param>
    /// <param name="btn">觸發之滑鼠按鍵。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseUp(core::Point clientPos, core::Point physicalPos, RawMouseButton btn, KeyModifiers mods, uint64_t ts = 0) noexcept {
        return MakeMouseUp(clientPos, physicalPos, clientPos, btn, mods, ts);
    }

    /// <summary>
    /// 建立單一座標之滑鼠釋放事件。
    /// </summary>
    /// <param name="pos">座標點。</param>
    /// <param name="btn">觸發之滑鼠按鍵。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseUp(core::Point pos, RawMouseButton btn, KeyModifiers mods, uint64_t ts = 0) noexcept {
        return MakeMouseUp(pos, pos, pos, btn, mods, ts);
    }

    /// <summary>
    /// 建立滑鼠滾輪事件 (包含客戶區排版、物理像素與邏輯像素座標)。
    /// </summary>
    /// <param name="clientPos">客戶區排版座標點。</param>
    /// <param name="physicalPos">物理像素座標點。</param>
    /// <param name="logicalPos">邏輯 DIP 座標點。</param>
    /// <param name="deltaY">縱向滾動量。</param>
    /// <param name="deltaX">橫向滾動量。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseWheel(core::Point clientPos, core::Point physicalPos, core::Point logicalPos, int32_t deltaY, int32_t deltaX, KeyModifiers mods, uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::MouseWheel;
        e.modifiers = mods;
        e.timestampMs = ts;
        e.mouse = { clientPos, physicalPos, logicalPos, RawMouseButton::None, deltaX, deltaY, 0 };
        return e;
    }

    /// <summary>
    /// 建立滑鼠滾輪事件 (包含客戶區排版與物理像素座標)。
    /// </summary>
    /// <param name="clientPos">客戶區排版座標點。</param>
    /// <param name="physicalPos">物理像素座標點。</param>
    /// <param name="deltaY">縱向滾動量。</param>
    /// <param name="deltaX">橫向滾動量。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseWheel(core::Point clientPos, core::Point physicalPos, int32_t deltaY, int32_t deltaX, KeyModifiers mods, uint64_t ts = 0) noexcept {
        return MakeMouseWheel(clientPos, physicalPos, clientPos, deltaY, deltaX, mods, ts);
    }

    /// <summary>
    /// 建立單一座標之滑鼠滾輪事件。
    /// </summary>
    /// <param name="pos">座標點。</param>
    /// <param name="deltaY">縱向滾動量。</param>
    /// <param name="deltaX">橫向滾動量。</param>
    /// <param name="mods">鍵盤修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeMouseWheel(core::Point pos, int32_t deltaY, int32_t deltaX, KeyModifiers mods, uint64_t ts = 0) noexcept {
        return MakeMouseWheel(pos, pos, pos, deltaY, deltaX, mods, ts);
    }

    /// <summary>
    /// 建立視窗大小調整事件。
    /// </summary>
    /// <param name="w">視窗寬度。</param>
    /// <param name="h">視窗高度。</param>
    /// <param name="dpi">DPI 縮放比率。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeResize(uint32_t w, uint32_t h, float dpi, uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::WindowResize;
        e.timestampMs = ts;
        e.window = { w, h, dpi };
        return e;
    }

    /// <summary>
    /// 建立視窗 DPI 變更事件。
    /// </summary>
    /// <param name="dpi">新的 DPI 縮放比率。</param>
    /// <param name="w">新寬度。</param>
    /// <param name="h">新高度。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeDpiChange(float dpi, uint32_t w, uint32_t h, uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::DpiChanged;
        e.timestampMs = ts;
        e.window = { w, h, dpi };
        return e;
    }

    /// <summary>
    /// 建立鍵盤按鍵按下事件。
    /// </summary>
    /// <param name="virtualKey">虛擬鍵碼 (VK_*)。</param>
    /// <param name="scanCode">平台掃描碼。</param>
    /// <param name="mods">修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeKeyDown(uint32_t virtualKey, uint32_t scanCode, KeyModifiers mods, uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::KeyDown;
        e.modifiers = mods;
        e.timestampMs = ts;
        e.key = { virtualKey, scanCode, false };
        return e;
    }

    /// <summary>
    /// 建立鍵盤按鍵放開事件。
    /// </summary>
    /// <param name="virtualKey">虛擬鍵碼 (VK_*)。</param>
    /// <param name="scanCode">平台掃描碼。</param>
    /// <param name="mods">修飾鍵狀態。</param>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeKeyUp(uint32_t virtualKey, uint32_t scanCode, KeyModifiers mods, uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::KeyUp;
        e.modifiers = mods;
        e.timestampMs = ts;
        e.key = { virtualKey, scanCode, false };
        return e;
    }

    /// <summary>
    /// 建立視窗關閉請求事件。
    /// </summary>
    /// <param name="ts">事件時間戳 (毫秒)。</param>
    /// <returns>RawInputEvent 實例。</returns>
    COMPAT_NODISCARD static RawInputEvent MakeClose(uint64_t ts = 0) noexcept {
        RawInputEvent e;
        e.type = RawEventType::WindowClose;
        e.timestampMs = ts;
        return e;
    }
};

} // namespace pal
} // namespace cppui
