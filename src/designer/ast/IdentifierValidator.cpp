/**
 * @file IdentifierValidator.cpp
 * @brief C++ 識別字合法性、關鍵字防護與命名消歧義實作
 */

#include "ui/designer/ast/IdentifierValidator.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include <unordered_set>
#include <cctype>

namespace cppui {
namespace designer {

namespace {

const std::unordered_set<std::string>& GetCppKeywords() {
    static const std::unordered_set<std::string> s_keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit",
        "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch",
        "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const",
        "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await",
        "co_return", "co_yield", "decltype", "default", "delete", "do", "double",
        "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float",
        "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace",
        "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq",
        "private", "protected", "public", "reflexpr", "register", "reinterpret_cast",
        "requires", "return", "short", "signed", "sizeof", "static", "static_assert",
        "static_cast", "struct", "switch", "synchronized", "template", "this",
        "thread_local", "throw", "true", "try", "typedef", "typeid", "typename",
        "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t",
        "while", "xor", "xor_eq"
    };
    return s_keywords;
}

const std::unordered_set<std::string>& GetFrameworkReservedMembers() {
    static const std::unordered_set<std::string> s_reserved = {
        "SetTitle", "GetTitle", "SetSize", "GetSize", "Show", "Close",
        "OnPaint", "OnKeyDown", "OnKeyUp", "OnPointerPressed", "OnPointerReleased",
        "OnPointerMoved", "Measure", "Arrange", "Invalidate", "UpdateLayout",
        "GetDesiredSize", "GetRenderSize", "GetParent", "SetParent", "AddChild",
        "RemoveChild", "FindByName", "FindByUuid"
    };
    return s_reserved;
}

void CheckNodeUniqueness(const WidgetNode* node, compat::string_view name, compat::string_view excludeUuid, bool& isUnique) {
    if (!node || !isUnique) {
        return;
    }
    if (node->GetName() == name && node->GetUuid() != excludeUuid) {
        isUnique = false;
        return;
    }
    for (const auto& child : node->GetChildren()) {
        CheckNodeUniqueness(child.get(), name, excludeUuid, isUnique);
    }
}

std::string GetBaseNameForType(NodeType type) {
    switch (type) {
        case NodeType::Canvas:                 return "canvas";
        case NodeType::StackPanel:             return "stackPanel";
        case NodeType::VirtualizingStackPanel: return "virtualizingStackPanel";
        case NodeType::Grid:                   return "grid";
        case NodeType::Border:                 return "border";
        case NodeType::ContentControl:         return "contentControl";
        case NodeType::ScrollViewer:           return "scrollViewer";
        case NodeType::Popup:                  return "popup";
        case NodeType::TreeView:               return "treeView";
        case NodeType::ContextMenu:            return "contextMenu";
        case NodeType::MenuItem:               return "menuItem";
        case NodeType::Button:                 return "button";
        case NodeType::CheckBox:               return "checkBox";
        case NodeType::ComboBox:               return "comboBox";
        case NodeType::DataGrid:               return "dataGrid";
        case NodeType::Label:                  return "label";
        case NodeType::ListBox:                return "listBox";
        case NodeType::PictureBox:             return "pictureBox";
        case NodeType::ProgressBar:            return "progressBar";
        case NodeType::RadioButton:            return "radioButton";
        case NodeType::Separator:              return "separator";
        case NodeType::Slider:                 return "slider";
        case NodeType::TextBlock:              return "textBlock";
        case NodeType::TextBox:                return "textBox";
        default:                               return "widget";
    }
}

} // anonymous namespace

compat::expected<void, DesignerError> IdentifierValidator::ValidateIdentifier(compat::string_view name) noexcept {
    if (name.empty()) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidIdentifier;
        err.message = "Identifier name cannot be empty";
        return compat::unexpected(std::move(err));
    }

    // 1. 首字元必須為字母或底線
    char first = name.front();
    if (!std::isalpha(static_cast<unsigned char>(first)) && first != '_') {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidIdentifier;
        err.message = "Identifier must start with a letter or an underscore";
        return compat::unexpected(std::move(err));
    }

    // 2. 後續字元必須為英數字或底線
    for (size_t i = 1; i < name.size(); ++i) {
        char ch = name[i];
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            DesignerError err;
            err.code = DesignerErrorCode::InvalidIdentifier;
            err.message = "Identifier contains invalid character: " + std::string(1, ch);
            return compat::unexpected(std::move(err));
        }
    }

    std::string nameStr(name);

    // 3. 檢查是否與 C++ 關鍵字衝突
    if (GetCppKeywords().count(nameStr) > 0) {
        DesignerError err;
        err.code = DesignerErrorCode::ReservedKeywordCollision;
        err.message = "Identifier '" + nameStr + "' collides with a C++ reserved keyword";
        return compat::unexpected(std::move(err));
    }

    // 4. 檢查是否與 CppUI 核心成員衝突
    if (GetFrameworkReservedMembers().count(nameStr) > 0) {
        DesignerError err;
        err.code = DesignerErrorCode::FrameworkMemberCollision;
        err.message = "Identifier '" + nameStr + "' collides with a CppUI framework member method";
        return compat::unexpected(std::move(err));
    }

    return {};
}

bool IdentifierValidator::IsUniqueInDocument(
    const FormDocumentNode& doc,
    compat::string_view name,
    compat::string_view excludeUuid
) noexcept {
    const WidgetNode* root = doc.GetRoot();
    if (!root) {
        return true;
    }
    bool isUnique = true;
    CheckNodeUniqueness(root, name, excludeUuid, isUnique);
    return isUnique;
}

std::string IdentifierValidator::GenerateUniqueName(
    const FormDocumentNode& doc,
    NodeType type
) {
    std::string base = GetBaseNameForType(type);
    int32_t counter = 1;
    while (true) {
        std::string candidate = base + std::to_string(counter);
        if (IsUniqueInDocument(doc, candidate)) {
            return candidate;
        }
        counter++;
    }
}

} // namespace designer
} // namespace cppui
