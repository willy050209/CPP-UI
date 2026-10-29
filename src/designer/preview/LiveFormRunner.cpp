/**
 * @file LiveFormRunner.cpp
 * @brief CPPUI Designer AST 表單實例化工廠與試運行實作
 */

#include "ui/designer/preview/LiveFormRunner.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/RadioButton.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/controls/Slider.hpp"
#include "ui/controls/ProgressBar.hpp"
#include "ui/controls/ComboBox.hpp"
#include "ui/controls/ListBox.hpp"
#include "ui/controls/DataGrid.hpp"
#include "ui/controls/Label.hpp"
#include "ui/controls/PictureBox.hpp"
#include "ui/controls/Separator.hpp"
#include "ui/controls/ContentControl.hpp"
#include "ui/controls/ScrollViewer.hpp"
#include "ui/controls/Popup.hpp"
#include "ui/layout/Canvas.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/VirtualizingStackPanel.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/layout/Border.hpp"
#include "ui/designer/inspector/PropertyEditors.hpp"

namespace cppui {
namespace designer {
namespace preview {

namespace {

static core::Color ParseColorHelper(const PropertyValue& val, core::Color defaultColor = core::Color(0, 0, 0, 0)) {
    if (auto c = val.Get<core::Color>()) return *c;
    if (auto s = val.Get<std::string>()) {
        auto parsed = inspector::PropertyEditorModel::ParseColor(*s);
        if (parsed.has_value() && parsed->Is<core::Color>()) {
            return parsed->Get<core::Color>().value();
        }
    }
    return defaultColor;
}

static layout::Visibility ParseVisibilityHelper(const PropertyValue& val) {
    if (auto v = val.Get<layout::Visibility>()) return *v;
    if (auto s = val.Get<std::string>()) {
        if (*s == "Hidden") return layout::Visibility::Hidden;
        if (*s == "Collapsed") return layout::Visibility::Collapsed;
        return layout::Visibility::Visible;
    }
    return layout::Visibility::Visible;
}

static layout::HorizontalAlignment ParseHAlignHelper(const PropertyValue& val) {
    if (auto h = val.Get<layout::HorizontalAlignment>()) return *h;
    if (auto s = val.Get<std::string>()) {
        if (*s == "Left") return layout::HorizontalAlignment::Left;
        if (*s == "Center") return layout::HorizontalAlignment::Center;
        if (*s == "Right") return layout::HorizontalAlignment::Right;
        if (*s == "Stretch") return layout::HorizontalAlignment::Stretch;
    }
    return layout::HorizontalAlignment::Stretch;
}

static layout::VerticalAlignment ParseVAlignHelper(const PropertyValue& val) {
    if (auto v = val.Get<layout::VerticalAlignment>()) return *v;
    if (auto s = val.Get<std::string>()) {
        if (*s == "Top") return layout::VerticalAlignment::Top;
        if (*s == "Center") return layout::VerticalAlignment::Center;
        if (*s == "Bottom") return layout::VerticalAlignment::Bottom;
        if (*s == "Stretch") return layout::VerticalAlignment::Stretch;
    }
    return layout::VerticalAlignment::Stretch;
}

static text::FontWeight ParseFontWeightHelper(const PropertyValue& val) {
    if (auto i = val.Get<int32_t>()) return static_cast<text::FontWeight>(*i);
    if (auto s = val.Get<std::string>()) {
        if (*s == "Thin") return text::FontWeight::Thin;
        if (*s == "ExtraLight") return text::FontWeight::ExtraLight;
        if (*s == "Light") return text::FontWeight::Light;
        if (*s == "Normal" || *s == "Regular") return text::FontWeight::Normal;
        if (*s == "Medium") return text::FontWeight::Medium;
        if (*s == "SemiBold") return text::FontWeight::SemiBold;
        if (*s == "Bold") return text::FontWeight::Bold;
        if (*s == "ExtraBold") return text::FontWeight::ExtraBold;
        if (*s == "Black") return text::FontWeight::Black;
    }
    return text::FontWeight::Normal;
}

static float ParseFloatHelper(const PropertyValue& val, float defaultVal = 0.0f) {
    if (auto d = val.Get<double>()) return static_cast<float>(*d);
    if (auto i = val.Get<int32_t>()) return static_cast<float>(*i);
    if (auto s = val.Get<std::string>()) {
        try {
            return std::stof(*s);
        } catch (...) {}
    }
    return defaultVal;
}

void ApplyCommonProperties(const WidgetNode& node, const std::shared_ptr<core::UIElement>& elem) {
    if (!elem) return;

    if (auto p = node.GetProperty("Width")) {
        elem->SetWidth(ParseFloatHelper(*p));
    }
    if (auto p = node.GetProperty("Height")) {
        elem->SetHeight(ParseFloatHelper(*p));
    }
    if (auto p = node.GetProperty("Margin")) {
        if (p->Is<core::Thickness>()) elem->SetMargin(p->Get<core::Thickness>().value());
    }
    if (auto p = node.GetProperty("HorizontalAlignment")) {
        elem->SetHorizontalAlignment(ParseHAlignHelper(*p));
    }
    if (auto p = node.GetProperty("VerticalAlignment")) {
        elem->SetVerticalAlignment(ParseVAlignHelper(*p));
    }
    if (auto p = node.GetProperty("Visibility")) {
        elem->SetVisibility(ParseVisibilityHelper(*p));
    }
    if (auto p = node.GetProperty("IsEnabled")) {
        if (p->Is<bool>()) {
            if (auto ctrl = std::dynamic_pointer_cast<controls::Control>(elem)) {
                ctrl->SetEnabled(p->Get<bool>().value());
            }
        }
    }
    if (auto p = node.GetProperty("Opacity")) {
        elem->SetOpacity(ParseFloatHelper(*p, 1.0f));
    }

    // 附加屬性 (Canvas.Left / Top, Grid.Row / Column)
    if (auto p = node.GetAttachedProperty("Canvas.Left")) {
        layout::Canvas::SetLeft(elem, ParseFloatHelper(*p));
    }
    if (auto p = node.GetAttachedProperty("Canvas.Top")) {
        layout::Canvas::SetTop(elem, ParseFloatHelper(*p));
    }
    if (auto p = node.GetAttachedProperty("Grid.Row")) {
        if (p->Is<int32_t>()) layout::Grid::SetRow(*elem, static_cast<size_t>(p->Get<int32_t>().value()));
    }
    if (auto p = node.GetAttachedProperty("Grid.Column")) {
        if (p->Is<int32_t>()) layout::Grid::SetColumn(*elem, static_cast<size_t>(p->Get<int32_t>().value()));
    }
    if (auto p = node.GetAttachedProperty("Grid.RowSpan")) {
        if (p->Is<int32_t>()) layout::Grid::SetRowSpan(*elem, static_cast<size_t>(p->Get<int32_t>().value()));
    }
    if (auto p = node.GetAttachedProperty("Grid.ColumnSpan")) {
        if (p->Is<int32_t>()) layout::Grid::SetColumnSpan(*elem, static_cast<size_t>(p->Get<int32_t>().value()));
    }
}

void ApplyControlProperties(const WidgetNode& node, const std::shared_ptr<controls::Control>& ctrl) {
    if (!ctrl) return;

    if (auto p = node.GetProperty("Background")) {
        ctrl->SetBackground(ParseColorHelper(*p));
    }
    if (auto p = node.GetProperty("Foreground")) {
        ctrl->SetForeground(ParseColorHelper(*p, core::Color(255, 255, 255, 255)));
    }
    if (auto p = node.GetProperty("FontSize")) {
        ctrl->SetFontSize(ParseFloatHelper(*p, 14.0f));
    }
    if (auto p = node.GetProperty("FontFamily")) {
        if (auto s = p->Get<std::string>()) ctrl->SetFontFamily(*s);
    }
    if (auto p = node.GetProperty("FontWeight")) {
        ctrl->SetFontWeight(ParseFontWeightHelper(*p));
    }
    if (auto p = node.GetProperty("Padding")) {
        if (p->Is<core::Thickness>()) ctrl->SetPadding(p->Get<core::Thickness>().value());
    }
    if (auto p = node.GetProperty("BorderBrush")) {
        ctrl->SetBorderBrush(ParseColorHelper(*p));
    }
    if (auto p = node.GetProperty("BorderThickness")) {
        if (p->Is<core::Thickness>()) ctrl->SetBorderThickness(p->Get<core::Thickness>().value());
    }
    if (auto p = node.GetProperty("CornerRadius")) {
        ctrl->SetCornerRadius(ParseFloatHelper(*p));
    }
    if (auto p = node.GetProperty("AutoSize")) {
        if (p->Is<bool>()) {
            ctrl->SetAutoSize(p->Get<bool>().value());
        }
    }
}

} // anonymous namespace

std::shared_ptr<core::UIElement> LiveFormRunner::CreateWidget(
    const WidgetNode& node,
    LiveEventCallback eventCallback
) {
    std::shared_ptr<core::UIElement> element;
    const std::string uuid = node.GetUuid();

    switch (node.GetType()) {
        case NodeType::Canvas: {
            auto canvas = std::make_shared<layout::Canvas>();
            for (size_t i = 0; i < node.GetChildCount(); ++i) {
                if (auto child = CreateWidget(*node.GetChild(i), eventCallback)) {
                    canvas->AddChild(child);
                }
            }
            element = canvas;
            break;
        }
        case NodeType::StackPanel: {
            auto orientation = layout::Orientation::Vertical;
            if (auto p = node.GetProperty("Orientation")) {
                if (p->Is<layout::Orientation>()) orientation = p->Get<layout::Orientation>().value();
            }
            auto stack = std::make_shared<layout::StackPanel>(orientation);
            if (auto p = node.GetProperty("Spacing")) {
                if (p->Is<double>()) stack->SetSpacing(static_cast<float>(p->Get<double>().value()));
            }
            for (size_t i = 0; i < node.GetChildCount(); ++i) {
                if (auto child = CreateWidget(*node.GetChild(i), eventCallback)) {
                    stack->AddChild(child);
                }
            }
            element = stack;
            break;
        }
        case NodeType::VirtualizingStackPanel: {
            auto vstack = std::make_shared<layout::VirtualizingStackPanel>();
            element = vstack;
            break;
        }
        case NodeType::Grid: {
            auto grid = std::make_shared<layout::Grid>();
            for (const auto& rowDef : node.GetRowDefinitions()) {
                grid->AddRow(rowDef);
            }
            for (const auto& colDef : node.GetColumnDefinitions()) {
                grid->AddColumn(colDef);
            }
            for (size_t i = 0; i < node.GetChildCount(); ++i) {
                if (auto child = CreateWidget(*node.GetChild(i), eventCallback)) {
                    grid->AddChild(child);
                }
            }
            element = grid;
            break;
        }
        case NodeType::Border: {
            auto border = std::make_shared<layout::Border>();
            if (auto p = node.GetProperty("Background")) {
                border->SetBackground(ParseColorHelper(*p));
            }
            if (auto p = node.GetProperty("BorderBrush")) {
                border->SetBorderBrush(ParseColorHelper(*p));
            }
            if (auto p = node.GetProperty("BorderThickness")) {
                if (p->Is<core::Thickness>()) border->SetBorderThickness(p->Get<core::Thickness>().value());
            }
            if (auto p = node.GetProperty("CornerRadius")) {
                if (p->Is<double>()) border->SetCornerRadius(static_cast<float>(p->Get<double>().value()));
            }
            if (auto p = node.GetProperty("Padding")) {
                if (p->Is<core::Thickness>()) border->SetPadding(p->Get<core::Thickness>().value());
            }
            if (node.GetChildCount() > 0) {
                if (auto child = CreateWidget(*node.GetChild(0), eventCallback)) {
                    border->SetChild(child);
                }
            }
            element = border;
            break;
        }
        case NodeType::Button: {
            std::string content;
            if (auto p = node.GetProperty("Content")) {
                if (p->Is<std::string>()) content = p->Get<std::string>().value();
            }
            auto btn = std::make_shared<controls::Button>(content);
            if (eventCallback) {
                btn->OnClick([eventCallback, uuid]() {
                    eventCallback(uuid, "Click");
                });
            }
            element = btn;
            break;
        }
        case NodeType::TextBox: {
            std::string text;
            if (auto p = node.GetProperty("Text")) {
                if (p->Is<std::string>()) text = p->Get<std::string>().value();
            }
            auto tb = std::make_shared<controls::TextBox>(text);
            if (auto p = node.GetProperty("Placeholder")) {
                if (p->Is<std::string>()) tb->SetPlaceholder(p->Get<std::string>().value());
            }
            if (auto p = node.GetProperty("IsReadOnly")) {
                if (p->Is<bool>()) tb->SetReadOnly(p->Get<bool>().value());
            }
            element = tb;
            break;
        }
        case NodeType::TextBlock: {
            std::string text;
            if (auto pt = node.GetProperty("Text")) {
                if (pt->Is<std::string>()) text = pt->Get<std::string>().value();
            } else if (auto pc = node.GetProperty("Content")) {
                if (pc->Is<std::string>()) text = pc->Get<std::string>().value();
            }
            auto txt = std::make_shared<controls::TextBlock>(text);
            if (auto p = node.GetProperty("Foreground")) {
                txt->SetForeground(ParseColorHelper(*p, core::Color(255, 255, 255, 255)));
            }
            if (auto p = node.GetProperty("FontSize")) {
                txt->SetFontSize(ParseFloatHelper(*p, 14.0f));
            }
            if (auto p = node.GetProperty("FontFamily")) {
                if (auto s = p->Get<std::string>()) txt->SetFontFamily(*s);
            }
            if (auto p = node.GetProperty("FontWeight")) {
                txt->SetFontWeight(ParseFontWeightHelper(*p));
            }
            element = txt;
            break;
        }
        case NodeType::CheckBox: {
            std::string content;
            if (auto p = node.GetProperty("Content")) {
                if (p->Is<std::string>()) content = p->Get<std::string>().value();
            }
            auto cb = std::make_shared<controls::CheckBox>(content);
            if (auto p = node.GetProperty("IsChecked")) {
                if (p->Is<bool>()) cb->SetChecked(p->Get<bool>().value());
            }
            element = cb;
            break;
        }
        case NodeType::RadioButton: {
            std::string content;
            if (auto p = node.GetProperty("Content")) {
                if (p->Is<std::string>()) content = p->Get<std::string>().value();
            }
            auto rb = std::make_shared<controls::RadioButton>(content);
            if (auto p = node.GetProperty("IsChecked")) {
                if (p->Is<bool>()) rb->SetChecked(p->Get<bool>().value());
            }
            element = rb;
            break;
        }
        case NodeType::Slider: {
            auto sl = std::make_shared<controls::Slider>();
            if (auto p = node.GetProperty("Minimum")) {
                if (p->Is<double>()) sl->SetMinimum(static_cast<float>(p->Get<double>().value()));
            }
            if (auto p = node.GetProperty("Maximum")) {
                if (p->Is<double>()) sl->SetMaximum(static_cast<float>(p->Get<double>().value()));
            }
            if (auto p = node.GetProperty("Value")) {
                if (p->Is<double>()) sl->SetValue(static_cast<float>(p->Get<double>().value()));
            }
            element = sl;
            break;
        }
        case NodeType::ProgressBar: {
            auto pb = std::make_shared<controls::ProgressBar>();
            if (auto p = node.GetProperty("Minimum")) {
                if (p->Is<double>()) pb->SetMinimum(static_cast<float>(p->Get<double>().value()));
            }
            if (auto p = node.GetProperty("Maximum")) {
                if (p->Is<double>()) pb->SetMaximum(static_cast<float>(p->Get<double>().value()));
            }
            if (auto p = node.GetProperty("Value")) {
                if (p->Is<double>()) pb->SetValue(static_cast<float>(p->Get<double>().value()));
            }
            element = pb;
            break;
        }
        case NodeType::ComboBox: {
            auto cb = std::make_shared<controls::ComboBox>();
            element = cb;
            break;
        }
        case NodeType::ListBox: {
            auto lb = std::make_shared<controls::ListBox>();
            element = lb;
            break;
        }
        case NodeType::DataGrid: {
            auto dg = std::make_shared<controls::DataGrid>();
            element = dg;
            break;
        }
        case NodeType::Label: {
            std::string text;
            if (auto p = node.GetProperty("Content")) {
                if (p->Is<std::string>()) text = p->Get<std::string>().value();
            }
            auto lbl = std::make_shared<controls::Label>(text);
            element = lbl;
            break;
        }
        case NodeType::PictureBox: {
            auto pb = std::make_shared<controls::PictureBox>();
            if (auto p = node.GetProperty("AutoSize")) {
                if (p->Is<bool>() && p->Get<bool>().value()) {
                    pb->SetSizeMode(graphics::PictureBoxSizeMode::AutoSize);
                }
            }
            element = pb;
            break;
        }
        case NodeType::Separator: {
            auto sep = std::make_shared<controls::Separator>();
            element = sep;
            break;
        }
        case NodeType::ScrollViewer: {
            auto sv = std::make_shared<controls::ScrollViewer>();
            if (node.GetChildCount() > 0) {
                if (auto child = CreateWidget(*node.GetChild(0), eventCallback)) {
                    sv->SetContent(child);
                }
            }
            element = sv;
            break;
        }
        case NodeType::Popup: {
            auto popup = std::make_shared<controls::Popup>();
            element = popup;
            break;
        }
        case NodeType::ContentControl:
        default: {
            auto cc = std::make_shared<controls::ContentControl>();
            if (node.GetChildCount() > 0) {
                if (auto child = CreateWidget(*node.GetChild(0), eventCallback)) {
                    cc->SetContent(child);
                }
            }
            element = cc;
            break;
        }
    }

    if (auto ctrl = std::dynamic_pointer_cast<controls::Control>(element)) {
        ApplyControlProperties(node, ctrl);
    }
    ApplyCommonProperties(node, element);
    return element;
}

std::shared_ptr<app::Form> LiveFormRunner::CreateForm(
    const FormDocumentNode& doc,
    LiveEventCallback eventCallback
) {
    auto form = std::make_shared<app::Form>();

    const auto& settings = doc.GetSettings();
    form->SetTitle(settings.title);
    form->SetFormSize(
        static_cast<uint32_t>(settings.initialSize.width),
        static_cast<uint32_t>(settings.initialSize.height)
    );

    if (settings.autoSize) {
        form->SetAutoSize(true);
        if (settings.autoSizeMode == "GrowOnly") {
            form->SetAutoSizeMode(app::AutoSizeMode::GrowOnly);
        } else {
            form->SetAutoSizeMode(app::AutoSizeMode::GrowAndShrink);
        }
    }

    if (const auto* rootNode = doc.GetRoot()) {
        auto rootElement = CreateWidget(*rootNode, std::move(eventCallback));
        if (rootElement) {
            form->SetContent(rootElement);
        }
        if (auto p = rootNode->GetProperty("Background")) {
            form->SetBackground(ParseColorHelper(*p));
        }
        if (auto p = rootNode->GetProperty("Padding")) {
            if (p->Is<core::Thickness>()) form->SetPadding(p->Get<core::Thickness>().value());
        }
    }

    return form;
}

app::DialogResult LiveFormRunner::RunModal(
    const FormDocumentNode& doc,
    std::shared_ptr<app::Form> owner,
    LiveEventCallback eventCallback
) {
    auto form = CreateForm(doc, std::move(eventCallback));
    return form->ShowDialog(owner);
}

} // namespace preview
} // namespace designer
} // namespace cppui
