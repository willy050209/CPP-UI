/**
 * @file InheritanceMetadataRegistry.cpp
 * @brief 靜態繼承鏈屬性與事件元資料註冊表實作
 */

#include "ui/designer/inspector/InheritanceMetadataRegistry.hpp"
#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace cppui {
namespace designer {
namespace inspector {

namespace {

// 繼承層級之屬性定義
std::vector<PropertyDescriptor> CreateVisualProperties() {
    return {
        { "Opacity", PropertyValue(1.0), PropertyCategory::Appearance, false }
    };
}

std::vector<PropertyDescriptor> CreateUIElementProperties() {
    std::vector<PropertyDescriptor> list = CreateVisualProperties();
    list.push_back({ "Width", PropertyValue(0.0), PropertyCategory::Layout, false });
    list.push_back({ "Height", PropertyValue(0.0), PropertyCategory::Layout, false });
    list.push_back({ "MinWidth", PropertyValue(0.0), PropertyCategory::Layout, false });
    list.push_back({ "MinHeight", PropertyValue(0.0), PropertyCategory::Layout, false });
    list.push_back({ "MaxWidth", PropertyValue(10000.0), PropertyCategory::Layout, false });
    list.push_back({ "MaxHeight", PropertyValue(10000.0), PropertyCategory::Layout, false });
    list.push_back({ "Margin", PropertyValue(core::Thickness(0.0f, 0.0f, 0.0f, 0.0f)), PropertyCategory::Layout, false });
    list.push_back({ "HorizontalAlignment", PropertyValue(layout::HorizontalAlignment::Stretch), PropertyCategory::Layout, false });
    list.push_back({ "VerticalAlignment", PropertyValue(layout::VerticalAlignment::Stretch), PropertyCategory::Layout, false });
    list.push_back({ "Visibility", PropertyValue(layout::Visibility::Visible), PropertyCategory::Common, false });
    list.push_back({ "IsEnabled", PropertyValue(true), PropertyCategory::Common, false });
    return list;
}

std::vector<PropertyDescriptor> CreateControlProperties() {
    std::vector<PropertyDescriptor> list = CreateUIElementProperties();
    list.push_back({ "Background", PropertyValue(core::Color{ 0, 0, 0, 0 }), PropertyCategory::Appearance, false });
    list.push_back({ "Foreground", PropertyValue(core::Color{ 0, 0, 0, 255 }), PropertyCategory::Appearance, false });
    list.push_back({ "Padding", PropertyValue(core::Thickness(0.0f, 0.0f, 0.0f, 0.0f)), PropertyCategory::Layout, false });
    list.push_back({ "FontFamily", PropertyValue(std::string("Segoe UI")), PropertyCategory::Text, false });
    list.push_back({ "FontSize", PropertyValue(14.0), PropertyCategory::Text, false });
    list.push_back({ "FontWeight", PropertyValue(int32_t(400)), PropertyCategory::Text, false });
    list.push_back({ "BorderBrush", PropertyValue(core::Color{ 0, 0, 0, 255 }), PropertyCategory::Appearance, false });
    list.push_back({ "BorderThickness", PropertyValue(core::Thickness(1.0f, 1.0f, 1.0f, 1.0f)), PropertyCategory::Appearance, false });
    list.push_back({ "AutoSize", PropertyValue(false), PropertyCategory::Layout, false });
    return list;
}

std::vector<PropertyDescriptor> CreateContentControlProperties() {
    std::vector<PropertyDescriptor> list = CreateControlProperties();
    list.push_back({ "Content", PropertyValue(std::string("")), PropertyCategory::Common, false });
    return list;
}

std::vector<PropertyDescriptor> BuildDescriptors(NodeType type) {
    switch (type) {
        case NodeType::Button: {
            return CreateContentControlProperties();
        }
        case NodeType::CheckBox: {
            auto list = CreateContentControlProperties();
            list.push_back({ "IsChecked", PropertyValue(false), PropertyCategory::Behavior, false });
            return list;
        }
        case NodeType::RadioButton: {
            auto list = CreateContentControlProperties();
            list.push_back({ "IsChecked", PropertyValue(false), PropertyCategory::Behavior, false });
            list.push_back({ "GroupName", PropertyValue(std::string("")), PropertyCategory::Behavior, false });
            return list;
        }
        case NodeType::TextBox: {
            auto list = CreateControlProperties();
            list.push_back({ "Text", PropertyValue(std::string("")), PropertyCategory::Text, false });
            list.push_back({ "Placeholder", PropertyValue(std::string("")), PropertyCategory::Text, false });
            list.push_back({ "IsReadOnly", PropertyValue(false), PropertyCategory::Behavior, false });
            list.push_back({ "MaxLength", PropertyValue(int32_t(0)), PropertyCategory::Behavior, false });
            return list;
        }
        case NodeType::TextBlock: {
            auto list = CreateUIElementProperties();
            list.push_back({ "Text", PropertyValue(std::string("")), PropertyCategory::Text, false });
            list.push_back({ "FontFamily", PropertyValue(std::string("Segoe UI")), PropertyCategory::Text, false });
            list.push_back({ "FontSize", PropertyValue(14.0), PropertyCategory::Text, false });
            list.push_back({ "FontWeight", PropertyValue(int32_t(400)), PropertyCategory::Text, false });
            list.push_back({ "Foreground", PropertyValue(core::Color{ 0, 0, 0, 255 }), PropertyCategory::Appearance, false });
            list.push_back({ "TextWrapping", PropertyValue(false), PropertyCategory::Text, false });
            return list;
        }
        case NodeType::Slider: {
            auto list = CreateControlProperties();
            list.push_back({ "Minimum", PropertyValue(0.0), PropertyCategory::Behavior, false });
            list.push_back({ "Maximum", PropertyValue(100.0), PropertyCategory::Behavior, false });
            list.push_back({ "Value", PropertyValue(0.0), PropertyCategory::Behavior, false });
            list.push_back({ "Orientation", PropertyValue(layout::Orientation::Horizontal), PropertyCategory::Layout, false });
            return list;
        }
        case NodeType::ProgressBar: {
            auto list = CreateControlProperties();
            list.push_back({ "Minimum", PropertyValue(0.0), PropertyCategory::Behavior, false });
            list.push_back({ "Maximum", PropertyValue(100.0), PropertyCategory::Behavior, false });
            list.push_back({ "Value", PropertyValue(0.0), PropertyCategory::Behavior, false });
            list.push_back({ "Orientation", PropertyValue(layout::Orientation::Horizontal), PropertyCategory::Layout, false });
            return list;
        }
        case NodeType::Border: {
            auto list = CreateUIElementProperties();
            list.push_back({ "Background", PropertyValue(core::Color{ 0, 0, 0, 0 }), PropertyCategory::Appearance, false });
            list.push_back({ "BorderBrush", PropertyValue(core::Color{ 0, 0, 0, 255 }), PropertyCategory::Appearance, false });
            list.push_back({ "BorderThickness", PropertyValue(core::Thickness(1.0f, 1.0f, 1.0f, 1.0f)), PropertyCategory::Appearance, false });
            list.push_back({ "CornerRadius", PropertyValue(0.0), PropertyCategory::Appearance, false });
            list.push_back({ "Padding", PropertyValue(core::Thickness(0.0f, 0.0f, 0.0f, 0.0f)), PropertyCategory::Layout, false });
            return list;
        }
        case NodeType::StackPanel: {
            auto list = CreateUIElementProperties();
            list.push_back({ "Orientation", PropertyValue(layout::Orientation::Vertical), PropertyCategory::Layout, false });
            list.push_back({ "Spacing", PropertyValue(0.0), PropertyCategory::Layout, false });
            return list;
        }
        case NodeType::VirtualizingStackPanel: {
            auto list = CreateUIElementProperties();
            list.push_back({ "Orientation", PropertyValue(layout::Orientation::Vertical), PropertyCategory::Layout, false });
            list.push_back({ "Spacing", PropertyValue(0.0), PropertyCategory::Layout, false });
            return list;
        }
        case NodeType::Canvas: {
            return CreateUIElementProperties();
        }
        case NodeType::Grid: {
            auto list = CreateUIElementProperties();
            list.push_back({ "ShowGridLines", PropertyValue(false), PropertyCategory::Appearance, false });
            return list;
        }
        case NodeType::ContentControl:
        case NodeType::ScrollViewer:
        case NodeType::Label:
        case NodeType::MenuItem: {
            return CreateContentControlProperties();
        }
        case NodeType::Popup: {
            auto list = CreateUIElementProperties();
            list.push_back({ "IsOpen", PropertyValue(false), PropertyCategory::Behavior, false });
            return list;
        }
        case NodeType::ComboBox:
        case NodeType::ListBox: {
            auto list = CreateControlProperties();
            list.push_back({ "SelectedIndex", PropertyValue(int32_t(-1)), PropertyCategory::Behavior, false });
            return list;
        }
        case NodeType::DataGrid: {
            auto list = CreateControlProperties();
            list.push_back({ "CanUserSort", PropertyValue(true), PropertyCategory::Behavior, false });
            list.push_back({ "CanUserResize", PropertyValue(true), PropertyCategory::Behavior, false });
            return list;
        }
        case NodeType::PictureBox: {
            auto list = CreateControlProperties();
            list.push_back({ "Source", PropertyValue(std::string("")), PropertyCategory::Appearance, false });
            return list;
        }
        case NodeType::Separator: {
            auto list = CreateControlProperties();
            list.push_back({ "Orientation", PropertyValue(layout::Orientation::Horizontal), PropertyCategory::Layout, false });
            return list;
        }
        case NodeType::TreeView:
        case NodeType::ContextMenu: {
            return CreateControlProperties();
        }
        case NodeType::Timer: {
            return {
                { "IntervalMs", PropertyValue(int32_t(1000)), PropertyCategory::Behavior, false },
                { "IsEnabled", PropertyValue(true), PropertyCategory::Behavior, false }
            };
        }
        case NodeType::OpenFileDialog:
        case NodeType::SaveFileDialog: {
            return {
                { "Title", PropertyValue(std::string("Select File")), PropertyCategory::Common, false },
                { "Filter", PropertyValue(std::string("All Files (*.*)|*.*")), PropertyCategory::Common, false },
                { "InitialDirectory", PropertyValue(std::string("")), PropertyCategory::Common, false },
                { "DefaultExtension", PropertyValue(std::string("")), PropertyCategory::Common, false }
            };
        }
        case NodeType::MessageBox: {
            return {
                { "Caption", PropertyValue(std::string("Message")), PropertyCategory::Common, false },
                { "Text", PropertyValue(std::string("")), PropertyCategory::Common, false }
            };
        }
        case NodeType::FormDocument: {
            return {
                { "ClassName", PropertyValue(std::string("MainForm")), PropertyCategory::Common, false },
                { "TargetNamespace", PropertyValue(std::string("MyApp")), PropertyCategory::Common, false },
                { "Title", PropertyValue(std::string("Form")), PropertyCategory::Common, false },
                { "Width", PropertyValue(800.0), PropertyCategory::Layout, false },
                { "Height", PropertyValue(600.0), PropertyCategory::Layout, false },
                { "StartPosition", PropertyValue(std::string("CenterScreen")), PropertyCategory::Layout, false },
                { "WindowState", PropertyValue(std::string("Normal")), PropertyCategory::Layout, false },
                { "Background", PropertyValue(core::Color(28, 30, 36)), PropertyCategory::Appearance, false },
                { "Padding", PropertyValue(core::Thickness(16.0f)), PropertyCategory::Layout, false },
                { "DefaultTheme", PropertyValue(std::string("Dark")), PropertyCategory::Appearance, false },
                { "UseMvvm", PropertyValue(true), PropertyCategory::Behavior, false },
                { "UseIoC", PropertyValue(true), PropertyCategory::Behavior, false },
                { "Topmost", PropertyValue(false), PropertyCategory::Behavior, false },
                { "ShowInTaskbar", PropertyValue(true), PropertyCategory::Behavior, false },
                { "AutoSize", PropertyValue(false), PropertyCategory::Layout, false },
                { "AutoSizeMode", PropertyValue(std::string("GrowAndShrink")), PropertyCategory::Layout, false }
            };
        }
    }
    return CreateUIElementProperties();
}

std::vector<EventDescriptor> CreateUIElementEvents() {
    return {
        { "PointerPressed", "PointerPressed (指標按下)", "Mouse", "void(PointerEventArgs&)", "OnPointerPressed", false },
        { "PointerReleased", "PointerReleased (指標釋放)", "Mouse", "void(PointerEventArgs&)", "OnPointerReleased", false },
        { "PointerMoved", "PointerMoved (指標移動)", "Mouse", "void(PointerEventArgs&)", "OnPointerMoved", false },
        { "KeyDown", "KeyDown (鍵盤按下)", "Keyboard", "void(KeyEventArgs&)", "OnKeyDown", false },
        { "KeyUp", "KeyUp (鍵盤釋放)", "Keyboard", "void(KeyEventArgs&)", "OnKeyUp", false },
        { "GotFocus", "GotFocus (獲得焦點)", "Focus", "void(FocusEventArgs&)", "OnGotFocus", false },
        { "LostFocus", "LostFocus (失去焦點)", "Focus", "void(FocusEventArgs&)", "OnLostFocus", false }
    };
}

std::vector<EventDescriptor> BuildEvents(NodeType type) {
    if (type == NodeType::FormDocument) {
        return {
            { "Load", "Load (表單載入初始化)", "Action", "void(EventArgs&)", "OnLoad", false },
            { "Shown", "Shown (表單初次顯示)", "Action", "void(EventArgs&)", "OnShown", false },
            { "Closing", "Closing (表單即將關閉)", "Action", "void(FormClosingEventArgs&)", "OnClosing", false },
            { "Closed", "Closed (表單已關閉)", "Action", "void(FormClosedEventArgs&)", "OnClosed", false },
            { "Activated", "Activated (表單獲得焦點作用中)", "Focus", "void(EventArgs&)", "OnActivated", false },
            { "Deactivated", "Deactivated (表單失去焦點)", "Focus", "void(EventArgs&)", "OnDeactivate", false },
            { "Resize", "Resize (表單尺寸變更)", "Layout", "void(EventArgs&)", "OnResize", false },
            { "KeyDown", "KeyDown (視窗鍵盤按下)", "Keyboard", "void(KeyEventArgs&)", "OnKeyDown", false },
            { "KeyUp", "KeyUp (視窗鍵盤釋放)", "Keyboard", "void(KeyEventArgs&)", "OnKeyUp", false }
        };
    }
    if (type == NodeType::Timer) {
        return {
            { "Tick", "Tick (計時器觸發)", "Behavior", "void(EventArgs&)", "OnTick", false }
        };
    }
    if (type == NodeType::OpenFileDialog || type == NodeType::SaveFileDialog) {
        return {
            { "FileOk", "FileOk (檔案選取確認)", "Behavior", "void(CancelEventArgs&)", "OnFileOk", false }
        };
    }
    if (type == NodeType::MessageBox) {
        return {};
    }

    std::vector<EventDescriptor> list = CreateUIElementEvents();

    switch (type) {
        case NodeType::Button:
        case NodeType::MenuItem:
            list.insert(list.begin(), { "Click", "Click (點擊事件)", "Action", "void(RoutedEventArgs&)", "OnClick", false });
            list.insert(list.begin() + 1, { "Command", "Command (MVVM 命令綁定)", "Action", "ICommand", "Command", true });
            break;
        case NodeType::CheckBox:
        case NodeType::RadioButton:
            list.insert(list.begin(), { "Click", "Click (點擊事件)", "Action", "void(RoutedEventArgs&)", "OnClick", false });
            list.insert(list.begin() + 1, { "Checked", "Checked (選中狀態)", "Behavior", "void(RoutedEventArgs&)", "OnChecked", false });
            list.insert(list.begin() + 2, { "Unchecked", "Unchecked (取消選中)", "Behavior", "void(RoutedEventArgs&)", "OnUnchecked", false });
            break;
        case NodeType::TextBox:
            list.insert(list.begin(), { "TextChanged", "TextChanged (文字變更)", "Text", "void(RoutedEventArgs&)", "OnTextChanged", false });
            break;
        case NodeType::Slider:
            list.insert(list.begin(), { "ValueChanged", "ValueChanged (數值變更)", "Behavior", "void(RoutedEventArgs&)", "OnValueChanged", false });
            break;
        case NodeType::ComboBox:
        case NodeType::ListBox:
            list.insert(list.begin(), { "SelectionChanged", "SelectionChanged (選取項目變更)", "Behavior", "void(RoutedEventArgs&)", "OnSelectionChanged", false });
            break;
        case NodeType::DataGrid:
            list.insert(list.begin(), { "SelectionChanged", "SelectionChanged (選取項目變更)", "Behavior", "void(RoutedEventArgs&)", "OnSelectionChanged", false });
            list.insert(list.begin() + 1, { "CellEditEnded", "CellEditEnded (單元格編輯完成)", "Behavior", "void(RoutedEventArgs&)", "OnCellEditEnded", false });
            break;
        case NodeType::TreeView:
            list.insert(list.begin(), { "SelectedItemChanged", "SelectedItemChanged (節點選取變更)", "Behavior", "void(RoutedEventArgs&)", "OnSelectedItemChanged", false });
            break;
        default:
            break;
    }

    return list;
}

const std::unordered_map<NodeType, std::vector<PropertyDescriptor>>& GetPropertyMap() {
    static const auto s_map = []() {
        std::unordered_map<NodeType, std::vector<PropertyDescriptor>> map;
        const NodeType types[] = {
            NodeType::FormDocument, NodeType::Canvas, NodeType::StackPanel, NodeType::VirtualizingStackPanel,
            NodeType::Grid, NodeType::Border, NodeType::ContentControl, NodeType::ScrollViewer, NodeType::Popup,
            NodeType::TreeView, NodeType::ContextMenu, NodeType::MenuItem, NodeType::Button, NodeType::CheckBox,
            NodeType::ComboBox, NodeType::DataGrid, NodeType::Label, NodeType::ListBox, NodeType::PictureBox,
            NodeType::ProgressBar, NodeType::RadioButton, NodeType::Separator, NodeType::Slider, NodeType::TextBlock,
            NodeType::TextBox, NodeType::Timer, NodeType::OpenFileDialog, NodeType::SaveFileDialog, NodeType::MessageBox
        };
        for (NodeType t : types) {
            map[t] = BuildDescriptors(t);
        }
        return map;
    }();
    return s_map;
}

const std::unordered_map<NodeType, std::vector<EventDescriptor>>& GetEventMap() {
    static const auto s_map = []() {
        std::unordered_map<NodeType, std::vector<EventDescriptor>> map;
        const NodeType types[] = {
            NodeType::FormDocument, NodeType::Canvas, NodeType::StackPanel, NodeType::VirtualizingStackPanel,
            NodeType::Grid, NodeType::Border, NodeType::ContentControl, NodeType::ScrollViewer, NodeType::Popup,
            NodeType::TreeView, NodeType::ContextMenu, NodeType::MenuItem, NodeType::Button, NodeType::CheckBox,
            NodeType::ComboBox, NodeType::DataGrid, NodeType::Label, NodeType::ListBox, NodeType::PictureBox,
            NodeType::ProgressBar, NodeType::RadioButton, NodeType::Separator, NodeType::Slider, NodeType::TextBlock,
            NodeType::TextBox, NodeType::Timer, NodeType::OpenFileDialog, NodeType::SaveFileDialog, NodeType::MessageBox
        };
        for (NodeType t : types) {
            map[t] = BuildEvents(t);
        }
        return map;
    }();
    return s_map;
}

} // anonymous namespace

const std::vector<PropertyDescriptor>& InheritanceMetadataRegistry::GetDescriptorsForType(NodeType type) {
    const auto& map = GetPropertyMap();
    auto it = map.find(type);
    if (it != map.end()) {
        return it->second;
    }
    static const std::vector<PropertyDescriptor> s_empty;
    return s_empty;
}

const std::vector<EventDescriptor>& InheritanceMetadataRegistry::GetEventDescriptorsForType(NodeType type) {
    const auto& map = GetEventMap();
    auto it = map.find(type);
    if (it != map.end()) {
        return it->second;
    }
    static const std::vector<EventDescriptor> s_empty;
    return s_empty;
}

std::vector<PropertyDescriptor> InheritanceMetadataRegistry::ComputePropertyIntersection(
    const std::vector<NodeType>& types
) {
    if (types.empty()) {
        return {};
    }

    std::vector<PropertyDescriptor> intersection = GetDescriptorsForType(types[0]);

    for (size_t i = 1; i < types.size(); ++i) {
        const auto& currentList = GetDescriptorsForType(types[i]);
        std::vector<PropertyDescriptor> nextIntersection;

        for (const auto& item : intersection) {
            bool found = false;
            for (const auto& cur : currentList) {
                if (cur.name == item.name) {
                    found = true;
                    break;
                }
            }
            if (found) {
                nextIntersection.push_back(item);
            }
        }
        intersection = std::move(nextIntersection);
        if (intersection.empty()) {
            break;
        }
    }

    return intersection;
}

std::vector<EventDescriptor> InheritanceMetadataRegistry::ComputeEventIntersection(
    const std::vector<NodeType>& types
) {
    if (types.empty()) {
        return {};
    }

    std::vector<EventDescriptor> intersection = GetEventDescriptorsForType(types[0]);

    for (size_t i = 1; i < types.size(); ++i) {
        const auto& currentList = GetEventDescriptorsForType(types[i]);
        std::vector<EventDescriptor> nextIntersection;

        for (const auto& item : intersection) {
            bool found = false;
            for (const auto& cur : currentList) {
                if (cur.eventName == item.eventName) {
                    found = true;
                    break;
                }
            }
            if (found) {
                nextIntersection.push_back(item);
            }
        }
        intersection = std::move(nextIntersection);
        if (intersection.empty()) {
            break;
        }
    }

    return intersection;
}

std::vector<PropertyDescriptor> InheritanceMetadataRegistry::GetAttachedPropertiesForParent(
    compat::optional<NodeType> parentType
) {
    std::vector<PropertyDescriptor> list;

    if (parentType.has_value()) {
        if (*parentType == NodeType::Canvas) {
            list.push_back({ "Canvas.Left", PropertyValue(0.0), PropertyCategory::Layout, false });
            list.push_back({ "Canvas.Top", PropertyValue(0.0), PropertyCategory::Layout, false });
        } else if (*parentType == NodeType::Grid) {
            list.push_back({ "Grid.Row", PropertyValue(int32_t(0)), PropertyCategory::Layout, false });
            list.push_back({ "Grid.Column", PropertyValue(int32_t(0)), PropertyCategory::Layout, false });
            list.push_back({ "Grid.RowSpan", PropertyValue(int32_t(1)), PropertyCategory::Layout, false });
            list.push_back({ "Grid.ColumnSpan", PropertyValue(int32_t(1)), PropertyCategory::Layout, false });
        }
    }

    list.push_back({ "ToolTip.Text", PropertyValue(std::string("")), PropertyCategory::Common, false });
    return list;
}

std::string InheritanceMetadataRegistry::GenerateDefaultHandlerName(
    compat::string_view nodeName,
    compat::string_view eventName
) {
    std::string baseName;
    if (!nodeName.empty()) {
        baseName = std::string(nodeName);
        if (std::islower(static_cast<unsigned char>(baseName[0]))) {
            baseName[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(baseName[0])));
        }
    }

