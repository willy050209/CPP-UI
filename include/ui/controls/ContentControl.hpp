#pragma once

#include <memory>
#include <string>
#include "compat/compat.hpp"
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
    ContentControl();
    explicit ContentControl(compat::string_view text);
    explicit ContentControl(std::shared_ptr<core::UIElement> content);
    ~ContentControl() override = default;

    COMPAT_NODISCARD std::shared_ptr<core::UIElement> GetContent() const noexcept { return m_content; }
    void SetContent(std::shared_ptr<core::UIElement> content);
    void SetContent(compat::string_view text);

    COMPAT_NODISCARD compat::string_view GetContentText() const noexcept;
    void SetContentText(compat::string_view text);

    core::Size MeasureOverride(core::Size availableSize) override;
    void ArrangeOverride(core::Size finalSize) override;
    void OnRender(rendering::DrawingContext& dc) override;

protected:
    std::shared_ptr<core::UIElement> m_content;
};

} // namespace controls
} // namespace cppui
