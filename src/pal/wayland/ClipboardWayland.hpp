/*
 * File: ClipboardWayland.hpp
 * Purpose: Declares Linux native Wayland clipboard implementation using wl_data_device_manager.
 */

#pragma once

#if defined(CPPUI_HAS_WAYLAND) && CPPUI_HAS_WAYLAND

#include <cstdint>
#include <string>
#include <mutex>
#include "compat/Compat.hpp"
#include "ui/pal/IClipboard.hpp"

#include <wayland-client.h>

namespace cppui {
namespace pal {
namespace wayland {

/// <summary>
/// Linux Wayland 原生剪貼簿實作類別 (ClipboardWayland)
/// 實作 IClipboard 介面，透過 wl_data_device_manager、wl_data_source 與 wl_data_device
/// 與 Wayland 合成器及其他應用程式交換文字資料。若於無 Wayland 合成器之無頭環境運行，
/// 自動平滑降級為執行緒安全之純記憶體快取模式。
/// </summary>
class ClipboardWayland : public IClipboard {
public:
    /// <summary>建構子：初始化 Wayland 連線與剪貼簿狀態。</summary>
    ClipboardWayland();

    /// <summary>解構子：釋放 Wayland Data Device、Data Source 與連線資源。</summary>
    ~ClipboardWayland() override;

    // 禁止拷貝
    ClipboardWayland(const ClipboardWayland&) = delete;
    ClipboardWayland& operator=(const ClipboardWayland&) = delete;

    /// <summary>查詢剪貼簿目前是否包含文字。</summary>
    /// <returns>若包含文字傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool ContainsText() const override;

    /// <summary>自 Wayland 剪貼簿取得 UTF-8 編碼之文字。</summary>
    /// <returns>UTF-8 字串。</returns>
    COMPAT_NODISCARD std::string GetText() const override;

    /// <summary>將純文字設定至 Wayland 剪貼簿。</summary>
    /// <param name="text">字串視圖。</param>
    void SetText(compat::string_view text) override;

    /// <summary>清空 Wayland 剪貼簿內容。</summary>
    void Clear() override;

    /// <summary>
    /// 設定與主視窗共享之 Wayland Display、Seat 與 Data Device Manager。
    /// </summary>
    /// <param name="display">wl_display 連線指標。</param>
    /// <param name="seat">wl_seat 指標。</param>
    /// <param name="dataDeviceManager">wl_data_device_manager 指標。</param>
    void SetWaylandContext(struct wl_display* display, struct wl_seat* seat, struct wl_data_device_manager* dataDeviceManager);

    /// <summary>
    /// 重設 Wayland 共享連線。
    /// </summary>
    /// <param name="display">待解除之 wl_display 指標。</param>
    void ResetWaylandContext(struct wl_display* display);

    /// <summary>內部回呼使用：取得當前快取文字字串指標與長度。</summary>
    /// <returns>快取字串引用。</returns>
    COMPAT_NODISCARD const std::string& GetCachedText() const noexcept { return m_localText; }

    /// <summary>內部回呼使用：清除當前 Data Source 控制代碼。</summary>
    void OnDataSourceCancelled() noexcept;

    /// <summary>內部回呼使用：接收到新的 Data Offer。</summary>
    /// <param name="offer">wl_data_offer 指標。</param>
    void OnDataOffer(struct wl_data_offer* offer) noexcept;

    /// <summary>內部回呼使用：選擇剪貼簿 Offer。</summary>
    /// <param name="offer">wl_data_offer 指標。</param>
    void OnSelection(struct wl_data_offer* offer) noexcept;

    /// <summary>內部回呼使用：Offer 支援之 MIME 類型回呼。</summary>
    /// <param name="mimeType">MIME 類型字串。</param>
    void OnOfferMimeType(compat::string_view mimeType) noexcept;

private:
    void EnsureInitialized();
    void CleanupWayland();

    mutable std::mutex m_mutex;
    struct wl_display* m_display = nullptr;
    struct wl_registry* m_registry = nullptr;
    struct wl_seat* m_seat = nullptr;
    struct wl_data_device_manager* m_dataDeviceManager = nullptr;
    struct wl_data_device* m_dataDevice = nullptr;
    struct wl_data_source* m_dataSource = nullptr;
    struct wl_data_offer* m_currentOffer = nullptr;

    bool m_ownsDisplay = false;
    bool m_isInitialized = false;
    mutable bool m_offerHasText = false;
    mutable std::string m_localText;
};

} // namespace wayland
} // namespace pal
} // namespace cppui

#endif // CPPUI_HAS_WAYLAND
