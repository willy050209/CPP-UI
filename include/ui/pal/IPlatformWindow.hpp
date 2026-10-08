/*
 * File: IPlatformWindow.hpp
 * Purpose: Declares the IPlatformWindow platform abstraction layer interface.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/pal/RawInputEvent.hpp"

namespace cppui {
namespace pal {

/// <summary>
/// 視窗建構描述符
/// </summary>
struct WindowDesc {
    compat::string_view title = "CPPUI Window";
    uint32_t width = 800;
    uint32_t height = 600;
    bool resizable = true;
    bool visible = true;
    bool decorated = true;
};

/// <summary>
/// 跨平台抽象視窗介面 (PAL)
/// </summary>
class IPlatformWindow {
public:
    virtual ~IPlatformWindow() = default;

    // 視窗生命週期控制

    /// <summary>顯示視窗。</summary>
    virtual void Show() = 0;

    /// <summary>隱藏視窗。</summary>
    virtual void Hide() = 0;

    /// <summary>關閉視窗。</summary>
    virtual void Close() = 0;

    /// <summary>檢查視窗是否已關閉。</summary>
    /// <returns>若已關閉傳回 true。</returns>
    COMPAT_NODISCARD virtual bool IsClosed() const noexcept = 0;

    // 幾何與標題

    /// <summary>取得視窗邏輯尺寸 (點)。</summary>
    /// <returns>Size 邏輯尺寸。</returns>
    COMPAT_NODISCARD virtual core::Size GetSize() const noexcept = 0;

    /// <summary>設定視窗邏輯尺寸。</summary>
    /// <param name="width">邏輯寬度。</param>
    /// <param name="height">邏輯高度。</param>
    virtual void SetSize(uint32_t width, uint32_t height) = 0;

    /// <summary>取得像素幀緩衝區實體尺寸 (實體像素)。</summary>
    /// <returns>Size 實體尺寸。</returns>
    COMPAT_NODISCARD virtual core::Size GetFramebufferSize() const noexcept = 0;

    /// <summary>取得當前螢幕 High-DPI 縮放比例。</summary>
    /// <returns>DPI 縮放因子 (如 1.0f, 1.25f, 2.0f)。</returns>
    COMPAT_NODISCARD virtual float GetDpiScale() const noexcept = 0;

    /// <summary>設定視窗標題。</summary>
    /// <param name="title">標題文字視圖。</param>
    virtual void SetTitle(compat::string_view title) = 0;

    // 原生控制代碼 (例如 Win32 HWND, Cocoa NSWindow, Wayland wl_surface)

    /// <summary>取得原生平台視窗控制代碼 (如 HWND)。</summary>
    /// <returns>原生指標/代碼。</returns>
    COMPAT_NODISCARD virtual void* GetNativeHandle() const noexcept = 0;

    // 事件通知回呼

    /// <summary>事件通知回呼委派。</summary>
    using EventCallback = std::function<void(const RawInputEvent&)>;

    /// <summary>設定原始輸入事件回呼函式。</summary>
    /// <param name="callback">事件回呼委派。</param>
    virtual void SetEventCallback(EventCallback callback) = 0;

    // 訊息輪詢與等待

    /// <summary>非阻塞輪詢原生平台事件佇列。</summary>
    virtual void PollEvents() = 0;

    /// <summary>阻塞等待原生平台事件佇列。</summary>
    /// <param name="timeoutMs">逾時毫秒數，-1 表示無限等待。</param>
    /// <returns>若有事件處理傳回 true，若逾時傳回 false。</returns>
    virtual bool WaitEvents(int32_t timeoutMs = -1) = 0;

    /// <summary>
    /// 喚醒處於 WaitEvents 等待狀態的原生訊息迴圈 (支援跨執行緒調用)。
    /// </summary>
    virtual void WakeUp() = 0;

    /// <summary>向原生平台請求觸發視窗重繪。</summary>
    virtual void RequestRedraw() = 0;

    /// <summary>即時重繪渲染回呼委派。</summary>
    using PaintCallback = std::function<void()>;

    /// <summary>設定重繪渲染回呼函式。</summary>
    /// <param name="callback">重繪回呼委派。</param>
    virtual void SetPaintCallback(PaintCallback callback) = 0;

    /// <summary>
    /// 將 32-bit BGRA/RGBA 像素緩衝區呈現在視窗表面 (span 多載)。
    /// </summary>
    /// <param name="pixelBuffer">像素緩衝區 span。</param>
    /// <param name="width">緩衝區寬度。</param>
    /// <param name="height">緩衝區高度。</param>
    virtual void PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) = 0;

    /// <summary>
    /// 幀緩衝呈現 raw 指標便利多載。
    /// </summary>
    /// <param name="pixelBuffer">像素緩衝區指標。</param>
    /// <param name="width">緩衝區寬度。</param>
    /// <param name="height">緩衝區高度。</param>
    void PresentFramebuffer(const uint32_t* pixelBuffer, uint32_t width, uint32_t height) {
        if (pixelBuffer && width > 0 && height > 0) {
            PresentFramebuffer(compat::span<const uint32_t>(pixelBuffer, static_cast<size_t>(width) * height), width, height);
        }
    }
};

} // namespace pal
} // namespace cppui
