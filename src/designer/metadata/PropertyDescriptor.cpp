/**
 * @file PropertyDescriptor.cpp
 * @brief PropertyMetadataRegistry 註冊表實作
 */

#include "ui/designer/metadata/PropertyDescriptor.hpp"
#include <unordered_map>

namespace cppui {
namespace designer {

namespace {

std::vector<PropertyDescriptor> CreateCommonDescriptors() {
    return {
        { "Width", PropertyValue(0.0), PropertyCategory::Layout, false },
        { "Height", PropertyValue(0.0), PropertyCategory::Layout, false },
        { "Margin", PropertyValue(core::Thickness(0.0f, 0.0f, 0.0f, 0.0f)), PropertyCategory::Layout, false },
        { "HorizontalAlignment", PropertyValue(layout::HorizontalAlignment::Stretch), PropertyCategory::Layout, false },
        { "VerticalAlignment", PropertyValue(layout::VerticalAlignment::Stretch), PropertyCategory::Layout, false },
        { "Visibility", PropertyValue(layout::Visibility::Visible), PropertyCategory::Common, false },
        { "IsEnabled", PropertyValue(true), PropertyCategory::Common, false },
        { "AutoSize", PropertyValue(false), PropertyCategory::Layout, false }
    };
}

std::vector<PropertyDescriptor> BuildDescriptorsForType(NodeType type) {
    std::vector<PropertyDescriptor> list = CreateCommonDescriptors();

    switch (type) {
        case NodeType::Button:
            list.push_back({ "Content", PropertyValue(std::string("")), PropertyCategory::Common, false });
            list.push_back({ "FontSize", PropertyValue(14.0), PropertyCategory::Text, false });
            break;

        case NodeType::TextBlock:
            list.push_back({ "Text", PropertyValue(std::string("")), PropertyCategory::Text, false });
            list.push_back({ "FontSize", PropertyValue(14.0), PropertyCategory::Text, false });
            list.push_back({ "FontFamily", PropertyValue(std::string("Segoe UI")), PropertyCategory::Text, false });
            break;

        case NodeType::TextBox:
            list.push_back({ "Text", PropertyValue(std::string("")), PropertyCategory::Text, false });
            list.push_back({ "Placeholder", PropertyValue(std::string("")), PropertyCategory::Text, false });
            list.push_back({ "FontSize", PropertyValue(14.0), PropertyCategory::Text, false });
            list.push_back({ "IsReadOnly", PropertyValue(false), PropertyCategory::Behavior, false });
            break;

        case NodeType::CheckBox:
        case NodeType::RadioButton:
            list.push_back({ "Content", PropertyValue(std::string("")), PropertyCategory::Common, false });
            list.push_back({ "IsChecked", PropertyValue(false), PropertyCategory::Behavior, false });
            if (type == NodeType::RadioButton) {
                list.push_back({ "GroupName", PropertyValue(std::string("")), PropertyCategory::Behavior, false });
            }
            break;

        case NodeType::Slider:
        case NodeType::ProgressBar:
            list.push_back({ "Minimum", PropertyValue(0.0), PropertyCategory::Behavior, false });
            list.push_back({ "Maximum", PropertyValue(100.0), PropertyCategory::Behavior, false });
            list.push_back({ "Value", PropertyValue(0.0), PropertyCategory::Behavior, false });
            break;

        case NodeType::Border:
            list.push_back({ "BorderThickness", PropertyValue(core::Thickness(1.0f, 1.0f, 1.0f, 1.0f)), PropertyCategory::Appearance, false });
            list.push_back({ "CornerRadius", PropertyValue(0.0), PropertyCategory::Appearance, false });
            list.push_back({ "Padding", PropertyValue(core::Thickness(0.0f, 0.0f, 0.0f, 0.0f)), PropertyCategory::Layout, false });
            break;

        case NodeType::StackPanel:
            list.push_back({ "Orientation", PropertyValue(layout::Orientation::Vertical), PropertyCategory::Layout, false });
            list.push_back({ "Spacing", PropertyValue(0.0), PropertyCategory::Layout, false });
            break;

        default:
            break;
    }

    return list;
}

const std::unordered_map<NodeType, std::vector<PropertyDescriptor>>& GetGlobalRegistry() {
    static const auto registry = []() {
        std::unordered_map<NodeType, std::vector<PropertyDescriptor>> map;
        const NodeType allTypes[] = {
            NodeType::FormDocument, NodeType::Canvas, NodeType::StackPanel, NodeType::VirtualizingStackPanel,
            NodeType::Grid, NodeType::Border, NodeType::ContentControl, NodeType::ScrollViewer, NodeType::Popup,
            NodeType::TreeView, NodeType::ContextMenu, NodeType::MenuItem, NodeType::Button, NodeType::CheckBox,
            NodeType::ComboBox, NodeType::DataGrid, NodeType::Label, NodeType::ListBox, NodeType::PictureBox,
            NodeType::ProgressBar, NodeType::RadioButton, NodeType::Separator, NodeType::Slider, NodeType::TextBlock,
            NodeType::TextBox
        };
        for (NodeType t : allTypes) {
            map[t] = BuildDescriptorsForType(t);
        }
        return map;
    }();
    return registry;
}

} // anonymous namespace

const std::vector<PropertyDescriptor>& PropertyMetadataRegistry::GetDescriptors(NodeType type) {
    const auto& reg = GetGlobalRegistry();
    auto it = reg.find(type);
    if (it != reg.end()) {
        return it->second;
    }
    static const std::vector<PropertyDescriptor> s_empty;
    return s_empty;
}

compat::optional<PropertyValue> PropertyMetadataRegistry::GetDefaultValue(NodeType type, compat::string_view propName) {
    const auto& descriptors = GetDescriptors(type);
    for (const auto& desc : descriptors) {
        if (desc.name == propName) {
            return desc.defaultValue;
        }
    }
    return compat::nullopt;
}

bool PropertyMetadataRegistry::IsDefaultValue(NodeType type, compat::string_view propName, const PropertyValue& value) {
    auto defVal = GetDefaultValue(type, propName);
    if (!defVal.has_value()) {
        return false;
    }
    return *defVal == value;
}

} // namespace designer
} // namespace cppui
