/*
 * File: Popup.hpp
 * Purpose: Declares the Popup control that presents content in a floating overlay layer
 *          with smart placement, boundary auto-flip, and light-dismiss support.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "compat/compat.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/input/ILightDismissable.hpp"

namespace cppui {
namespace controls {

class OverlayLayer;

/// <summary>
/// 彈出層停靠定位模式列舉。
/// </summary>
enum class PlacementMode : uint8_t {
    Bottom = 0,
    Top,
    Left,
    Right,
    Mouse,
    Custom
};

/// <summary>
/// 懸浮彈出控制項 (Popup)
/// 透過 OverlayLayer 於視窗最頂層呈現內容，具備邊界防溢出自動翻轉、自訂偏移與 Light-Dismiss 外部關閉機制。
/// </summary>
class Popup : public ContentControl, public input::ILightDismissable {
public:
    using PopupEventHandler = std::function<void(Popup&)>;

    /// <summary>
    /// 預設建構子。
    /// </summary>
    Popup();

    /// <summary>
    /// 建構帶有特定內容之 Popup。
    /// </summary>
    /// <param name="content">內容元件指標。</param>
    explicit Popup(std::shared_ptr<core::UIElement> content);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~Popup() override;

    /// <summary>
    /// 查詢彈出層當前是否處於開啟狀態。
    /// </summary>
    /// <returns>若已開啟傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsOpen() const noexcept { return m_isOpen; }

    /// <summary>
    /// 設定彈出層開啟或關閉狀態。
    /// </summary>
    /// <param name="value">true 開啟，false 關閉。</param>
    void SetIsOpen(bool value) override;

    /// <summary>
    /// 查詢是否保持開啟 (若為 false 則啟用外部點擊或 ESC 鍵 Light-Dismiss 自動關閉)。
    /// </summary>
    /// <returns>若保持開啟傳回 true。</returns>
    COMPAT_NODISCARD bool StaysOpen() const noexcept override { return m_staysOpen; }

    /// <summary>
    /// 設定是否保持開啟。
    /// </summary>
    /// <param name="value">true 保持開啟，false 啟用自動收合。</param>
    void SetStaysOpen(bool value) noexcept { m_staysOpen = value; }

    /// <summary>
    /// 取得定位目標參考元件。
    /// </summary>
    /// <returns>目標元件指標，若未設定傳回 nullptr。</returns>
    COMPAT_NODISCARD core::UIElement* PlacementTarget() const noexcept override { return m_placementTarget; }

    /// <summary>
    /// 設定定位目標參考元件。
    /// </summary>
    /// <param name="target">目標元件指標。</param>
    void SetPlacementTarget(core::UIElement* target);

    /// <summary>
    /// 取得停靠定位模式。
    /// </summary>
    /// <returns>PlacementMode 列舉值。</returns>
    COMPAT_NODISCARD PlacementMode GetPlacementMode() const noexcept { return m_placementMode; }

    /// <summary>
    /// 設定停靠定位模式。
    /// </summary>
    /// <param name="mode">PlacementMode 列舉值。</param>
    void SetPlacementMode(PlacementMode mode) noexcept;

    /// <summary>
    /// 取得額外定位位移 (像素)。
    /// </summary>
    /// <returns>Point 座標位移。</returns>
    COMPAT_NODISCARD core::Point GetOffset() const noexcept { return m_offset; }

    /// <summary>
    /// 設定額外定位位移。
    /// </summary>
    /// <param name="offset">Point 座標位移。</param>
    void SetOffset(core::Point offset) noexcept;

    /// <summary>
    /// 取得水平額外位移。
    /// </summary>
    /// <returns>水平位移。</returns>
    COMPAT_NODISCARD float GetHorizontalOffset() const noexcept { return m_offset.x; }

    /// <summary>
    /// 設定水平額外位移。
    /// </summary>
    /// <param name="x">水平位移。</param>
    void SetHorizontalOffset(float x) noexcept;

    /// <summary>
    /// 取得垂直額外位移。
    /// </summary>
    /// <returns>垂直位移。</returns>
    COMPAT_NODISCARD float GetVerticalOffset() const noexcept { return m_offset.y; }

    /// <summary>
    /// 設定垂直額外位移。
    /// </summary>
    /// <param name="y">垂直位移。</param>
    void SetVerticalOffset(float y) noexcept;

    /// <summary>
    /// 重新計算並更新彈出層於視窗中的幾何位置（執行自動翻轉與邊界限制）。
    /// </summary>
    void UpdatePlacement();

    /// <summary>
    /// 註冊彈出層已開啟事件回呼。
    /// </summary>
    /// <param name="handler">事件委派常式。</param>
    void AddOpenedHandler(PopupEventHandler handler);

    /// <summary>
    /// 註冊彈出層已關閉事件回呼。
    /// </summary>
    /// <param name="handler">事件委派常式。</param>
    void AddClosedHandler(PopupEventHandler handler);

protected:
    /// <summary>
    /// 彈出層開啟時之虛擬回呼掛鉤。
    /// </summary>
    virtual void OnOpened();

    /// <summary>
    /// 彈出層關閉時之虛擬回呼掛鉤。
    /// </summary>
    virtual void OnClosed();

private:
    bool m_isOpen = false;
    bool m_staysOpen = false;
    core::UIElement* m_placementTarget = nullptr;
    PlacementMode m_placementMode = PlacementMode::Bottom;
    core::Point m_offset{ 0.0f, 0.0f };

    OverlayLayer* m_currentOverlayLayer = nullptr;
    std::vector<PopupEventHandler> m_openedHandlers;
    std::vector<PopupEventHandler> m_closedHandlers;
};

} // namespace controls
} // namespace cppui
