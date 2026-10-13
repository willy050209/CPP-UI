/*
 * File: PlatformWindowFactory.hpp
 * Purpose: Declares the factory class for creating platform-specific or generic windows.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include "ui/pal/IPlatformWindow.hpp"

namespace cppui {
namespace pal {

/// <summary>
/// 跨平台視窗工廠 (PlatformWindowFactory)
/// 負責根據作業系統環境與配置建立對應的原生視窗或通用虛擬視窗。
/// </summary>
class PlatformWindowFactory {
public:
    /// <summary>
    /// 依據當前作業系統建立最適的原生視窗實例 (Windows 上為 Win32，其他系統為 Generic)。
    /// </summary>
    /// <param name="desc">視窗建構描述符，寬高必須大於 0。</param>
    /// <returns>已建立之視窗實例智慧指標。</returns>
    /// <exception cref="std::invalid_argument">當 desc.width 或 desc.height 為 0 時擲出。</exception>
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> Create(const WindowDesc& desc = WindowDesc{});

    /// <summary>
    /// 明確建立純 C++ 跨平台通用視窗 (適用於 Headless CI、單元測試、無顯卡環境)。
    /// </summary>
    /// <param name="desc">視窗建構描述符，寬高必須大於 0。</param>
    /// <returns>已建立之視窗實例智慧指標。</returns>
    /// <exception cref="std::invalid_argument">當 desc.width 或 desc.height 為 0 時擲出。</exception>
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> CreateGeneric(const WindowDesc& desc = WindowDesc{});

    /// <summary>
    /// 明確建立 Linux X11 原生實體視窗 (適用於 WSLg、Linux 桌面環境)。
    /// </summary>
    /// <param name="desc">視窗建構描述符，寬高必須大於 0。</param>
    /// <returns>已建立之 X11 視窗實例智慧指標。</returns>
    /// <exception cref="std::invalid_argument">當 desc.width 或 desc.height 為 0 時擲出。</exception>
    /// <exception cref="std::runtime_error">當 X11 支援未編譯或無法連接 DISPLAY 時擲出。</exception>
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> CreateX11(const WindowDesc& desc = WindowDesc{});

    /// <summary>
    /// 明確建立 Linux Wayland 原生實體視窗 (適用於 GNOME、KDE 現代 Linux 桌面環境)。
    /// </summary>
    /// <param name="desc">視窗建構描述符，寬高必須大於 0。</param>
    /// <returns>已建立之 Wayland 視窗實例智慧指標。</returns>
    /// <exception cref="std::invalid_argument">當 desc.width 或 desc.height 為 0 時擲出。</exception>
    /// <exception cref="std::runtime_error">當 Wayland 支援未編譯或無法連接 WAYLAND_DISPLAY 時擲出。</exception>
    COMPAT_NODISCARD static std::shared_ptr<IPlatformWindow> CreateWayland(const WindowDesc& desc = WindowDesc{});
};

} // namespace pal
} // namespace cppui
