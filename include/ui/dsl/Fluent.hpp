#pragma once

#include <memory>
#include <vector>
#include <string>
#include <initializer_list>
#include <functional>
#include <type_traits>

#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/Canvas.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/controls/Control.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/mvvm/ICommand.hpp"

namespace cppui {
namespace dsl {

/// <summary>
/// 宣告式 Fluent Builder 包裝樣板類別 (UIBuilder)
/// 內部持有 std::shared_ptr<T>，提供型別安全、零成本的現代 C++ 鏈式屬性設定與階層樹建構能力。
/// </summary>
template <typename T>
class UIBuilder {
public:
    UIBuilder() : m_element(std::make_shared<T>()) {}
    explicit UIBuilder(std::shared_ptr<T> elem) : m_element(std::move(elem)) {}

    // --- 隱式型別轉換 (允許無縫傳入容器的 initializer_list) ---
    operator std::shared_ptr<T>() const noexcept { return m_element; }
    operator std::shared_ptr<core::UIElement>() const noexcept { return m_element; }
    operator std::shared_ptr<core::Visual>() const noexcept { return m_element; }

    COMPAT_NODISCARD std::shared_ptr<T> Get() const noexcept { return m_element; }
    COMPAT_NODISCARD T* operator->() const noexcept { return m_element.get(); }
    COMPAT_NODISCARD T& operator*() const noexcept { return *m_element; }

    // --- UIElement 基礎幾何與排版屬性 ---

    UIBuilder& Width(float w) { m_element->SetWidth(w); return *this; }
    UIBuilder& Height(float h) { m_element->SetHeight(h); return *this; }
    UIBuilder& MinWidth(float mw) { m_element->SetMinWidth(mw); return *this; }
    UIBuilder& MaxWidth(float mw) { m_element->SetMaxWidth(mw); return *this; }
    UIBuilder& MinHeight(float mh) { m_element->SetMinHeight(mh); return *this; }
    UIBuilder& MaxHeight(float mh) { m_element->SetMaxHeight(mh); return *this; }

    UIBuilder& Margin(const core::Thickness& m) { m_element->SetMargin(m); return *this; }
    UIBuilder& Margin(float uniform) { m_element->SetMargin(core::Thickness(uniform)); return *this; }
    UIBuilder& Margin(float h, float v) { m_element->SetMargin(core::Thickness(h, v, h, v)); return *this; }
    UIBuilder& Margin(float l, float t, float r, float b) { m_element->SetMargin(core::Thickness(l, t, r, b)); return *this; }

    UIBuilder& HorizontalAlignment(layout::HorizontalAlignment ha) { m_element->SetHorizontalAlignment(ha); return *this; }
    UIBuilder& VerticalAlignment(layout::VerticalAlignment va) { m_element->SetVerticalAlignment(va); return *this; }
    UIBuilder& Visibility(layout::Visibility v) { m_element->SetVisibility(v); return *this; }
    UIBuilder& Visible(bool v) {
        m_element->SetVisibility(v ? layout::Visibility::Visible : layout::Visibility::Collapsed);
        return *this;
    }

    UIBuilder& Opacity(float o) { m_element->SetOpacity(o); return *this; }
    UIBuilder& Focusable(bool f) { m_element->SetFocusable(f); return *this; }
    UIBuilder& TabIndex(int32_t idx) { m_element->SetTabIndex(idx); return *this; }
    UIBuilder& TabStop(bool ts) { m_element->SetTabStop(ts); return *this; }

    // --- Grid 附加屬性捷徑 ---
    UIBuilder& Row(size_t r) { layout::Grid::SetRow(*m_element, r); return *this; }
    UIBuilder& Column(size_t c) { layout::Grid::SetColumn(*m_element, c); return *this; }
    UIBuilder& RowSpan(size_t rs) { layout::Grid::SetRowSpan(*m_element, rs); return *this; }
    UIBuilder& ColumnSpan(size_t cs) { layout::Grid::SetColumnSpan(*m_element, cs); return *this; }

    // --- Canvas 附加屬性捷徑 ---
    UIBuilder& Left(float l) { layout::Canvas::SetLeft(*m_element, l); return *this; }
    UIBuilder& Top(float t) { layout::Canvas::SetTop(*m_element, t); return *this; }

