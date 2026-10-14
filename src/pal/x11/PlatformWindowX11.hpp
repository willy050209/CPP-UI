/*
 * File: PlatformWindowX11.hpp
 * Purpose: Declares the Linux X11 platform window implementation for native X11/WSLg execution.
 */

#pragma once

#if defined(CPPUI_HAS_X11) && CPPUI_HAS_X11

#include <cstdint>
#include <string>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>

#include "compat/compat.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/core/CoordinateTransform.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

// 清除 X11 巨集污染，避免影響 C++ 型別、列舉與輔助函式 (如 Key::None, IsModifierKey, IsFunctionKey 等)
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
#ifdef Font
#undef Font
#endif
#ifdef Cursor
#undef Cursor
#endif

namespace cppui {
namespace pal {
namespace x11 {

/// <summary>
/// Linux X11 原生實體視窗實作類別 (繼承 IPlatformWindow)。
/// 支援原生 X11 事件輪詢、WSLg 實體彈窗、雙緩衝 XImage/XPutImage 繪製及 Self-pipe 跨執行緒喚醒。
/// </summary>
class PlatformWindowX11 : public IPlatformWindow {
public:
    /// <summary>
    /// 建構子：依據描述符建立並初始化 X11 原生視窗。
    /// </summary>
    /// <param name="desc">視窗建構描述符。</param>
    /// <exception cref="std::invalid_argument">當寬高為 0 時擲出。</exception>
    /// <exception cref="std::runtime_error">當無法連接 X11 Display 時擲出。</exception>
    explicit PlatformWindowX11(const WindowDesc& desc);

    /// <summary>
    /// 解構子：安全釋放 X11 GC、Window、Display 與 Self-pipe 資源。
    /// </summary>
    ~PlatformWindowX11() override;

    // 禁止拷貝
    PlatformWindowX11(const PlatformWindowX11&) = delete;
    PlatformWindowX11& operator=(const PlatformWindowX11&) = delete;

    // --- IPlatformWindow 生命週期控制 ---

    /// <summary>顯示視窗。</summary>
    void Show() override;

    /// <summary>隱藏視窗。</summary>
    void Hide() override;

    /// <summary>關閉視窗。</summary>
    void Close() override;

    /// <summary>檢查視窗是否已關閉。</summary>
    /// <returns>若已關閉傳回 true。</returns>
    COMPAT_NODISCARD bool IsClosed() const noexcept override { return m_isClosed; }

    // --- 幾何與標題 ---

    /// <summary>取得視窗邏輯尺寸 (點)。</summary>
    /// <returns>邏輯尺寸。</returns>
    COMPAT_NODISCARD core::Size GetSize() const noexcept override;

    /// <summary>設定視窗邏輯尺寸。</summary>
    /// <param name="width">邏輯寬度。</param>
    /// <param name="height">邏輯高度。</param>
    void SetSize(uint32_t width, uint32_t height) override;

    /// <summary>取得像素幀緩衝區實體尺寸 (實體像素)。</summary>
    /// <returns>實體尺寸。</returns>
    COMPAT_NODISCARD core::Size GetFramebufferSize() const noexcept override;

    /// <summary>取得當前螢幕 High-DPI 縮放比例。</summary>
    /// <returns>DPI 縮放因子。</returns>
    COMPAT_NODISCARD float GetDpiScale() const noexcept override { return m_dpiScale; }

    /// <summary>設定視窗標題。</summary>
    /// <param name="title">標題文字視圖。</param>
    void SetTitle(compat::string_view title) override;

    /// <summary>取得原生平台視窗控制代碼 (X11 Window ID)。</summary>
    /// <returns>原生控制代碼指標。</returns>
    COMPAT_NODISCARD void* GetNativeHandle() const noexcept override {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(m_window));
    }

    // --- 事件與回呼委派 ---

    /// <summary>設定原始輸入事件回呼函式。</summary>
    /// <param name="callback">事件回呼委派。</param>
    void SetEventCallback(EventCallback callback) override {
        m_eventCallback = std::move(callback);
    }

    /// <summary>非阻塞輪詢原生 X11 事件佇列。</summary>
    void PollEvents() override;

    /// <summary>阻塞等待原生 X11 事件佇列 (低 CPU 消耗休眠)。</summary>
    /// <param name="timeoutMs">逾時毫秒數，-1 表示無限等待。</param>
    /// <returns>若有事件處理傳回 true，逾時傳回 false。</returns>
    bool WaitEvents(int32_t timeoutMs = -1) override;

    /// <summary>喚醒處於 WaitEvents 等待狀態的事件迴圈 (支援跨執行緒調用)。</summary>
    void WakeUp() override;

    /// <summary>向 X11 請求觸發視窗重繪。</summary>
    void RequestRedraw() override;

    /// <summary>設定重繪渲染回呼函式。</summary>
    /// <param name="callback">重繪回呼委派。</param>
    void SetPaintCallback(PaintCallback callback) override {
        m_paintCallback = std::move(callback);
    }

    /// <summary>將 32-bit BGRA/RGBA 像素緩衝區呈現在 X11 視窗表面。</summary>
    /// <param name="pixelBuffer">像素緩衝區 span。</param>
    /// <param name="width">緩衝區寬度。</param>
    /// <param name="height">緩衝區高度。</param>
    void PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) override;

    // --- X11 特有原生存取器 ---

    /// <summary>取得底層 X11 Display 指標。</summary>
    /// <returns>Display 指標。</returns>
    COMPAT_NODISCARD Display* GetDisplay() const noexcept { return m_display; }

    /// <summary>取得底層 X11 Window ID。</summary>
    /// <returns>Window ID。</returns>
    COMPAT_NODISCARD Window GetX11Window() const noexcept { return m_window; }

private:
    void HandleXEvent(const XEvent& event);
    void DispatchEvent(const RawInputEvent& event);
    void QueryDpiScale();
    KeyModifiers ConvertModifiers(uint32_t state) const noexcept;
    uint64_t GetCurrentTimestampMs() const noexcept;

    Display*                  m_display = nullptr;
    Window                    m_window = 0;
    GC                        m_gc = nullptr;
    Visual*                   m_visual = nullptr;
    int32_t                   m_depth = 24;
    int32_t                   m_screen = 0;
    Atom                      m_wmDeleteWindow = 0;
    Atom                      m_netWmName = 0;
    Atom                      m_utf8String = 0;

    int32_t                   m_wakePipe[2] = { -1, -1 };
    bool                      m_isClosed = false;
    bool                      m_isVisible = false;
    bool                      m_pendingRedraw = false;
    float                     m_dpiScale = 1.0f;
    core::CoordinateTransform m_coordTransform{ 1.0f };
    uint32_t                  m_width = 800;
    uint32_t                  m_height = 600;
    std::string               m_title;

    EventCallback             m_eventCallback;
    PaintCallback             m_paintCallback;
};

} // namespace x11
} // namespace pal
} // namespace cppui

#endif // CPPUI_HAS_X11
