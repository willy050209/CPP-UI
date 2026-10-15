/**
 * @file ProgressBar.hpp
 * @brief 進度指示條控制項，支援水平/垂直方向、平滑數值填色與步進運算
 */

#pragma once

#include <functional>
#include "compat/Compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/input/RoutedEvent.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 進度指示條控制項 (ProgressBar)
/// 支援數值範圍 (Min/Max/Value)、步進推進 (Step/Increment)、水平或垂直方向與未定狀態。
/// </summary>
class ProgressBar : public Control {
public:
    static const input::RoutedEvent ValueChangedEvent;

    /// <summary>
    /// 建構預設進度條控制項 (範圍 0.0 ~ 100.0，初始值 0.0)。
    /// </summary>
    ProgressBar();

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~ProgressBar() override = default;

    /// <summary>
    /// 取得進度下限數值 (Minimum)。
    /// </summary>
    /// <returns>最小值。</returns>
    COMPAT_NODISCARD float GetMinimum() const noexcept { return m_minimum; }

    /// <summary>
    /// 設定進度下限數值。
    /// </summary>
    /// <param name="min">最小值。</param>
    void SetMinimum(float min);

    /// <summary>
    /// 取得進度上限數值 (Maximum)。
    /// </summary>
    /// <returns>最大值。</returns>
    COMPAT_NODISCARD float GetMaximum() const noexcept { return m_maximum; }

    /// <summary>
    /// 設定進度上限數值。
    /// </summary>
    /// <param name="max">最大值。</param>
    void SetMaximum(float max);

    /// <summary>
    /// 取得目前進度數值 (Value)。
    /// </summary>
    /// <returns>目前值。</returns>
    COMPAT_NODISCARD float GetValue() const noexcept { return m_value; }

    /// <summary>
    /// 設定目前進度數值 (將自動限制於 [Minimum, Maximum] 範圍)。
    /// </summary>
    /// <param name="val">目標值。</param>
    void SetValue(float val);

    /// <summary>
    /// 取得步進量數值 (Step，預設為 10.0)。
    /// </summary>
    /// <returns>步進值。</returns>
    COMPAT_NODISCARD float GetStep() const noexcept { return m_step; }

    /// <summary>
    /// 設定步進量數值。
    /// </summary>
    /// <param name="step">步進值。</param>
    void SetStep(float step) noexcept { m_step = step; }

    /// <summary>
    /// 依目前設定之 Step 步進量推進進度值 (對標 WinForms PerformStep)。
    /// </summary>
    void PerformStep();

    /// <summary>
    /// 增加指定之進度增量 (對標 WinForms Increment)。
    /// </summary>
    /// <param name="val">欲增加之數值。</param>
    void Increment(float val);

    /// <summary>
    /// 取得進度條配置方向 (Horizontal / Vertical)。
    /// </summary>
    /// <returns>Orientation 列舉值。</returns>
    COMPAT_NODISCARD layout::Orientation GetOrientation() const noexcept { return m_orientation; }

    /// <summary>
    /// 設定進度條配置方向。
    /// </summary>
    /// <param name="orientation">Orientation 列舉值。</param>
    void SetOrientation(layout::Orientation orientation);

    /// <summary>
    /// 檢查是否處於未定跑馬燈狀態 (Indeterminate)。
    /// </summary>
    /// <returns>若為未定狀態傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsIndeterminate() const noexcept { return m_isIndeterminate; }

    /// <summary>
    /// 設定是否啟用未定跑馬燈狀態。
    /// </summary>
    /// <param name="indeterminate">true 啟用未定態，false 為確定數值態。</param>
    void SetIndeterminate(bool indeterminate);

    /// <summary>
    /// 註冊進度數值變更回呼函式。
    /// </summary>
    /// <param name="handler">回呼常式，接收最新數值。</param>
    void OnValueChanged(std::function<void(float)> handler);

    /// <summary>
    /// 覆寫排版測量階段，計算進度條期望尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 繪製進度條軌道槽與進度填充條。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣至進度條。
    /// </summary>
    /// <param name="theme">主題常數引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    float               m_minimum = 0.0f;
    float               m_maximum = 100.0f;
    float               m_value = 0.0f;
    float               m_step = 10.0f;
    layout::Orientation m_orientation = layout::Orientation::Horizontal;
    bool                m_isIndeterminate = false;

    core::Color         m_progressColor{ 0, 160, 255, 255 };
};

} // namespace controls
} // namespace cppui
