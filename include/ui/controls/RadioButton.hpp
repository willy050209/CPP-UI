/**
 * @file RadioButton.hpp
 * @brief 單選按鈕控制項，支援群組互斥選取、圓形向量指示標記與狀態變更事件
 */

#pragma once

#include <functional>
#include <string>
#include "compat/compat.hpp"
#include "ui/controls/Button.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 單選按鈕控制項 (RadioButton)
/// 繼承自 Button，提供圓形標記與文字標籤，支援同父容器或同 GroupName 互斥選取機制。
/// </summary>
class RadioButton : public Button {
public:
    static const input::RoutedEvent CheckedEvent;
    static const input::RoutedEvent UncheckedEvent;

    /// <summary>
    /// 建構空白單選按鈕控制項。
    /// </summary>
    RadioButton();

    /// <summary>
    /// 建構帶有標籤文字之單選按鈕控制項。
    /// </summary>
    /// <param name="label">標籤文字視圖。</param>
    explicit RadioButton(compat::string_view label);

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~RadioButton() override = default;

    /// <summary>
    /// 檢查當前單選按鈕是否被選取。
    /// </summary>
    /// <returns>若已選取傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsChecked() const noexcept { return m_isChecked; }

    /// <summary>
    /// 設定單選按鈕之選取狀態，並自動觸發同群組互斥排他。
    /// </summary>
    /// <param name="checked">true 為選取，false 為取消選取。</param>
    void SetChecked(bool checked);

    /// <summary>
    /// 取得單選按鈕所屬群組名稱 (GroupName)。
    /// </summary>
    /// <returns>群組名稱字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetGroupName() const noexcept { return m_groupName; }

    /// <summary>
    /// 設定單選按鈕所屬群組名稱 (空字串代表依所屬父容器自動分組)。
    /// </summary>
    /// <param name="name">群組名稱字串視圖。</param>
    void SetGroupName(compat::string_view name);

    /// <summary>
    /// 註冊選取狀態變更回呼函式。
    /// </summary>
    /// <param name="handler">回呼常式，接收當前選取狀態布林值。</param>
    void OnCheckedChanged(std::function<void(bool)> handler);

    /// <summary>
    /// 覆寫尺寸度量，計算圓圈與標籤文字所需期望尺寸。
    /// </summary>
    /// <param name="availableSize">父容器可用尺寸。</param>
    /// <returns>期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 覆寫排列階段，佈局圓圈與標籤文字相對位置。
    /// </summary>
    /// <param name="finalSize">實際分配尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

    /// <summary>
    /// 繪製單選按鈕外框、選取內核心圓與標籤文字。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void OnRender(rendering::DrawingContext& dc) override;

    /// <summary>
    /// 套用主題規格代幣至單選按鈕。
    /// </summary>
    /// <param name="theme">主題常數引用。</param>
    void ApplyTheme(const theme::Theme& theme) override;

private:
    void ClearOtherRadioButtonsInGroup();

    bool        m_isChecked = false;
    std::string m_groupName;
    float       m_circleRadius = 8.0f;
    float       m_boxSpacing = 8.0f;

    core::Color m_checkColor{ 0, 160, 255, 255 };
};

} // namespace controls
} // namespace cppui
