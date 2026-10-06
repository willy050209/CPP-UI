#pragma once

#include <functional>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/pal/RawInputEvent.hpp"

namespace cppui::pal {

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
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void Close() = 0;
    virtual bool IsClosed() const noexcept = 0;

    // 幾何與標題
    virtual core::Size GetSize() const noexcept = 0;
    virtual void SetSize(uint32_t width, uint32_t height) = 0;
    virtual core::Size GetFramebufferSize() const noexcept = 0;
    virtual float GetDpiScale() const noexcept = 0;
    virtual void SetTitle(compat::string_view title) = 0;

    // 原生控制代碼 (例如 Win32 HWND, Cocoa NSWindow, Wayland wl_surface)
    virtual void* GetNativeHandle() const noexcept = 0;

    // 事件通知回呼
    using EventCallback = std::function<void(const RawInputEvent&)>;
    virtual void SetEventCallback(EventCallback callback) = 0;

    // 訊息輪詢與等待
    virtual void PollEvents() = 0;
    virtual bool WaitEvents(int timeoutMs = -1) = 0;

    // 觸發視窗重繪請求
    virtual void RequestRedraw() = 0;
};

} // namespace cppui::pal
