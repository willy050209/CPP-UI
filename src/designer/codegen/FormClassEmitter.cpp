/**
 * @file FormClassEmitter.cpp
 * @brief 表單 Partial Class / Code-Behind 發射器實作
 */

#include "ui/designer/codegen/FormClassEmitter.hpp"
#include "ui/designer/metadata/PropertyDescriptor.hpp"
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <cstdio>

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

struct NodeCollectInfo {
    const WidgetNode* node = nullptr;
    std::string varName;
    std::string memberName;
    bool isMember = false;
};

static void CollectNodes(
    const WidgetNode* node,
    size_t& autoIndex,
    const CodegenOptions& options,
    std::vector<NodeCollectInfo>& outList
) {
    if (!node) return;

    NodeCollectInfo info;
    info.node = node;
    std::string rawName = node->GetName();

    if (!rawName.empty()) {
        info.isMember = true;
        info.memberName = FormatIdentifier(rawName, options.memberNaming);
        info.varName = info.memberName;
    } else {
        info.isMember = false;
        info.varName = "elem" + std::to_string(++autoIndex);
    }

    outList.push_back(info);

    for (size_t i = 0; i < node->GetChildCount(); ++i) {
        CollectNodes(node->GetChild(i), autoIndex, options, outList);
    }
}

} // anonymous namespace

