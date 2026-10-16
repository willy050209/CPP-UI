/**
 * @file FluentDslEmitter.cpp
 * @brief 現代 C++20 宣告式 Fluent RAD DSL 發射器實作
 */

#include "ui/designer/codegen/FluentDslEmitter.hpp"
#include "ui/designer/metadata/PropertyDescriptor.hpp"
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <map>

namespace cppui {
namespace designer {
namespace codegen {

namespace {

static std::string FormatFloat(double val) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.4g", val);
    std::string s = buf;
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos) {
        s += ".0";
    }
    s += "f";
    return s;
}

static std::string EscapeString(compat::string_view str) {
    std::string res;
    res.reserve(str.size() + 2);
    for (char c : str) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else res += c;
    }
    return res;
}

static std::string FormatPropertyValue(const PropertyValue& val) {
    if (val.Is<bool>()) {
        return val.Get<bool>().value() ? "true" : "false";
    }
    if (val.Is<int32_t>()) {
        return std::to_string(val.Get<int32_t>().value());
    }
    if (val.Is<int64_t>()) {
        return std::to_string(val.Get<int64_t>().value()) + "LL";
    }
    if (val.Is<double>()) {
        return FormatFloat(val.Get<double>().value());
    }
    if (val.Is<std::string>()) {
        return "\"" + EscapeString(val.Get<std::string>().value()) + "\"";
    }
    if (val.Is<core::Point>()) {
        auto pt = val.Get<core::Point>().value();
        return "cppui::core::Point(" + FormatFloat(pt.x) + ", " + FormatFloat(pt.y) + ")";
    }
    if (val.Is<core::Size>()) {
        auto sz = val.Get<core::Size>().value();
        return "cppui::core::Size(" + FormatFloat(sz.width) + ", " + FormatFloat(sz.height) + ")";
    }
    if (val.Is<core::Rect>()) {
        auto rc = val.Get<core::Rect>().value();
        return "cppui::core::Rect(" + FormatFloat(rc.x) + ", " + FormatFloat(rc.y) + ", " +
               FormatFloat(rc.width) + ", " + FormatFloat(rc.height) + ")";
    }
    if (val.Is<core::Thickness>()) {
        auto th = val.Get<core::Thickness>().value();
        if (th.left == th.top && th.top == th.right && th.right == th.bottom) {
            return FormatFloat(th.left);
        }
        if (th.left == th.right && th.top == th.bottom) {
            return "cppui::core::Thickness(" + FormatFloat(th.left) + ", " + FormatFloat(th.top) + ")";
        }
        return "cppui::core::Thickness(" + FormatFloat(th.left) + ", " + FormatFloat(th.top) + ", " +
               FormatFloat(th.right) + ", " + FormatFloat(th.bottom) + ")";
    }
    if (val.Is<layout::HorizontalAlignment>()) {
        switch (val.Get<layout::HorizontalAlignment>().value()) {
            case layout::HorizontalAlignment::Left:    return "cppui::layout::HorizontalAlignment::Left";
            case layout::HorizontalAlignment::Center:  return "cppui::layout::HorizontalAlignment::Center";
            case layout::HorizontalAlignment::Right:   return "cppui::layout::HorizontalAlignment::Right";
            case layout::HorizontalAlignment::Stretch: return "cppui::layout::HorizontalAlignment::Stretch";
        }
    }
    if (val.Is<layout::VerticalAlignment>()) {
        switch (val.Get<layout::VerticalAlignment>().value()) {
            case layout::VerticalAlignment::Top:     return "cppui::layout::VerticalAlignment::Top";
            case layout::VerticalAlignment::Center:  return "cppui::layout::VerticalAlignment::Center";
            case layout::VerticalAlignment::Bottom:  return "cppui::layout::VerticalAlignment::Bottom";
            case layout::VerticalAlignment::Stretch: return "cppui::layout::VerticalAlignment::Stretch";
        }
    }
    if (val.Is<layout::Orientation>()) {
        switch (val.Get<layout::Orientation>().value()) {
            case layout::Orientation::Horizontal: return "cppui::layout::Orientation::Horizontal";
            case layout::Orientation::Vertical:   return "cppui::layout::Orientation::Vertical";
        }
    }
    if (val.Is<layout::Visibility>()) {
        switch (val.Get<layout::Visibility>().value()) {
            case layout::Visibility::Visible:   return "cppui::layout::Visibility::Visible";
            case layout::Visibility::Hidden:    return "cppui::layout::Visibility::Hidden";
            case layout::Visibility::Collapsed: return "cppui::layout::Visibility::Collapsed";
        }
    }
    if (val.Is<core::Color>()) {
        auto c = val.Get<core::Color>().value();
        return "cppui::core::Color(" + std::to_string(c.r) + ", " + std::to_string(c.g) + ", " +
               std::to_string(c.b) + ", " + std::to_string(c.a) + ")";
    }
    if (val.Is<BrushDescriptor>()) {
        auto brush = val.Get<BrushDescriptor>().value();
        auto c = brush.color;
        return "cppui::core::Color(" + std::to_string(c.r) + ", " + std::to_string(c.g) + ", " +
               std::to_string(c.b) + ", " + std::to_string(c.a) + ")";
    }
    if (val.Is<ThemeTokenRef>()) {
        auto token = val.Get<ThemeTokenRef>().value();
        return "cppui::theme::Tokens::" + token.tokenName;
    }
    return "\"\"";
}

} // anonymous namespace

