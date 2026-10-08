/*
 * File: PlatformWindowWin32.hpp
 * Purpose: Declares the Win32 platform window implementation for native Windows execution.
 */

#pragma once

#ifdef _WIN32

#include <cstdint>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>
#include <chrono>
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/core/CoordinateTransform.hpp"

namespace cppui {
namespace pal {
namespace win32 {

class PlatformWindowWin32 : public IPlatformWindow {
public:
    explicit PlatformWindowWin32(const WindowDesc& desc);
    ~PlatformWindowWin32() override;

    // IPlatformWindow 介面實作
    void Show() override;
    void Hide() override;
    void Close() override;
    bool IsClosed() const noexcept override { return m_isClosed; }

    core::Size GetSize() const noexcept override;
    void SetSize(uint32_t width, uint32_t height) override;
    core::Size GetFramebufferSize() const noexcept override;
    float GetDpiScale() const noexcept override { return m_dpiScale; }
    void SetTitle(compat::string_view title) override;

    void* GetNativeHandle() const noexcept override { return static_cast<void*>(m_hwnd); }

    void SetEventCallback(EventCallback callback) override {
        m_eventCallback = std::move(callback);
    }

    void PollEvents() override;
    bool WaitEvents(int32_t timeoutMs = -1) override;
    void WakeUp() override;

    void RequestRedraw() override;
    void SetPaintCallback(PaintCallback callback) override {
        m_paintCallback = std::move(callback);
    }

    void PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) override;

    HWND GetHwnd() const noexcept { return m_hwnd; }
    HDC  GetHdc() const noexcept { return m_hdc; }

private:
    static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void DispatchEvent(const RawInputEvent& event);
    KeyModifiers QueryCurrentModifiers() const noexcept;
    uint64_t GetCurrentTimestampMs() const noexcept;
    void UpdateDpiScale();

    HWND          m_hwnd = nullptr;
    HDC           m_hdc = nullptr;
    bool          m_isClosed = false;
    bool          m_isTrackingMouse = false;
    float                    m_dpiScale = 1.0f;
    core::CoordinateTransform m_coordTransform{ 1.0f };
    uint32_t                 m_width = 800;
    uint32_t                 m_height = 600;
    EventCallback m_eventCallback;
    PaintCallback m_paintCallback;
};

} // namespace win32
} // namespace pal
} // namespace cppui

#endif // _WIN32
