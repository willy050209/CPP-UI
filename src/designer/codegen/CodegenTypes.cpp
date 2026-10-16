/**
 * @file CodegenTypes.cpp
 * @brief CPPUI Designer 程式碼生成型別、命名風格與 SourceMap 實作
 */

#include "ui/designer/codegen/CodegenTypes.hpp"
#include <cctype>
#include <algorithm>

namespace cppui {
namespace designer {
namespace codegen {

void SourceMap::RegisterSymbol(const SymbolInfo& info) {
    size_t idx = m_symbols.size();
    m_symbols.push_back(info);
    if (!info.nodeUuid.empty()) {
        m_uuidIndex[info.nodeUuid] = idx;
    }
    if (!info.variableName.empty()) {
        m_varIndex[info.variableName] = idx;
    }
    if (!info.memberName.empty()) {
        m_memberIndex[info.memberName] = idx;
    }
}

compat::optional<SymbolInfo> SourceMap::FindByNodeUuid(compat::string_view uuid) const {
    std::string key(uuid.data(), uuid.size());
    auto it = m_uuidIndex.find(key);
    if (it != m_uuidIndex.end() && it->second < m_symbols.size()) {
        return m_symbols[it->second];
    }
    return compat::nullopt;
}

compat::optional<SymbolInfo> SourceMap::FindByVariableName(compat::string_view name) const {
    std::string key(name.data(), name.size());
    auto itVar = m_varIndex.find(key);
    if (itVar != m_varIndex.end() && itVar->second < m_symbols.size()) {
        return m_symbols[itVar->second];
    }
    auto itMember = m_memberIndex.find(key);
    if (itMember != m_memberIndex.end() && itMember->second < m_symbols.size()) {
        return m_symbols[itMember->second];
    }
    return compat::nullopt;
}

compat::optional<SymbolInfo> SourceMap::FindByLine(size_t line) const {
    for (const auto& sym : m_symbols) {
        if (sym.spanInDesignerHpp.startLine <= line && line <= sym.spanInDesignerHpp.endLine) {
            return sym;
        }
    }
    return compat::nullopt;
}

const std::vector<SymbolInfo>& SourceMap::GetAllSymbols() const noexcept {
    return m_symbols;
}

void SourceMap::Clear() noexcept {
    m_symbols.clear();
    m_uuidIndex.clear();
    m_varIndex.clear();
    m_memberIndex.clear();
}

std::string FormatIdentifier(compat::string_view rawName, NamingStyle style) {
    if (rawName.empty()) {
        return "";
    }

    std::string str(rawName.data(), rawName.size());
    // 去除常見前綴 m_ 或 m
    if (str.rfind("m_", 0) == 0 && str.size() > 2) {
        str = str.substr(2);
    }

    // 分詞分析 (按底線或大寫字母切分)
    std::vector<std::string> words;
    std::string currentWord;
    for (size_t i = 0; i < str.size(); ++i) {
        char ch = str[i];
        if (ch == '_' || ch == '-' || ch == ' ') {
            if (!currentWord.empty()) {
                words.push_back(currentWord);
                currentWord.clear();
            }
        } else if (std::isupper(static_cast<unsigned char>(ch))) {
            if (!currentWord.empty() && (i + 1 < str.size() && std::islower(static_cast<unsigned char>(str[i + 1])))) {
                words.push_back(currentWord);
                currentWord.clear();
            }
            currentWord.push_back(ch);
        } else {
            currentWord.push_back(ch);
        }
    }
    if (!currentWord.empty()) {
        words.push_back(currentWord);
    }

    if (words.empty()) {
        return "control";
    }

    switch (style) {
        case NamingStyle::CamelCase: {
            std::string result;
            for (size_t i = 0; i < words.size(); ++i) {
                std::string w = words[i];
                if (i == 0) {
                    for (char& c : w) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                } else {
                    if (!w.empty()) {
                        w[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(w[0])));
                        for (size_t j = 1; j < w.size(); ++j) {
                            w[j] = static_cast<char>(std::tolower(static_cast<unsigned char>(w[j])));
                        }
                    }
                }
                result += w;
            }
            return result;
        }
        case NamingStyle::PascalCase: {
            std::string result;
            for (auto& w : words) {
                if (!w.empty()) {
                    w[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(w[0])));
                    for (size_t j = 1; j < w.size(); ++j) {
                        w[j] = static_cast<char>(std::tolower(static_cast<unsigned char>(w[j])));
                    }
                }
                result += w;
            }
            return result;
        }
        case NamingStyle::SnakeCase: {
            std::string result;
            for (size_t i = 0; i < words.size(); ++i) {
                if (i > 0) result += '_';
                std::string w = words[i];
                for (char& c : w) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                result += w;
            }
            return result;
        }
        case NamingStyle::MemberMStyle: {
            return "m_" + FormatIdentifier(rawName, NamingStyle::CamelCase);
        }
    }
    return str;
}

std::string GetWidgetCppType(NodeType type) {
    switch (type) {
        case NodeType::Canvas:                 return "cppui::layout::Canvas";
        case NodeType::StackPanel:             return "cppui::layout::StackPanel";
        case NodeType::VirtualizingStackPanel: return "cppui::layout::VirtualizingStackPanel";
        case NodeType::Grid:                   return "cppui::layout::Grid";
        case NodeType::Border:                 return "cppui::layout::Border";
        case NodeType::ContentControl:         return "cppui::controls::ContentControl";
        case NodeType::ScrollViewer:           return "cppui::controls::ScrollViewer";
        case NodeType::Popup:                  return "cppui::controls::Popup";
        case NodeType::TreeView:               return "cppui::controls::TreeView";
        case NodeType::ContextMenu:            return "cppui::controls::ContextMenu";
        case NodeType::MenuItem:               return "cppui::controls::MenuItem";
        case NodeType::Button:                 return "cppui::controls::Button";
        case NodeType::CheckBox:               return "cppui::controls::CheckBox";
        case NodeType::ComboBox:               return "cppui::controls::ComboBox";
        case NodeType::DataGrid:               return "cppui::controls::DataGrid";
        case NodeType::Label:                  return "cppui::controls::Label";
        case NodeType::ListBox:                return "cppui::controls::ListBox";
        case NodeType::PictureBox:             return "cppui::controls::PictureBox";
        case NodeType::ProgressBar:            return "cppui::controls::ProgressBar";
        case NodeType::RadioButton:            return "cppui::controls::RadioButton";
        case NodeType::Separator:              return "cppui::controls::Separator";
        case NodeType::Slider:                 return "cppui::controls::Slider";
        case NodeType::TextBlock:              return "cppui::controls::TextBlock";
        case NodeType::TextBox:                return "cppui::controls::TextBox";
        default:                               return "cppui::core::UIElement";
    }
}

std::string GetWidgetDslFactory(NodeType type) {
    switch (type) {
        case NodeType::Canvas:                 return "Canvas";
        case NodeType::StackPanel:             return "VStack"; // 預設 VStack，依 Orientation 可變更為 HStack
        case NodeType::VirtualizingStackPanel: return "VirtualizingStackPanel";
        case NodeType::Grid:                   return "Grid";
        case NodeType::Border:                 return "Border";
        case NodeType::ContentControl:         return "ContentControl";
        case NodeType::ScrollViewer:           return "ScrollViewer";
        case NodeType::Popup:                  return "Popup";
        case NodeType::TreeView:               return "TreeView";
        case NodeType::ContextMenu:            return "ContextMenu";
        case NodeType::MenuItem:               return "MenuItem";
        case NodeType::Button:                 return "Button";
        case NodeType::CheckBox:               return "CheckBox";
        case NodeType::ComboBox:               return "ComboBox";
        case NodeType::DataGrid:               return "DataGrid";
        case NodeType::Label:                  return "Label";
        case NodeType::ListBox:                return "ListBox";
        case NodeType::PictureBox:             return "PictureBox";
        case NodeType::ProgressBar:            return "ProgressBar";
        case NodeType::RadioButton:            return "RadioButton";
        case NodeType::Separator:              return "Separator";
        case NodeType::Slider:                 return "Slider";
        case NodeType::TextBlock:              return "TextBlock";
        case NodeType::TextBox:                return "TextBox";
        default:                               return "UIElement";
    }
}

} // namespace codegen
} // namespace designer
} // namespace cppui
