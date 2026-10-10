/**
 * @file CheckBox.hpp
 * @brief 核取方塊控制項，支援勾選狀態切換、勾選標記繪製與狀態變更路由事件
 */

#pragma once

#include <functional>
#include "compat/compat.hpp"
#include "ui/controls/Button.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 核取方塊勾選狀態列舉 (CheckState，對標 .NET CheckState)。
/// </summary>
enum class CheckState : uint32_t {
    Unchecked     = 0,
    Checked       = 1,
    Indeterminate = 2
};

/// <summary>
/// 核取方塊控制項 (CheckBox)
/// 具備打勾狀態切換、三態未定模式、勾選標記向量繪製、自訂標籤與 Checked / Unchecked / Indeterminate 路由事件。
/// </summary>
class CheckBox : public Button {
public:
    using CheckState = cppui::controls::CheckState;

    static const input::RoutedEvent CheckedEvent;
    static const input::RoutedEvent UncheckedEvent;
    static const input::RoutedEvent IndeterminateEvent;

    /// <summary>
    /// 建構空白核取方塊控制項。
    /// </summary>
    CheckBox();

    /// <summary>
    /// 建構帶有標籤文字之核取方塊控制項。
    /// </summary>
    /// <param name="label">標籤文字視圖。</param>
    explicit CheckBox(compat::string_view label);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~CheckBox() override = default;

    /// <summary>
    /// 檢查當前是否處於勾選狀態。
    /// </summary>
    /// <returns>若已勾選回傳 true，否則回傳 false。</returns>
    COMPAT_NODISCARD bool IsChecked() const noexcept { return m_checkState == CheckState::Checked; }

    /// <summary>
    /// 設定核取方塊之勾選狀態，並視情況觸發 Checked 或 Unchecked 事件。
    /// </summary>
    /// <param name="checked">true 為勾選，false 為取消勾選。</param>
    void SetChecked(bool checked);

    /// <summary>
    /// 檢查是否支援未定 (三態) 勾選模式。
    /// </summary>
    /// <returns>若支援三態傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsThreeState() const noexcept { return m_isThreeState; }

    /// <summary>
    /// 設定是否啟用三態勾選模式。
    /// </summary>
    /// <param name="threeState">true 為啟用三態，false 為標準雙態。</param>
    void SetIsThreeState(bool threeState) noexcept { m_isThreeState = threeState; }

    /// <summary>
    /// 取得目前三態勾選狀態 (CheckState)。
    /// </summary>
    /// <returns>CheckState 列舉值。</returns>
    COMPAT_NODISCARD CheckState GetCheckState() const noexcept { return m_checkState; }

    /// <summary>
    /// 設定目前三態勾選狀態。
    /// </summary>
    /// <param name="state">CheckState 列舉值。</param>
    void SetCheckState(CheckState state);

    /// <summary>
    /// 反轉目前勾選狀態 (雙態時切換，三態時循環 Unchecked -> Checked -> Indeterminate)。
    /// </summary>
    void Toggle();

    /// <summary>
    /// 註冊勾選狀態變更回呼函式。
    /// </summary>
    /// <param name="handler">狀態變更回呼函式，接受目前勾選狀態。</param>
    void OnCheckedChanged(std::function<void(bool)> handler);

    /// <summary>
    /// 註冊未定狀態事件回呼函式。
    /// </summary>
    /// <param name="handler">事件回呼常式。</param>
    void OnIndeterminate(std::function<void()> handler);

    /// <summary>
    /// 覆寫尺寸度量，計算方塊與文字之綜合期望尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 覆寫排列階段，佈局方塊與文字相對位置。
    /// </summary>
    /// <param name="finalSize">實際分配尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

    /// <summary>
    /// 繪製核取方塊外框、勾選狀態標記與文字。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣至核取方塊。
    /// </summary>
    /// <param name="theme">欲套用之 Theme 常數引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    CheckState  m_checkState = CheckState::Unchecked;
    bool        m_isThreeState = false;
    float       m_boxSize = 18.0f;
    float       m_boxSpacing = 8.0f;

    core::Color m_checkColor{ 0, 160, 255, 255 }; // 勾選時藍色填色
};

} // namespace controls
} // namespace cppui
