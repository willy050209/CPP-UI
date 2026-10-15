/*
 * File: ContentControl.hpp
 * Purpose: Declares ContentControl, the base class for controls containing a single child or text content.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include "compat/Compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/TextBlock.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 單內容容器控制項 (ContentControl)
/// 支援承載單一子元件（Content），內容可為字串文字或任意 UIElement 視覺節點。
/// </summary>
class ContentControl : public Control {
public:
    /// <summary>
    /// 建構空白內容控制項。
    /// </summary>
    ContentControl();

    /// <summary>
    /// 建構帶有純文字內容之 ContentControl。
    /// </summary>
    /// <param name="text">顯示字串視圖。</param>
    explicit ContentControl(compat::string_view text);

    /// <summary>
    /// 建構帶有視覺子節點之 ContentControl。
    /// </summary>
    /// <param name="content">子節點智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 content 為 nullptr 時擲出。</exception>
    explicit ContentControl(std::shared_ptr<core::UIElement> content);

    ~ContentControl() override = default;

    /// <summary>
    /// 取得當前內容節點。
    /// </summary>
    /// <returns>內容節點智慧指標，若無內容傳回 nullptr。</returns>
    COMPAT_NODISCARD std::shared_ptr<core::UIElement> GetContent() const noexcept { return m_content; }

    /// <summary>
    /// 設定視覺節點內容。
    /// </summary>
    /// <param name="content">子節點智慧指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">當 content 為 nullptr 時擲出。</exception>
    void SetContent(std::shared_ptr<core::UIElement> content);

    /// <summary>
    /// 設定純文字內容 (自動包裝為 TextBlock)。
    /// </summary>
    /// <param name="text">文字字串視圖。</param>
    void SetContent(compat::string_view text);

    /// <summary>
    /// 若內部承載 TextBlock，取得其字串內容。
    /// </summary>
    /// <returns>字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetContentText() const noexcept;

    /// <summary>
    /// 設定內部 TextBlock 字串內容或新建 TextBlock。
    /// </summary>
    /// <param name="text">文字字串視圖。</param>
    void SetContentText(compat::string_view text);

    /// <summary>
    /// 取得內容文字 (多型覆寫 Control::GetText)。
    /// </summary>
    /// <returns>文字字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetText() const noexcept override;

    /// <summary>
    /// 設定內容文字 (多型覆寫 Control::SetText)。
    /// </summary>
    /// <param name="text">文字字串視圖。</param>
    void SetText(compat::string_view text) override;

    /// <summary>
    /// 覆寫排版測量階段，計算包含 Padding 與 BorderThickness 之內容所需尺寸。
    /// </summary>
    /// <param name="availableSize">父容器提供之可用尺寸。</param>
    /// <returns>元件期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 覆寫排版排列階段，定位單一子內容至除去內距與邊框後之區域。
    /// </summary>
    /// <param name="finalSize">配置給控制項之最終尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

    /// <summary>
    /// 繪製背景與邊框外觀。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

protected:
    std::shared_ptr<core::UIElement> m_content;
};

} // namespace controls
} // namespace cppui