    if (baseName.empty()) {
        return "On" + std::string(eventName);
    }

    return "On" + baseName + std::string(eventName);
}

std::string InheritanceMetadataRegistry::GetFriendlyTypeName(const PropertyValue& val) {
    if (val.Is<bool>()) return "bool";
    if (val.Is<int32_t>() || val.Is<int64_t>()) return "integer";
    if (val.Is<double>()) return "double";
    if (val.Is<std::string>()) return "string";
    if (val.Is<core::Thickness>()) return "Thickness";
    if (val.Is<core::Color>()) return "Color";
    if (val.Is<BrushDescriptor>()) return "Brush";
    if (val.Is<layout::HorizontalAlignment>()) return "HorizontalAlignment";
    if (val.Is<layout::VerticalAlignment>()) return "VerticalAlignment";
    if (val.Is<layout::Orientation>()) return "Orientation";
    if (val.Is<layout::Visibility>()) return "Visibility";
    if (val.Is<ThemeTokenRef>()) return "ThemeToken";
    return std::string(val.GetTypeName());
}

std::vector<std::string> InheritanceMetadataRegistry::GetEnumOptions(compat::string_view propName) {
    if (propName == "FontWeight") {
        return { "Normal", "Bold", "SemiBold", "Light", "Thin", "ExtraBold", "Black" };
    }
    if (propName == "FontFamily") {
        return { "Segoe UI", "Arial", "Consolas", "Microsoft JhengHei", "Tahoma", "Times New Roman" };
    }
    if (propName == "HorizontalAlignment") {
        return { "Left", "Center", "Right", "Stretch" };
    }
    if (propName == "VerticalAlignment") {
        return { "Top", "Center", "Bottom", "Stretch" };
    }
    if (propName == "Orientation") {
        return { "Horizontal", "Vertical" };
    }
    if (propName == "Visibility") {
        return { "Visible", "Hidden", "Collapsed" };
    }
    if (propName == "StartPosition") {
        return { "CenterScreen", "Manual", "WindowsDefaultLocation" };
    }
    if (propName == "WindowState") {
        return { "Normal", "Maximized", "Minimized" };
    }
    if (propName == "DefaultTheme") {
        return { "Dark", "Light" };
    }
    if (propName == "AutoSizeMode") {
        return { "GrowAndShrink", "GrowOnly" };
    }
    return {};
}

std::vector<std::string> InheritanceMetadataRegistry::GetThemeTokenSuggestions(compat::string_view propName) {
    if (propName == "Background" || propName == "Foreground" || propName == "BorderBrush") {
        return {
            "accentPrimary",
            "accentHover",
            "accentPressed",
            "controlBackground",
            "controlBackgroundHover",
            "surfaceDefault",
            "textPrimary",
            "textSecondary",
            "borderDefault",
            "focusBorder"
        };
    }
    if (propName == "Margin" || propName == "Padding" || propName == "Spacing" ||
        propName == "CornerRadius" || propName == "BorderThickness") {
        return {
            "spacingXs",
            "spacingSm",
            "spacingMd",
            "spacingLg",
            "spacingXl",
            "cornerRadiusSm",
            "cornerRadiusMd",
            "cornerRadiusLg",
            "borderThicknessThin",
            "borderThicknessMedium"
        };
    }
    if (propName == "FontFamily" || propName == "FontSize") {
        return {
            "fontFamilyDefault",
            "fontFamilyMonospace",
            "fontSizeCaption",
            "fontSizeBody",
            "fontSizeSubheading",
            "fontSizeHeading",
            "fontSizeTitle"
        };
    }
    return {};
}

} // namespace inspector
} // namespace designer
} // namespace cppui
