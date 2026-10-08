/*
 * File: PlatformWindowGeneric.hpp
 * Purpose: Declares PlatformWindowGeneric for cross-platform headless and testing environments.
 */

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "compat/compat.hpp"
#include "ui/pal/IPlatformWindow.hpp"

namespace cppui {
namespace pal {

/// <summary>
/// 跨平台通用純 C++ 視窗實作 (Generic PAL Window)
/// 不依賴任何特定作業系統視窗 API (Win32, Cocoa, X11, Wayland)。
/// 內建記憶體 Framebuffer 容器與事件佇列，支援事件注入與像素快照，
/// 適用於跨平台環境、Headless CI 測試、伺服器端離線渲染以及無顯卡環境。
/// </summary>
class PlatformWindowGeneric : public IPlatformWindow {
public:
    explicit PlatformWindowGeneric(const WindowDesc& desc = WindowDesc{});
    ~PlatformWindowGeneric() override = default;

    // 視窗生命週期控制
    void Show() override;
    void Hide() override;
    void Close() override;
    COMPAT_NODISCARD bool IsClosed() const noexcept override;

    // 幾何與標題
    COMPAT_NODISCARD core::Size GetSize() const noexcept override;
    void SetSize(uint32_t width, uint32_t height) override;
    COMPAT_NODISCARD core::Size GetFramebufferSize() const noexcept override;
    COMPAT_NODISCARD float GetDpiScale() const noexcept override;
    void SetDpiScale(float dpiScale) noexcept;
    void SetTitle(compat::string_view title) override;
    COMPAT_NODISCARD compat::string_view GetTitle() const noexcept;

    // 原生控制代碼 (通用視窗回傳 nullptr 或由外部注入的虛擬控制代碼)
    COMPAT_NODISCARD void* GetNativeHandle() const noexcept override;
    void SetNativeHandle(void* handle) noexcept;

    // 事件通知與調度
    void SetEventCallback(EventCallback callback) override;
    void PollEvents() override;
    bool WaitEvents(int32_t timeoutMs = -1) override;
    void WakeUp() override;

    // 觸發視窗重繪請求
    void RequestRedraw() override;
    void SetPaintCallback(PaintCallback callback) override;

    // 幀緩衝呈現
    void PresentFramebuffer(compat::span<const uint32_t> pixelBuffer, uint32_t width, uint32_t height) override;

    // --- 跨平台測試與 Headless 專用輔助方法 ---

    /// <summary>
    /// 手動注入外部輸入事件至事件佇列，於下次 PollEvents() 時派發至回呼
    /// </summary>
    void InjectEvent(const RawInputEvent& event);

    /// <summary>
    /// 取得當前視窗所保存的 Framebuffer 像素視圖
    /// </summary>
    COMPAT_NODISCARD compat::span<const uint32_t> GetFramebufferPixels() const noexcept;

    /// <summary>
    /// 檢查是否有待處理的重繪請求
    /// </summary>
    COMPAT_NODISCARD bool HasPendingRedraw() const noexcept;

    /// <summary>
    /// 清除重繪標記 (通常由測試 harness 或渲染循環呼叫)
    /// </summary>
    void ClearRedrawRequest() noexcept;

private:
    std::string m_title;
    uint32_t m_width = 800;
    uint32_t m_height = 600;
    float m_dpiScale = 1.0f;
    bool m_isVisible = false;
    bool m_isClosed = false;
    bool m_pendingRedraw = false;
    void* m_nativeHandle = nullptr;

    EventCallback m_eventCallback;
    PaintCallback m_paintCallback;

    std::vector<uint32_t> m_framebuffer;
    std::vector<RawInputEvent> m_eventQueue;
    mutable std::mutex m_eventMutex;
    std::condition_variable m_eventCv;
};

} // namespace pal
} // namespace cppui
