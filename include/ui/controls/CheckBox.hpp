#pragma once

#include <functional>
#include "compat/compat.hpp"
#include "ui/controls/Button.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 核取方塊控制項 (CheckBox)
/// 具備打勾狀態切換、勾選標記向量繪製、自訂標籤與 Checked / Unchecked 路由事件。
/// </summary>
class CheckBox : public Button {
public:
    static const input::RoutedEvent CheckedEvent;
    static const input::RoutedEvent UncheckedEvent;

    CheckBox();
    explicit CheckBox(compat::string_view label);
    ~CheckBox() override = default;

    COMPAT_NODISCARD bool IsChecked() const noexcept { return m_isChecked; }
    void SetChecked(bool checked);
    void Toggle();

    // 便利回呼
    void OnCheckedChanged(std::function<void(bool)> handler);

    core::Size MeasureOverride(core::Size availableSize) override;
    void ArrangeOverride(core::Size finalSize) override;
    void OnRender(rendering::DrawingContext& dc) override;

private:
    bool m_isChecked = false;
    float m_boxSize = 18.0f;
    float m_boxSpacing = 8.0f;

    core::Color m_checkColor{ 0, 160, 255, 255 }; // 勾選時藍色填色
};

} // namespace controls
} // namespace cppui