struct FormClassEmitter::Impl {
    std::string EmitDesignerHeaderInternal(
        const FormDocumentNode& document,
        const CodegenOptions& options,
        SourceMap* outSourceMap
    ) const {
        std::ostringstream ss;
        std::string baseClass = options.customBaseClass.empty() ? "cppui::app::Form" : options.customBaseClass;
        std::string className = options.formClassName.empty() ? "MainForm" : options.formClassName;
        std::string designerClassName = className + "_Designer";

        // 收集所有節點
        size_t autoIndex = 0;
        std::vector<NodeCollectInfo> collected;
        const WidgetNode* root = document.GetRoot();
        if (root) {
            CollectNodes(root, autoIndex, options, collected);
        }

        // 收集需要 include 的標頭檔與事件列表
        std::set<std::string> controlHeaders;
        std::vector<EventBindingDescriptor> allEventBindings;

        for (const auto& item : collected) {
            NodeType type = item.node->GetType();
            switch (type) {
                case NodeType::Canvas:                 controlHeaders.insert("ui/layout/Canvas.hpp"); break;
                case NodeType::StackPanel:             controlHeaders.insert("ui/layout/StackPanel.hpp"); break;
                case NodeType::VirtualizingStackPanel: controlHeaders.insert("ui/layout/VirtualizingStackPanel.hpp"); break;
                case NodeType::Grid:                   controlHeaders.insert("ui/layout/Grid.hpp"); break;
                case NodeType::Border:                 controlHeaders.insert("ui/layout/Border.hpp"); break;
                case NodeType::ContentControl:         controlHeaders.insert("ui/controls/ContentControl.hpp"); break;
                case NodeType::ScrollViewer:           controlHeaders.insert("ui/controls/ScrollViewer.hpp"); break;
                case NodeType::Popup:                  controlHeaders.insert("ui/controls/Popup.hpp"); break;
                case NodeType::TreeView:               controlHeaders.insert("ui/controls/TreeView.hpp"); break;
                case NodeType::ContextMenu:            controlHeaders.insert("ui/controls/ContextMenu.hpp"); break;
                case NodeType::MenuItem:               controlHeaders.insert("ui/controls/MenuItem.hpp"); break;
                case NodeType::Button:                 controlHeaders.insert("ui/controls/Button.hpp"); break;
                case NodeType::CheckBox:               controlHeaders.insert("ui/controls/CheckBox.hpp"); break;
                case NodeType::ComboBox:               controlHeaders.insert("ui/controls/ComboBox.hpp"); break;
                case NodeType::DataGrid:               controlHeaders.insert("ui/controls/DataGrid.hpp"); break;
                case NodeType::Label:                  controlHeaders.insert("ui/controls/Label.hpp"); break;
                case NodeType::ListBox:                controlHeaders.insert("ui/controls/ListBox.hpp"); break;
                case NodeType::PictureBox:             controlHeaders.insert("ui/controls/PictureBox.hpp"); break;
                case NodeType::ProgressBar:            controlHeaders.insert("ui/controls/ProgressBar.hpp"); break;
                case NodeType::RadioButton:            controlHeaders.insert("ui/controls/RadioButton.hpp"); break;
                case NodeType::Separator:              controlHeaders.insert("ui/controls/Separator.hpp"); break;
                case NodeType::Slider:                 controlHeaders.insert("ui/controls/Slider.hpp"); break;
                case NodeType::TextBlock:              controlHeaders.insert("ui/controls/TextBlock.hpp"); break;
                case NodeType::TextBox:                controlHeaders.insert("ui/controls/TextBox.hpp"); break;
                default: break;
            }

            for (const auto& ev : item.node->GetEventBindings()) {
                allEventBindings.push_back(ev);
            }
        }

        // 發射標頭頂部
        ss << "/**\n"
           << " * @file " << className << ".designer.hpp\n"
           << " * @brief DO NOT EDIT: Automatically generated by CPPUI Visual Form Designer.\n"
           << " */\n\n"
           << "#pragma once\n\n"
           << "#include <memory>\n"
           << "#include <string>\n"
           << "#include \"compat/Compat.hpp\"\n"
           << "#include \"ui/app/Form.hpp\"\n"
           << "#include \"ui/input/PointerEventArgs.hpp\"\n"
           << "#include \"ui/input/KeyEventArgs.hpp\"\n";

        for (const auto& h : controlHeaders) {
            ss << "#include \"" << h << "\"\n";
        }
        ss << "\n";

        if (!options.rootNamespace.empty()) {
            ss << "namespace " << options.rootNamespace << " {\n\n";
        }

        ss << "/// <summary>\n"
           << "/// " << designerClassName << " 設計器基底類別。\n"
           << "/// 負責定義所有表單視覺元件指標、初始化階層與提供虛擬事件常式宣告。\n"
           << "/// </summary>\n"
           << "class " << designerClassName << " : public " << baseClass << " {\n"
           << "public:\n"
           << "    " << designerClassName << "();\n"
           << "    virtual ~" << designerClassName << "() = default;\n\n"
           << "protected:\n"
           << "    // =========================================================================\n"
           << "    // 控制項成員指標 (Control Members)\n"
           << "    // =========================================================================\n";

        size_t currentLine = 35; // 大約行數估算

        // 發射成員指標
        for (const auto& item : collected) {
            if (item.isMember) {
                std::string cppType = GetWidgetCppType(item.node->GetType());
                ss << "    std::shared_ptr<" << cppType << "> " << item.memberName << ";\n";

                if (outSourceMap) {
                    SymbolInfo sym;
                    sym.nodeUuid = item.node->GetUuid();
                    sym.nodeType = item.node->GetType();
                    sym.memberName = item.memberName;
                    sym.variableName = item.varName;
                    sym.typeName = cppType;
                    sym.spanInDesignerHpp.startLine = currentLine;
                    sym.spanInDesignerHpp.endLine = currentLine;
                    outSourceMap->RegisterSymbol(sym);
                }
                currentLine++;
            }
        }

        ss << "\n"
           << "    // =========================================================================\n"
           << "    // 視覺元件樹初始化 (InitializeComponent)\n"
           << "    // =========================================================================\n"
           << "    void InitializeComponent();\n\n"
           << "    // =========================================================================\n"
           << "    // 虛擬事件處理常式 (Virtual Event Stubs)\n"
           << "    // =========================================================================\n";

        for (const auto& ev : allEventBindings) {
            if (ev.eventName == "Click") {
                ss << "    virtual void " << ev.handlerName << "(const cppui::input::PointerEventArgs& e) { (void)e; }\n";
            } else if (ev.eventName == "TextChanged") {
                ss << "    virtual void " << ev.handlerName << "(compat::string_view text) { (void)text; }\n";
            } else if (ev.eventName == "CheckedChanged") {
                ss << "    virtual void " << ev.handlerName << "(bool isChecked) { (void)isChecked; }\n";
            } else if (ev.eventName == "ValueChanged") {
                ss << "    virtual void " << ev.handlerName << "(float value) { (void)value; }\n";
            } else {
                ss << "    virtual void " << ev.handlerName << "() {}\n";
            }
        }

        ss << "};\n\n";

        // 發射建構子與 InitializeComponent inline 實作
        ss << "inline " << designerClassName << "::" << designerClassName << "() {\n"
           << "    // 呼叫 InitializeComponent 建立元件樹\n"
           << "}\n\n";

        ss << "inline void " << designerClassName << "::InitializeComponent() {\n";

        // 設定表單基本屬性
        const auto& settings = document.GetSettings();
        ss << "    SetTitle(\"" << EscapeString(settings.title) << "\");\n";
        ss << "    SetFormSize(" << static_cast<uint32_t>(settings.initialSize.width) << ", "
           << static_cast<uint32_t>(settings.initialSize.height) << ");\n\n";

        // 1. 建立所有節點實例
        for (const auto& item : collected) {
            std::string cppType = GetWidgetCppType(item.node->GetType());
            if (item.isMember) {
                ss << "    " << item.memberName << " = std::make_shared<" << cppType << ">();\n";
            } else {
                ss << "    auto " << item.varName << " = std::make_shared<" << cppType << ">();\n";
            }
        }
        ss << "\n";

        // 2. 設定屬性
        for (const auto& item : collected) {
            const auto* node = item.node;
            NodeType type = node->GetType();

            // 自身屬性
            std::map<std::string, PropertyValue> sortedProps;
            for (const auto& kv : node->GetAllProperties()) {
                sortedProps.insert(kv);
            }

            for (const auto& kv : sortedProps) {
                const std::string& propName = kv.first;
                const PropertyValue& val = kv.second;

                if (options.elideDefaults && PropertyMetadataRegistry::IsDefaultValue(type, propName, val)) {
                    continue;
                }

                ss << "    " << item.varName << "->Set" << propName << "(" << FormatPropertyValue(val) << ");\n";
            }

            // Grid 定義
            if (type == NodeType::Grid) {
                for (const auto& rowDef : node->GetRowDefinitions()) {
                    const auto& len = rowDef.height;
                    if (len.type == cppui::layout::GridUnitType::Star) {
                        ss << "    " << item.varName << "->AddRowDefinition(cppui::layout::RowDefinition(cppui::layout::GridLength::Star("
                           << FormatFloat(len.value) << ")));\n";
                    } else if (len.type == cppui::layout::GridUnitType::Auto) {
                        ss << "    " << item.varName << "->AddRowDefinition(cppui::layout::RowDefinition(cppui::layout::GridLength::Auto()));\n";
                    } else {
                        ss << "    " << item.varName << "->AddRowDefinition(cppui::layout::RowDefinition(cppui::layout::GridLength::Pixel("
                           << FormatFloat(len.value) << ")));\n";
                    }
                }
                for (const auto& colDef : node->GetColumnDefinitions()) {
                    const auto& len = colDef.width;
                    if (len.type == cppui::layout::GridUnitType::Star) {
                        ss << "    " << item.varName << "->AddColumnDefinition(cppui::layout::ColumnDefinition(cppui::layout::GridLength::Star("
                           << FormatFloat(len.value) << ")));\n";
                    } else if (len.type == cppui::layout::GridUnitType::Auto) {
                        ss << "    " << item.varName << "->AddColumnDefinition(cppui::layout::ColumnDefinition(cppui::layout::GridLength::Auto()));\n";
                    } else {
                        ss << "    " << item.varName << "->AddColumnDefinition(cppui::layout::ColumnDefinition(cppui::layout::GridLength::Pixel("
                           << FormatFloat(len.value) << ")));\n";
                    }
                }
            }

            // 附加屬性
            for (const auto& kv : node->GetAllAttachedProperties()) {
                const std::string& attName = kv.first;
                const PropertyValue& val = kv.second;

                if (attName == "Grid.Row" && val.Is<int32_t>()) {
                    ss << "    cppui::layout::Grid::SetRow(*" << item.varName << ", " << val.Get<int32_t>().value() << ");\n";
                } else if (attName == "Grid.Column" && val.Is<int32_t>()) {
                    ss << "    cppui::layout::Grid::SetColumn(*" << item.varName << ", " << val.Get<int32_t>().value() << ");\n";
                } else if (attName == "Grid.RowSpan" && val.Is<int32_t>()) {
                    ss << "    cppui::layout::Grid::SetRowSpan(*" << item.varName << ", " << val.Get<int32_t>().value() << ");\n";
                } else if (attName == "Grid.ColumnSpan" && val.Is<int32_t>()) {
                    ss << "    cppui::layout::Grid::SetColumnSpan(*" << item.varName << ", " << val.Get<int32_t>().value() << ");\n";
                } else if (attName == "Canvas.Left" && val.Is<double>()) {
                    ss << "    cppui::layout::Canvas::SetLeft(*" << item.varName << ", " << FormatFloat(val.Get<double>().value()) << ");\n";
                } else if (attName == "Canvas.Top" && val.Is<double>()) {
                    ss << "    cppui::layout::Canvas::SetTop(*" << item.varName << ", " << FormatFloat(val.Get<double>().value()) << ");\n";
                }
            }

            // 事件繫結
            for (const auto& ev : node->GetEventBindings()) {
                if (ev.eventName == "Click") {
                    ss << "    " << item.varName << "->OnClick([this](const cppui::input::PointerEventArgs& e) {\n"
                       << "        " << ev.handlerName << "(e);\n"
                       << "    });\n";
                } else if (ev.eventName == "TextChanged") {
                    ss << "    " << item.varName << "->OnTextChanged([this](compat::string_view text) {\n"
                       << "        " << ev.handlerName << "(text);\n"
                       << "    });\n";
                } else if (ev.eventName == "CheckedChanged") {
                    ss << "    " << item.varName << "->OnCheckedChanged([this](bool checked) {\n"
                       << "        " << ev.handlerName << "(checked);\n"
                       << "    });\n";
                } else if (ev.eventName == "ValueChanged") {
                    ss << "    " << item.varName << "->OnValueChanged([this](float val) {\n"
                       << "        " << ev.handlerName << "(val);\n"
                       << "    });\n";
                }
            }
        }
        ss << "\n";

        // 3. 建立階層父子關係
        std::map<const WidgetNode*, std::string> nodeToVar;
        for (const auto& item : collected) {
            nodeToVar[item.node] = item.varName;
        }

        for (const auto& item : collected) {
            const auto* parentNode = item.node;
            if (parentNode->GetChildCount() > 0) {
                std::string parentVar = item.varName;
                if (parentNode->GetType() == NodeType::Border) {
                    const auto* childNode = parentNode->GetChild(0);
                    ss << "    " << parentVar << "->SetChild(" << nodeToVar[childNode] << ");\n";
                } else {
                    for (size_t i = 0; i < parentNode->GetChildCount(); ++i) {
                        const auto* childNode = parentNode->GetChild(i);
                        ss << "    " << parentVar << "->AddChild(" << nodeToVar[childNode] << ");\n";
                    }
                }
            }
        }
        ss << "\n";

        // 4. 掛載至 Form 客戶區 Content
        if (root) {
            ss << "    SetContent(" << nodeToVar[root] << ");\n";
        }

        ss << "}\n\n";

        if (!options.rootNamespace.empty()) {
            ss << "} // namespace " << options.rootNamespace << "\n";
        }

        return ss.str();
    }

