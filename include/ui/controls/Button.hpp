/*
 * File: Button.hpp
 * Purpose: Declares Button control with click routing, states, and MVVM command execution.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include "compat/compat.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"

namespace cppui {
namespace mvvm {
class ICommand;
} // namespace mvvm

namespace controls {

class Button;

/// <summary>
/// 按鈕點擊路由事件參數 (ClickEventArgs)。
/// </summary>
class ClickEventArgs : public input::RoutedEventArgs {
public:
    /// <summary>
    /// 建構點擊事件參數。
    /// </summary>
    /// <param name="source">引發點擊之按鈕來源物件。</param>
    explicit ClickEventArgs(Button* source) noexcept;
};

using ClickEventHandler = std::function<void(Button&)>;

/// <summary>
/// 互動式按鈕控制項 (Button)
/// 實作完整的滑鼠懸停、按壓、獲焦霓虹外框與鍵盤空白鍵/Enter觸發狀態機，提供 Click 路由事件。
/// </summary>
class Button : public ContentControl {
public:
    static const input::RoutedEvent ClickEvent;

    /// <summary>
    /// 建構空白按鈕控制項。
    /// </summary>
    Button();

    /// <summary>
    /// 建構帶有標題文字之按鈕控制項。
    /// </summary>
    /// <param name="text">按鈕初始標題文字視圖。</param>
    explicit Button(compat::string_view text);

    ~Button() override = default;

    /// <summary>
    /// 檢查按鈕當前是否處於下壓狀態 (Pressed)。
    /// </summary>
    /// <returns>若正在下壓則回傳 true，否則回傳 false。</returns>
    COMPAT_NODISCARD bool IsPressed() const noexcept { return m_isPressed; }

    /// <summary>
    /// 檢查滑鼠指標當前是否懸停於按鈕上方 (Hovered)。
    /// </summary>
    /// <returns>若正在懸停則回傳 true，否則回傳 false。</returns>
    COMPAT_NODISCARD bool IsHovered() const noexcept { return m_isHovered; }

    /// <summary>
    /// 檢查按鈕是否處於可用啟用狀態。
    /// </summary>
    /// <returns>若啟用則回傳 true，禁用回傳 false。</returns>
    COMPAT_NODISCARD bool IsEnabled() const noexcept override { return Control::IsEnabled(); }

    /// <summary>
    /// 設定按鈕是否可用 (Enabled)。
    /// </summary>
    /// <param name="enabled">true 啟用，false 停用。</param>
    void SetEnabled(bool enabled) override;

    /// <summary>
    /// 設定滑鼠懸停時之背景色彩。
    /// </summary>
    /// <param name="color">懸停背景色。</param>
    void SetHoverBackground(core::Color color);

    /// <summary>
    /// 設定滑鼠下壓時之背景色彩。
    /// </summary>
    /// <param name="color">下壓背景色。</param>
    void SetPressedBackground(core::Color color);

    /// <summary>
    /// 設定獲得鍵盤焦點時之外框筆刷色彩。
    /// </summary>
    /// <param name="color">焦點外框色。</param>
    void SetFocusBorderBrush(core::Color color);

    /// <summary>
    /// 註冊按鈕點擊事件處理常式 (攜帶按鈕引用)。
    /// </summary>
    /// <param name="handler">事件回呼常式。</param>
    void OnClick(ClickEventHandler handler);

    /// <summary>
    /// 註冊無參數按鈕點擊事件處理常式。
    /// </summary>
    /// <param name="handler">事件回呼常式。</param>
    void OnClick(std::function<void()> handler);

    /// <summary>
    /// 綁定 MVVM 命令 (ICommand) 至按鈕。
    /// </summary>
    /// <param name="command">命令實例智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 command 為 nullptr 時擲出。</exception>
    void SetCommand(std::shared_ptr<mvvm::ICommand> command);

    /// <summary>
    /// 取得當前繫結之 MVVM 命令實例指標。
    /// </summary>
    /// <returns>ICommand 實例指標，若未繫結則為 nullptr。</returns>
    COMPAT_NODISCARD std::shared_ptr<mvvm::ICommand> GetCommand() const noexcept { return m_command; }

    /// <summary>
    /// 覆寫排版排列階段，將內容元件幾何置中排列於按鈕內部內距區域。
    /// </summary>
    /// <param name="finalSize">配置給按鈕之最終尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

    /// <summary>
    /// 繪製按鈕視覺外觀與當前互動狀態效果。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣至按鈕。
    /// </summary>
    /// <param name="theme">欲套用之 Theme 常數引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

protected:
    /// <summary>
    /// 觸發 Click 路由事件並執行繫結命令。
    /// </summary>
    void RaiseClick();

    /// <summary>
    /// 依據當前 Hover/Pressed/Enabled 狀態更新實際呈現之背景與外框色彩。
    /// </summary>
    void UpdateEffectiveColors();

    bool        m_isPressed = false;
    bool        m_isHovered = false;
    std::shared_ptr<mvvm::ICommand> m_command;

    core::Color m_normalBackground{ 45, 52, 70, 255 };
    core::Color m_hoverBackground{ 60, 75, 105, 255 };
    core::Color m_pressedBackground{ 32, 40, 58, 255 };
    core::Color m_disabledBackground{ 35, 38, 45, 255 };

    core::Color m_normalBorder{ 70, 80, 100, 255 };
    core::Color m_focusBorder{ 0, 160, 255, 255 }; // 獲焦時藍色霓虹框
};

} // namespace controls
} // namespace cppui