    // --- 控制項外觀樣式屬性 (動態相容 Control, Border, TextBlock) ---

    UIBuilder& Background(core::Color bg) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetBackground(bg);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetBackground(bg);
        }
        return *this;
    }

    UIBuilder& Foreground(core::Color fg) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetForeground(fg);
        } else if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetForeground(fg);
        }
        return *this;
    }

    UIBuilder& BorderBrush(core::Color brush) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetBorderBrush(brush);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetBorderBrush(brush);
        }
        return *this;
    }

    UIBuilder& BorderThickness(const core::Thickness& th) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetBorderThickness(th);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetBorderThickness(th);
        }
        return *this;
    }
    UIBuilder& BorderThickness(float uniform) { return BorderThickness(core::Thickness(uniform)); }
    UIBuilder& BorderThickness(float h, float v) { return BorderThickness(core::Thickness(h, v)); }
    UIBuilder& BorderThickness(float l, float t, float r, float b) { return BorderThickness(core::Thickness(l, t, r, b)); }

    UIBuilder& CornerRadius(float r) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetCornerRadius(r);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetCornerRadius(r);
        }
        return *this;
    }

    UIBuilder& Padding(const core::Thickness& p) {
        if (auto* ctrl = dynamic_cast<controls::Control*>(m_element.get())) {
            ctrl->SetPadding(p);
        } else if (auto* b = dynamic_cast<layout::Border*>(m_element.get())) {
            b->SetPadding(p);
        }
        return *this;
    }
    UIBuilder& Padding(float uniform) { return Padding(core::Thickness(uniform)); }
    UIBuilder& Padding(float h, float v) { return Padding(core::Thickness(h, v, h, v)); }

    // --- 文字排版樣式屬性 ---

    UIBuilder& FontSize(float size) {
        if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetFontSize(size);
        } else if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->SetFontSize(size);
        }
        return *this;
    }

    UIBuilder& FontFamily(compat::string_view family) {
        if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetFontFamily(family);
        } else if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->SetFontFamily(family);
        }
        return *this;
    }

    UIBuilder& FontWeight(text::FontWeight weight) {
        if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetFontWeight(weight);
        }
        return *this;
    }

    // --- StackPanel 專屬屬性 ---
    UIBuilder& Spacing(float s) {
        if (auto* sp = dynamic_cast<layout::StackPanel*>(m_element.get())) {
            sp->SetSpacing(s);
        }
        return *this;
    }

    UIBuilder& Orientation(layout::Orientation o) {
        if (auto* sp = dynamic_cast<layout::StackPanel*>(m_element.get())) {
            sp->SetOrientation(o);
        }
        return *this;
    }

    // --- Button 專屬事件掛載 ---
    template <typename Handler>
    UIBuilder& OnClick(Handler&& handler) {
        if (auto* btn = dynamic_cast<controls::Button*>(m_element.get())) {
            btn->OnClick(std::forward<Handler>(handler));
        }
        return *this;
    }

    // --- CheckBox 專屬屬性與事件 ---
    UIBuilder& Checked(bool isChecked) {
        if (auto* cb = dynamic_cast<controls::CheckBox*>(m_element.get())) {
            cb->SetChecked(isChecked);
        }
        return *this;
    }

    template <typename Handler>
    UIBuilder& OnCheckedChanged(Handler&& handler) {
        if (auto* cb = dynamic_cast<controls::CheckBox*>(m_element.get())) {
            cb->OnCheckedChanged(std::forward<Handler>(handler));
        }
        return *this;
    }

    // --- TextBox 專屬屬性與事件 ---
    UIBuilder& Text(compat::string_view text) {
        if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->SetText(text);
        } else if (auto* tb = dynamic_cast<controls::TextBlock*>(m_element.get())) {
            tb->SetText(text);
        }
        return *this;
    }

    UIBuilder& Placeholder(compat::string_view placeholder) {
        if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->SetPlaceholder(placeholder);
        }
        return *this;
    }

    template <typename Handler>
    UIBuilder& OnTextChanged(Handler&& handler) {
        if (auto* tbox = dynamic_cast<controls::TextBox*>(m_element.get())) {
            tbox->OnTextChanged(std::forward<Handler>(handler));
        }
        return *this;
    }

    // --- Signal 響應式文字綁定 ---
    template <typename ValType, typename Formatter>
    UIBuilder& BindText(reactive::Signal<ValType>& sig, Formatter formatter) {
        if (dynamic_cast<controls::TextBlock*>(m_element.get())) {
            auto sharedTb = std::dynamic_pointer_cast<controls::TextBlock>(m_element);
            sig.Bind(sharedTb, [formatter](controls::TextBlock& t, const ValType& val) {
                t.SetText(formatter(val));
            });
        }
        return *this;
    }

    // --- MVVM 命令與 DataContext 綁定 ---
    UIBuilder& Command(std::shared_ptr<mvvm::ICommand> cmd) {
        if (auto* btn = dynamic_cast<controls::Button*>(m_element.get())) {
            btn->SetCommand(cmd);
        }
        return *this;
    }

    UIBuilder& DataContext(std::shared_ptr<void> dc) {
        if (m_element) {
            m_element->SetDataContext(dc);
        }
        return *this;
    }

