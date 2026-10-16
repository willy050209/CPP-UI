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

namespace cppui {
namespace designer {
namespace preview {

namespace {

void ApplyCommonProperties(const WidgetNode& node, const std::shared_ptr<core::UIElement>& elem) {
    if (!elem) return;

    if (auto p = node.GetProperty("Width")) {
        if (p->Is<double>()) elem->SetWidth(static_cast<float>(p->Get<double>().value()));
        else if (p->Is<int32_t>()) elem->SetWidth(static_cast<float>(p->Get<int32_t>().value()));
    }
    if (auto p = node.GetProperty("Height")) {
        if (p->Is<double>()) elem->SetHeight(static_cast<float>(p->Get<double>().value()));
        else if (p->Is<int32_t>()) elem->SetHeight(static_cast<float>(p->Get<int32_t>().value()));
    }
    if (auto p = node.GetProperty("Margin")) {
        if (p->Is<core::Thickness>()) elem->SetMargin(p->Get<core::Thickness>().value());
    }
    if (auto p = node.GetProperty("HorizontalAlignment")) {
        if (p->Is<layout::HorizontalAlignment>()) elem->SetHorizontalAlignment(p->Get<layout::HorizontalAlignment>().value());
    }
    if (auto p = node.GetProperty("VerticalAlignment")) {
        if (p->Is<layout::VerticalAlignment>()) elem->SetVerticalAlignment(p->Get<layout::VerticalAlignment>().value());
    }
    if (auto p = node.GetProperty("Visibility")) {
        if (p->Is<layout::Visibility>()) elem->SetVisibility(p->Get<layout::Visibility>().value());
    }
    if (auto p = node.GetProperty("IsEnabled")) {
        if (p->Is<bool>()) {
            if (auto ctrl = std::dynamic_pointer_cast<controls::Control>(elem)) {
                ctrl->SetEnabled(p->Get<bool>().value());
            }
        }
    }
    if (auto p = node.GetProperty("Opacity")) {
        if (p->Is<double>()) elem->SetOpacity(static_cast<float>(p->Get<double>().value()));
    }

    // 附加屬性 (Canvas.Left / Top, Grid.Row / Column)
    if (auto p = node.GetAttachedProperty("Canvas.Left")) {
        if (p->Is<double>()) layout::Canvas::SetLeft(elem, static_cast<float>(p->Get<double>().value()));
    }
    if (auto p = node.GetAttachedProperty("Canvas.Top")) {
        if (p->Is<double>()) layout::Canvas::SetTop(elem, static_cast<float>(p->Get<double>().value()));
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
                if (p->Is<core::Color>()) border->SetBackground(p->Get<core::Color>().value());
            }
            if (auto p = node.GetProperty("BorderBrush")) {
                if (p->Is<core::Color>()) border->SetBorderBrush(p->Get<core::Color>().value());
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
            if (auto p = node.GetProperty("Text")) {
                if (p->Is<std::string>()) text = p->Get<std::string>().value();
            }
            auto txt = std::make_shared<controls::TextBlock>(text);
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

    if (const auto* rootNode = doc.GetRoot()) {
        auto rootElement = CreateWidget(*rootNode, std::move(eventCallback));
        if (rootElement) {
            form->SetContent(rootElement);
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
