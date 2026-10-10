/**
 * @file Slider.hpp
 * @brief 數值滑桿控制項，支援水平/垂直方向、滑塊抓取拖曳、鍵盤導航與數值變更事件
 */

#pragma once

#include <functional>
#include "compat/compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/input/RoutedEvent.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 數值範圍拖曳滑桿控制項 (Slider / TrackBar)
/// 提供軌道與滑塊 (Thumb)，支援滑鼠拖曳、軌道跳躍點選與鍵盤方向鍵微調。
/// </summary>
class Slider : public Control {
public:
    static const input::RoutedEvent ValueChangedEvent;

    /// <summary>
    /// 建構預設滑桿控制項 (範圍 0.0 ~ 100.0，初始值 0.0)。
    /// </summary>
    Slider();

    /// <summary>
    /// 建構具備初始範圍與數值之滑桿控制項。
    /// </summary>
    /// <param name="min">最小值。</param>
    /// <param name="max">最大值。</param>
    /// <param name="value">初始值。</param>
    Slider(float min, float max, float value);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~Slider() override = default;

    /// <summary>
    /// 取得數值下限 (Minimum)。
    /// </summary>
    /// <returns>最小值。</returns>
    COMPAT_NODISCARD float GetMinimum() const noexcept { return m_minimum; }

    /// <summary>
    /// 設定數值下限。
    /// </summary>
    /// <param name="min">最小值。</param>
    void SetMinimum(float min);

    /// <summary>
    /// 取得數值上限 (Maximum)。
    /// </summary>
    /// <returns>最大值。</returns>
    COMPAT_NODISCARD float GetMaximum() const noexcept { return m_maximum; }

    /// <summary>
    /// 設定數值上限。
    /// </summary>
    /// <param name="max">最大值。</param>
    void SetMaximum(float max);

    /// <summary>
    /// 取得目前數值 (Value)。
    /// </summary>
    /// <returns>目前數值。</returns>
    COMPAT_NODISCARD float GetValue() const noexcept { return m_value; }

    /// <summary>
    /// 設定目前數值 (將自動限制於 [Minimum, Maximum] 範圍)。
    /// </summary>
    /// <param name="val">目標數值。</param>
    void SetValue(float val);

    /// <summary>
    /// 取得方向鍵小步進量 (SmallChange，預設為 1.0)。
    /// </summary>
    /// <returns>微調步進量。</returns>
    COMPAT_NODISCARD float GetSmallChange() const noexcept { return m_smallChange; }

    /// <summary>
    /// 設定方向鍵小步進量。
    /// </summary>
    /// <param name="change">微調步進量。</param>
    void SetSmallChange(float change) noexcept { m_smallChange = change; }

    /// <summary>
    /// 取得翻頁鍵大步進量 (LargeChange，預設為 10.0)。
    /// </summary>
    /// <returns>粗調步進量。</returns>
    COMPAT_NODISCARD float GetLargeChange() const noexcept { return m_largeChange; }

    /// <summary>
    /// 設定翻頁鍵大步進量。
    /// </summary>
    /// <param name="change">粗調步進量。</param>
    void SetLargeChange(float change) noexcept { m_largeChange = change; }

    /// <summary>
    /// 取得滑桿配置方向 (Horizontal / Vertical)。
    /// </summary>
    /// <returns>Orientation 列舉值。</returns>
    COMPAT_NODISCARD layout::Orientation GetOrientation() const noexcept { return m_orientation; }

    /// <summary>
    /// 設定滑桿配置方向。
    /// </summary>
    /// <param name="orientation">Orientation 列舉值。</param>
    void SetOrientation(layout::Orientation orientation);

    /// <summary>
    /// 註冊數值變更回呼函式。
    /// </summary>
    /// <param name="handler">回呼常式，接收最新數值。</param>
    void OnValueChanged(std::function<void(float)> handler);

    /// <summary>
    /// 覆寫排版測量階段，計算滑桿期望尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 繪製滑桿軌道、進度高亮槽與拖曳旋鈕 (Thumb)。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣至滑桿。
    /// </summary>
    /// <param name="theme">主題常數引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    void InitializeHandlers();
    void UpdateValueFromPosition(core::Point pos);
    core::Rect GetThumbRect() const noexcept;

    float               m_minimum = 0.0f;
    float               m_maximum = 100.0f;
    float               m_value = 0.0f;
    float               m_smallChange = 1.0f;
    float               m_largeChange = 10.0f;
    layout::Orientation m_orientation = layout::Orientation::Horizontal;

    float               m_thumbRadius = 7.0f;
    bool                m_isDragging = false;
    bool                m_isHovered = false;

    core::Color         m_trackColor{ 45, 52, 65, 255 };
    core::Color         m_activeTrackColor{ 0, 160, 255, 255 };
    core::Color         m_thumbColor{ 240, 245, 255, 255 };
};

} // namespace controls
} // namespace cppui