private:
    std::shared_ptr<T> m_element;
};

// =============================================================================
// 工廠與宣告式 DSL 輔助函式 (Declarative DSL Factory Functions)
// =============================================================================

/// <summary>
/// 建立垂直流式排版容器 (VStack)
/// </summary>
inline UIBuilder<layout::StackPanel> VStack(std::initializer_list<std::shared_ptr<core::UIElement>> children = {}) {
    auto panel = std::make_shared<layout::StackPanel>();
    panel->SetOrientation(layout::Orientation::Vertical);
    for (const auto& child : children) {
        if (child) panel->AddChild(child);
    }
    return UIBuilder<layout::StackPanel>(panel);
}

/// <summary>
/// 建立水平流式排版容器 (HStack)
/// </summary>
inline UIBuilder<layout::StackPanel> HStack(std::initializer_list<std::shared_ptr<core::UIElement>> children = {}) {
    auto panel = std::make_shared<layout::StackPanel>();
    panel->SetOrientation(layout::Orientation::Horizontal);
    for (const auto& child : children) {
        if (child) panel->AddChild(child);
    }
    return UIBuilder<layout::StackPanel>(panel);
}

/// <summary>
/// 建立邊框容器 (Border)
/// </summary>
inline UIBuilder<layout::Border> Border(std::shared_ptr<core::UIElement> child = nullptr) {
    auto border = std::make_shared<layout::Border>();
    if (child) border->SetChild(std::move(child));
    return UIBuilder<layout::Border>(border);
}

/// <summary>
/// 建立網格容器 (Grid)
/// </summary>
inline UIBuilder<layout::Grid> Grid(std::initializer_list<std::shared_ptr<core::UIElement>> children = {}) {
    auto grid = std::make_shared<layout::Grid>();
    for (const auto& child : children) {
        if (child) grid->AddChild(child);
    }
    return UIBuilder<layout::Grid>(grid);
}

/// <summary>
/// 建立文字標籤 (Label / TextBlock)
/// </summary>
inline UIBuilder<controls::TextBlock> Label(compat::string_view text = "") {
    return UIBuilder<controls::TextBlock>(std::make_shared<controls::TextBlock>(text));
}

/// <summary>
/// 建立互動按鈕 (Button)
/// </summary>
inline UIBuilder<controls::Button> Button(compat::string_view text = "") {
    return UIBuilder<controls::Button>(std::make_shared<controls::Button>(text));
}

/// <summary>
/// 建立核取方塊 (CheckBox)
/// </summary>
inline UIBuilder<controls::CheckBox> CheckBox(compat::string_view label = "") {
    return UIBuilder<controls::CheckBox>(std::make_shared<controls::CheckBox>(label));
}

/// <summary>
/// 建立文字編輯輸入框 (TextBox)
/// </summary>
inline UIBuilder<controls::TextBox> TextBox(compat::string_view placeholder = "") {
    return UIBuilder<controls::TextBox>(std::make_shared<controls::TextBox>(placeholder));
}

} // namespace dsl
} // namespace cppui
