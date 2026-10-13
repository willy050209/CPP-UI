/*
 * File: PlatformWindowWayland.hpp
 * Purpose: Declares the Linux native Wayland platform window implementation.
 */

#pragma once

#if defined(CPPUI_HAS_WAYLAND) && CPPUI_HAS_WAYLAND

#include <cstdint>
#include <string>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include "protocol/xdg-shell-client-protocol.h"

#include "compat/compat.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/core/CoordinateTransform.hpp"

namespace cppui {
namespace pal {
namespace wayland {

/// <summary>
/// Linux Wayland 原生實體視窗實作類別 (繼承 IPlatformWindow)。
/// 支援 xdg_wm_base Toplevel、wl_shm 雙緩衝軟體渲染、wl_pointer / wl_keyboard (xkbcommon) 與非同步 socket 輪詢。
/// </summary>
class PlatformWindowWayland : public IPlatformWindow {
public:
    /// <summary>
    /// 建構子：依據描述符建立並初始化 Wayland 原生視窗。
    /// </summary>
    /// <param name="desc">視窗建構描述符。</param>
    /// <exception cref="std::invalid_argument">當寬高為 0 時擲出。</exception>
    /// <exception cref="std::runtime_error">當無法連接 Wayland Display 時擲出。</exception>
    explicit PlatformWindowWayland(const WindowDesc& desc);

    /// <summary>
    /// 解構子：釋放 Wayland Surface, Shell, SHM 及 Display 連線資源。
    /// </summary>
    ~PlatformWindowWayland() override;

    // 禁止拷貝
    PlatformWindowWayland(const PlatformWindowWayland&) = delete;
    PlatformWindowWayland& operator=(const PlatformWindowWayland&) = delete;

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

    // --- 原生代碼 ---

    /// <summary>取得原生平台視窗控制代碼 (wl_surface)。</summary>
    /// <returns>wl_surface 指標。</returns>
    COMPAT_NODISCARD void* GetNativeHandle() const noexcept override { return static_cast<void*>(m_surface); }

    /// <summary>取得原生平台顯示伺服器連線 (wl_display)。</summary>
    /// <returns>wl_display 指標。</returns>
    COMPAT_NODISCARD void* GetNativeDisplay() const noexcept { return static_cast<void*>(m_display); }

    // --- 事件輪詢與回呼 ---

    /// <summary>設定原始輸入事件回呼函式。</summary>
    /// <param name="callback">事件回呼委派。</param>
    void SetEventCallback(EventCallback callback) override { m_eventCallback = std::move(callback); }

    /// <summary>非阻塞輪詢原生平台事件佇列。</summary>
    void PollEvents() override;

    /// <summary>阻塞等待原生平台事件佇列。</summary>
    /// <param name="timeoutMs">逾時毫秒數，-1 表示無限等待。</param>
    /// <returns>若有事件處理傳回 true，若逾時傳回 false。</returns>
    bool WaitEvents(int32_t timeoutMs = -1) override;

    /// <summary>喚醒處於 WaitEvents 等待狀態的原生訊息迴圈。</summary>
    void WakeUp() override;

    /// <summary>向原生平台請求觸發視窗重繪。</summary>
    void RequestRedraw() override;

    /// <summary>設定重繪渲染回呼函式。</summary>
    /// <param name="callback">重繪回呼委派。</param>
    void SetPaintCallback(PaintCallback callback) override { m_paintCallback = std::move(callback); }

    /// <summary>將 32-bit BGRA/RGBA 像素緩衝區呈現在視窗表面。</summary>
    /// <param name="pixelBuffer">像素緩衝區 span。</param>
    /// <param name="width">緩衝區寬度。</param>
    /// <param name="height">緩衝區高度。</param>
    void PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) override;

    /// <summary>設定 IME 候選字窗錨定矩形。</summary>
    /// <param name="caretRect">游標所在邊界矩形。</param>
    void SetImeCompositionWindow(const core::Rect& caretRect) override { m_caretRect = caretRect; }

private:
    struct ShmBuffer {
        struct wl_buffer* buffer = nullptr;
        void* data = nullptr;
        size_t size = 0;
        bool isBusy = false;
        int fd = -1;
    };

    void InitWayland(const WindowDesc& desc);
    void CleanupWayland();
    ShmBuffer* GetAvailableBuffer(uint32_t width, uint32_t height);
    void CreateShmBuffer(ShmBuffer& shmBuf, uint32_t width, uint32_t height);
    void DestroyShmBuffer(ShmBuffer& shmBuf);
    void DispatchEvent(const RawInputEvent& event);
    uint64_t GetCurrentTimestampMs() const noexcept;

    // Wayland 全域物件
    struct wl_display* m_display = nullptr;
    struct wl_registry* m_registry = nullptr;
    struct wl_compositor* m_compositor = nullptr;
    struct wl_shm* m_shm = nullptr;
    struct xdg_wm_base* m_wmBase = nullptr;
    struct wl_seat* m_seat = nullptr;

    // 視窗專屬物件
    struct wl_surface* m_surface = nullptr;
    struct xdg_surface* m_xdgSurface = nullptr;
    struct xdg_toplevel* m_xdgToplevel = nullptr;

    // 輸入設備
    struct wl_pointer* m_pointer = nullptr;
    struct wl_keyboard* m_keyboard = nullptr;
    struct xkb_context* m_xkbContext = nullptr;
    struct xkb_keymap* m_xkbKeymap = nullptr;
    struct xkb_state* m_xkbState = nullptr;

    // 雙緩衝
    ShmBuffer m_buffers[2];
    size_t m_currentBufferIndex = 0;

    // 狀態與設定
    uint32_t m_width = 800;
    uint32_t m_height = 600;
    float m_dpiScale = 1.0f;
    std::string m_title;
    bool m_isClosed = false;
    bool m_isVisible = false;
    bool m_isConfigured = false;
    core::Rect m_caretRect{ 0.0f, 0.0f, 0.0f, 0.0f };

    EventCallback m_eventCallback;
    PaintCallback m_paintCallback;
    int m_wakePipe[2] = { -1, -1 };
};

} // namespace wayland
} // namespace pal
} // namespace cppui

#endif // CPPUI_HAS_WAYLAND