    std::string EmitUserHeaderInternal(
        const FormDocumentNode& document,
        const CodegenOptions& options
    ) const {
        std::ostringstream ss;
        std::string className = options.formClassName.empty() ? "MainForm" : options.formClassName;
        std::string designerClassName = className + "_Designer";

        ss << "/**\n"
           << " * @file " << className << ".hpp\n"
           << " * @brief User custom form class declaration.\n"
           << " */\n\n"
           << "#pragma once\n\n"
           << "#include \"" << className << ".designer.hpp\"\n\n";

        if (!options.rootNamespace.empty()) {
            ss << "namespace " << options.rootNamespace << " {\n\n";
        }

        ss << "class " << className << " : public " << designerClassName << " {\n"
           << "public:\n"
           << "    " << className << "();\n"
           << "    ~" << className << "() override = default;\n\n"
           << "protected:\n";

        // 收集所有事件虛擬常式
        size_t autoIdx = 0;
        std::vector<NodeCollectInfo> collected;
        const WidgetNode* root = document.GetRoot();
        if (root) {
            CollectNodes(root, autoIdx, options, collected);
        }

        for (const auto& item : collected) {
            for (const auto& ev : item.node->GetEventBindings()) {
                if (ev.eventName == "Click") {
                    ss << "    void " << ev.handlerName << "(const cppui::input::PointerEventArgs& e) override;\n";
                } else if (ev.eventName == "TextChanged") {
                    ss << "    void " << ev.handlerName << "(compat::string_view text) override;\n";
                } else if (ev.eventName == "CheckedChanged") {
                    ss << "    void " << ev.handlerName << "(bool isChecked) override;\n";
                } else if (ev.eventName == "ValueChanged") {
                    ss << "    void " << ev.handlerName << "(float value) override;\n";
                } else {
                    ss << "    void " << ev.handlerName << "() override;\n";
                }
            }
        }

        ss << "};\n\n";

        if (!options.rootNamespace.empty()) {
            ss << "} // namespace " << options.rootNamespace << "\n";
        }

        return ss.str();
    }