struct FluentDslEmitter::Impl {
    std::string EmitNodeInternal(const WidgetNode& node, const CodegenOptions& options, size_t indentLevel) const {
        std::ostringstream ss;
        std::string indent = options.GetIndent(indentLevel);

        // 檢查是否有特殊 factory 參數
        bool handledTextInFactory = false;
        bool handledOrientationInFactory = false;
        std::string factoryPrefix;

        NodeType type = node.GetType();

        if (type == NodeType::Button) {
            auto textProp = node.GetProperty("Text");
            if (textProp && textProp->Is<std::string>()) {
                factoryPrefix = "Button(\"" + EscapeString(textProp->Get<std::string>().value()) + "\")";
                handledTextInFactory = true;
            } else {
                factoryPrefix = "Button()";
            }
        } else if (type == NodeType::Label) {
            auto textProp = node.GetProperty("Text");
            if (textProp && textProp->Is<std::string>()) {
                factoryPrefix = "Label(\"" + EscapeString(textProp->Get<std::string>().value()) + "\")";
                handledTextInFactory = true;
            } else {
                factoryPrefix = "Label()";
            }
        } else if (type == NodeType::TextBlock) {
            auto textProp = node.GetProperty("Text");
            if (textProp && textProp->Is<std::string>()) {
                factoryPrefix = "TextBlock(\"" + EscapeString(textProp->Get<std::string>().value()) + "\")";
                handledTextInFactory = true;
            } else {
                factoryPrefix = "TextBlock()";
            }
        } else if (type == NodeType::CheckBox) {
            auto textProp = node.GetProperty("Text");
            if (textProp && textProp->Is<std::string>()) {
                factoryPrefix = "CheckBox(\"" + EscapeString(textProp->Get<std::string>().value()) + "\")";
                handledTextInFactory = true;
            } else {
                factoryPrefix = "CheckBox()";
            }
        } else if (type == NodeType::RadioButton) {
            auto textProp = node.GetProperty("Text");
            if (textProp && textProp->Is<std::string>()) {
                factoryPrefix = "RadioButton(\"" + EscapeString(textProp->Get<std::string>().value()) + "\")";
                handledTextInFactory = true;
            } else {
                factoryPrefix = "RadioButton()";
            }
        } else if (type == NodeType::TextBox) {
            auto phProp = node.GetProperty("Placeholder");
            if (phProp && phProp->Is<std::string>()) {
                factoryPrefix = "TextBox(\"" + EscapeString(phProp->Get<std::string>().value()) + "\")";
            } else {
                factoryPrefix = "TextBox()";
            }
        } else if (type == NodeType::StackPanel) {
            auto oriProp = node.GetProperty("Orientation");
            bool isHorizontal = false;
            if (oriProp && oriProp->Is<layout::Orientation>() &&
                oriProp->Get<layout::Orientation>().value() == layout::Orientation::Horizontal) {
                isHorizontal = true;
            }
            handledOrientationInFactory = true;

            std::string funcName = isHorizontal ? "HStack" : "VStack";
            if (node.GetChildCount() == 0) {
                factoryPrefix = funcName + "({})";
            } else {
                factoryPrefix = funcName + "({\n";
                for (size_t i = 0; i < node.GetChildCount(); ++i) {
                    const auto* child = node.GetChild(i);
                    factoryPrefix += EmitNodeInternal(*child, options, indentLevel + 1);
                    if (i + 1 < node.GetChildCount()) {
                        factoryPrefix += ",\n";
                    } else {
                        factoryPrefix += "\n";
                    }
                }
                factoryPrefix += indent + "})";
            }
        } else if (type == NodeType::Grid) {
            if (node.GetChildCount() == 0) {
                factoryPrefix = "Grid({})";
            } else {
                factoryPrefix = "Grid({\n";
                for (size_t i = 0; i < node.GetChildCount(); ++i) {
                    const auto* child = node.GetChild(i);
                    factoryPrefix += EmitNodeInternal(*child, options, indentLevel + 1);
                    if (i + 1 < node.GetChildCount()) {
                        factoryPrefix += ",\n";
                    } else {
                        factoryPrefix += "\n";
                    }
                }
                factoryPrefix += indent + "})";
            }
        } else if (type == NodeType::Border) {
            if (node.GetChildCount() == 1) {
                const auto* child = node.GetChild(0);
                factoryPrefix = "Border(\n" + EmitNodeInternal(*child, options, indentLevel + 1) + "\n" + indent + ")";
            } else {
                factoryPrefix = "Border()";
            }
        } else if (type == NodeType::ListBox) {
            factoryPrefix = "ListBox()";
        } else if (type == NodeType::ProgressBar) {
            factoryPrefix = "ProgressBar()";
        } else if (type == NodeType::Slider) {
            factoryPrefix = "Slider()";
        } else if (type == NodeType::Separator) {
            factoryPrefix = "Separator()";
        } else if (type == NodeType::PictureBox) {
            factoryPrefix = "PictureBox()";
        } else {
            factoryPrefix = "cppui::dsl::UIBuilder<" + GetWidgetCppType(type) + ">()";
        }

        ss << indent << factoryPrefix;

        // 收集並排序屬性
        std::map<std::string, PropertyValue> sortedProps;
        for (const auto& kv : node.GetAllProperties()) {
            sortedProps.insert(kv);
        }

        for (const auto& kv : sortedProps) {
            const std::string& propName = kv.first;
            const PropertyValue& val = kv.second;

            if (handledTextInFactory && propName == "Text") continue;
            if (handledOrientationInFactory && propName == "Orientation") continue;

            if (options.elideDefaults && PropertyMetadataRegistry::IsDefaultValue(type, propName, val)) {
                continue;
            }

            // 輸出屬性鏈式呼叫
            std::string setterName = propName;
            if (propName == "IsEnabled") setterName = "Enabled";
            if (propName == "IsChecked") setterName = "Checked";

            ss << "\n" << indent << options.GetIndent(1) << "." << setterName << "(" << FormatPropertyValue(val) << ")";
        }

        // 附加屬性 (Attached Properties)
        std::map<std::string, PropertyValue> sortedAttached;
        for (const auto& kv : node.GetAllAttachedProperties()) {
            sortedAttached.insert(kv);
        }

        for (const auto& kv : sortedAttached) {
            const std::string& attName = kv.first;
            const PropertyValue& val = kv.second;

            if (attName == "Grid.Row") {
                if (val.Is<int32_t>()) {
                    ss << "\n" << indent << options.GetIndent(1) << ".Row(" << val.Get<int32_t>().value() << ")";
                }
            } else if (attName == "Grid.Column") {
                if (val.Is<int32_t>()) {
                    ss << "\n" << indent << options.GetIndent(1) << ".Column(" << val.Get<int32_t>().value() << ")";
                }
            } else if (attName == "Grid.RowSpan") {
                if (val.Is<int32_t>()) {
                    ss << "\n" << indent << options.GetIndent(1) << ".RowSpan(" << val.Get<int32_t>().value() << ")";
                }
            } else if (attName == "Grid.ColumnSpan") {
                if (val.Is<int32_t>()) {
                    ss << "\n" << indent << options.GetIndent(1) << ".ColumnSpan(" << val.Get<int32_t>().value() << ")";
                }
            } else if (attName == "Canvas.Left") {
                if (val.Is<double>()) {
                    ss << "\n" << indent << options.GetIndent(1) << ".Left(" << FormatFloat(val.Get<double>().value()) << ")";
                }
            } else if (attName == "Canvas.Top") {
                if (val.Is<double>()) {
                    ss << "\n" << indent << options.GetIndent(1) << ".Top(" << FormatFloat(val.Get<double>().value()) << ")";
                }
            }
        }

        return ss.str();
    }
};

FluentDslEmitter::FluentDslEmitter() : m_impl(std::make_unique<Impl>()) {}
FluentDslEmitter::~FluentDslEmitter() = default;
FluentDslEmitter::FluentDslEmitter(FluentDslEmitter&& other) noexcept = default;
FluentDslEmitter& FluentDslEmitter::operator=(FluentDslEmitter&& other) noexcept = default;

std::string FluentDslEmitter::EmitNode(
    const WidgetNode& node,
    const CodegenOptions& options,
    size_t indentLevel
) const {
    return m_impl->EmitNodeInternal(node, options, indentLevel);
}

std::string FluentDslEmitter::EmitDocument(
    const FormDocumentNode& document,
    const CodegenOptions& options
) const {
    const WidgetNode* root = document.GetRoot();
    if (!root) {
        return "// Empty Document\n";
    }

    std::ostringstream ss;
    ss << "/**\n"
       << " * Automatically generated by CPPUI Visual Form Designer.\n"
       << " */\n\n"
       << "#include \"ui/dsl/Fluent.hpp\"\n\n"
       << "using namespace cppui;\n"
       << "using namespace cppui::dsl;\n\n";

    if (!options.rootNamespace.empty()) {
        ss << "namespace " << options.rootNamespace << " {\n\n";
    }

    ss << "inline auto BuildFormUI() {\n"
       << "    return " << m_impl->EmitNodeInternal(*root, options, 1).substr(options.GetIndent(1).size()) << ";\n"
       << "}\n\n";

    if (!options.rootNamespace.empty()) {
        ss << "} // namespace " << options.rootNamespace << "\n";
    }

    return ss.str();
}

} // namespace codegen
} // namespace designer
} // namespace cppui