    std::string EmitUserSourceInternal(
        const FormDocumentNode& document,
        const CodegenOptions& options
    ) const {
        std::ostringstream ss;
        std::string className = options.formClassName.empty() ? "MainForm" : options.formClassName;

        ss << "/**\n"
           << " * @file " << className << ".cpp\n"
           << " * @brief User custom form class implementation.\n"
           << " */\n\n"
           << "#include \"" << className << ".hpp\"\n\n";

        if (!options.rootNamespace.empty()) {
            ss << "namespace " << options.rootNamespace << " {\n\n";
        }

        ss << className << "::" << className << "() {\n"
           << "    InitializeComponent();\n"
           << "}\n\n";

        // 收集並實作事件常式骨架
        size_t autoIdx = 0;
        std::vector<NodeCollectInfo> collected;
        const WidgetNode* root = document.GetRoot();
        if (root) {
            CollectNodes(root, autoIdx, options, collected);
        }

        for (const auto& item : collected) {
            for (const auto& ev : item.node->GetEventBindings()) {
                if (ev.eventName == "Click") {
                    ss << "void " << className << "::" << ev.handlerName << "(const cppui::input::PointerEventArgs& e) {\n"
                       << "    (void)e;\n"
                       << "    // TODO: Handle " << ev.handlerName << "\n"
                       << "}\n\n";
                } else if (ev.eventName == "TextChanged") {
                    ss << "void " << className << "::" << ev.handlerName << "(compat::string_view text) {\n"
                       << "    (void)text;\n"
                       << "    // TODO: Handle " << ev.handlerName << "\n"
                       << "}\n\n";
                } else if (ev.eventName == "CheckedChanged") {
                    ss << "void " << className << "::" << ev.handlerName << "(bool isChecked) {\n"
                       << "    (void)isChecked;\n"
                       << "    // TODO: Handle " << ev.handlerName << "\n"
                       << "}\n\n";
                } else if (ev.eventName == "ValueChanged") {
                    ss << "void " << className << "::" << ev.handlerName << "(float value) {\n"
                       << "    (void)value;\n"
                       << "    // TODO: Handle " << ev.handlerName << "\n"
                       << "}\n\n";
                } else {
                    ss << "void " << className << "::" << ev.handlerName << "() {\n"
                       << "    // TODO: Handle " << ev.handlerName << "\n"
                       << "}\n\n";
                }
            }
        }

        if (!options.rootNamespace.empty()) {
            ss << "} // namespace " << options.rootNamespace << "\n";
        }

        return ss.str();
    }
};

FormClassEmitter::FormClassEmitter() : m_impl(std::make_unique<Impl>()) {}
FormClassEmitter::~FormClassEmitter() = default;
FormClassEmitter::FormClassEmitter(FormClassEmitter&& other) noexcept = default;
FormClassEmitter& FormClassEmitter::operator=(FormClassEmitter&& other) noexcept = default;

std::string FormClassEmitter::EmitDesignerHeader(
    const FormDocumentNode& document,
    const CodegenOptions& options,
    SourceMap* outSourceMap
) const {
    return m_impl->EmitDesignerHeaderInternal(document, options, outSourceMap);
}

std::string FormClassEmitter::EmitUserHeader(
    const FormDocumentNode& document,
    const CodegenOptions& options
) const {
    return m_impl->EmitUserHeaderInternal(document, options);
}

std::string FormClassEmitter::EmitUserSource(
    const FormDocumentNode& document,
    const CodegenOptions& options
) const {
    return m_impl->EmitUserSourceInternal(document, options);
}

} // namespace codegen
} // namespace designer
} // namespace cppui
