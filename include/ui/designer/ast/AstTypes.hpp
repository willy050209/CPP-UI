/**
 * @file AstTypes.hpp
 * @brief CPPUI Designer AST 核心節點類型、容器分類與結構化錯誤定義
 */

#pragma once

#include <cstdint>
#include <string>
#include "compat/Compat.hpp"
#include "ui/core/Exports.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// AST 控制項與容器節點型別列舉
/// </summary>
enum class NodeType : uint32_t {
    // 根節點
    FormDocument = 0,

    // 多子元素佈局面板 (MultiChildPanel)
    Canvas,
    StackPanel,
    VirtualizingStackPanel,
    Grid,

    // 單一子元素裝飾/內容容器 (SingleChildContainer)
    Border,
    ContentControl,
    ScrollViewer,
    Popup,

    // 階層式項目容器 (HierarchicalContainer)
    TreeView,
    ContextMenu,
    MenuItem,

    // 葉節點控制項 (LeafControl)
    Button,
    CheckBox,
    ComboBox,
    DataGrid,
    Label,
    ListBox,
    PictureBox,
    ProgressBar,
    RadioButton,
    Separator,
    Slider,
    TextBlock,
    TextBox,

    // 非視覺元件與對話方塊 (Non-Visual Components & Dialogs)
    Timer,
    OpenFileDialog,
    SaveFileDialog,
    MessageBox
};

/// <summary>
/// 容器節點約束分類
/// </summary>
enum class ContainerCategory : uint8_t {
    RootDocument = 0, // 恰好持有 1 個根容器
    MultiChild   = 1, // 0 ~ N 個子節點
    SingleChild  = 2, // 0 ~ 1 個子節點
    Hierarchical = 3, // 0 ~ N 個特定型別子節點
    Leaf         = 4  // 0 個子節點 (禁止子元素)
};

/// <summary>
/// 取得指定節點型別之容器分類
/// </summary>
/// <param name="type">節點型別。</param>
/// <returns>容器分類列舉值。</returns>
COMPAT_NODISCARD inline ContainerCategory GetContainerCategory(NodeType type) noexcept {
    switch (type) {
        case NodeType::FormDocument:
            return ContainerCategory::RootDocument;
        case NodeType::Canvas:
        case NodeType::StackPanel:
        case NodeType::VirtualizingStackPanel:
        case NodeType::Grid:
            return ContainerCategory::MultiChild;
        case NodeType::Border:
        case NodeType::ContentControl:
        case NodeType::ScrollViewer:
        case NodeType::Popup:
            return ContainerCategory::SingleChild;
        case NodeType::TreeView:
        case NodeType::ContextMenu:
        case NodeType::MenuItem:
            return ContainerCategory::Hierarchical;
        default:
            return ContainerCategory::Leaf;
    }
}

/// <summary>
/// 取得節點型別標準字串名稱
/// </summary>
/// <param name="type">節點型別。</param>
/// <returns>字串視圖。</returns>
COMPAT_NODISCARD inline compat::string_view NodeTypeName(NodeType type) noexcept {
    switch (type) {
        case NodeType::FormDocument:           return "FormDocument";
        case NodeType::Canvas:                 return "Canvas";
        case NodeType::StackPanel:             return "StackPanel";
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
        case NodeType::Timer:                  return "Timer";
        case NodeType::OpenFileDialog:         return "OpenFileDialog";
        case NodeType::SaveFileDialog:         return "SaveFileDialog";
        case NodeType::MessageBox:             return "MessageBox";
        default:                               return "Unknown";
    }
}

/// <summary>
/// 從字串名稱解析節點型別
/// </summary>
/// <param name="str">節點名稱字串。</param>
/// <returns>解析成功之 NodeType，若未知則為 nullopt。</returns>
COMPAT_NODISCARD inline compat::optional<NodeType> NodeTypeFromString(compat::string_view str) noexcept {
    if (str == "FormDocument")           return NodeType::FormDocument;
    if (str == "Canvas")                 return NodeType::Canvas;
    if (str == "StackPanel")             return NodeType::StackPanel;
    if (str == "VirtualizingStackPanel") return NodeType::VirtualizingStackPanel;
    if (str == "Grid")                   return NodeType::Grid;
    if (str == "Border")                 return NodeType::Border;
    if (str == "ContentControl")         return NodeType::ContentControl;
    if (str == "ScrollViewer")           return NodeType::ScrollViewer;
    if (str == "Popup")                  return NodeType::Popup;
    if (str == "TreeView")               return NodeType::TreeView;
    if (str == "ContextMenu")            return NodeType::ContextMenu;
    if (str == "MenuItem")               return NodeType::MenuItem;
    if (str == "Button")                 return NodeType::Button;
    if (str == "CheckBox")               return NodeType::CheckBox;
    if (str == "ComboBox")               return NodeType::ComboBox;
    if (str == "DataGrid")               return NodeType::DataGrid;
    if (str == "Label")                  return NodeType::Label;
    if (str == "ListBox")                return NodeType::ListBox;
    if (str == "PictureBox")             return NodeType::PictureBox;
    if (str == "ProgressBar")            return NodeType::ProgressBar;
    if (str == "RadioButton")            return NodeType::RadioButton;
    if (str == "Separator")              return NodeType::Separator;
    if (str == "Slider")                 return NodeType::Slider;
    if (str == "TextBlock")              return NodeType::TextBlock;
    if (str == "TextBox")                return NodeType::TextBox;
    if (str == "Timer")                  return NodeType::Timer;
    if (str == "OpenFileDialog")         return NodeType::OpenFileDialog;
    if (str == "SaveFileDialog")         return NodeType::SaveFileDialog;
    if (str == "MessageBox")             return NodeType::MessageBox;
    return compat::nullopt;
}

/// <summary>
/// 結構化錯誤代碼列舉
/// </summary>
enum class DesignerErrorCode : uint32_t {
    Success = 0,
    InvalidNode,
    NodeNotFound,
    DuplicateIdentifier,
    InvalidIdentifier,
    ReservedKeywordCollision,
    FrameworkMemberCollision,
    ContainerFull,
    InvalidChildType,
    CyclicHierarchy,
    TypeMismatch,
    PropertyNotFound,
    SerializationError,
    DeserializationError,
    InvalidJson,
    SchemaMismatch,
    UndoStackEmpty,
    RedoStackEmpty
};

/// <summary>
/// Designer 結構化錯誤資訊描述項
/// </summary>
struct DesignerError {
    DesignerErrorCode code = DesignerErrorCode::Success;
    std::string message;
    std::string jsonPointer; // 例如 "/root/children/2/properties/Width"

    /// <summary>檢查操作是否成功。</summary>
    COMPAT_NODISCARD bool IsSuccess() const noexcept {
        return code == DesignerErrorCode::Success;
    }

    /// <summary>相等性比較運算子。</summary>
    COMPAT_NODISCARD bool operator==(const DesignerError& other) const noexcept {
        return code == other.code && message == other.message && jsonPointer == other.jsonPointer;
    }

    /// <summary>不相等性比較運算子。</summary>
    COMPAT_NODISCARD bool operator!=(const DesignerError& other) const noexcept {
        return !(*this == other);
    }
};

/// <summary>
/// 資料綁定模式 (OneWay / TwoWay)
/// </summary>
enum class BindingMode : uint8_t {
    OneWay = 0,
    TwoWay = 1
};

/// <summary>
/// 事件路由綁定描述項
/// </summary>
struct EventBindingDescriptor {
    std::string eventName;   // 例如 "Click", "TextChanged"
    std::string handlerName; // 例如 "OnSubmitClicked"

    COMPAT_NODISCARD bool operator==(const EventBindingDescriptor& other) const noexcept {
        return eventName == other.eventName && handlerName == other.handlerName;
    }
};

/// <summary>
/// MVVM 響應式屬性綁定描述項
/// </summary>
struct DataBindingDescriptor {
    std::string propertyName; // 例如 "Text", "Value"
    std::string sourcePath;   // 例如 "ViewModel.Status"
    BindingMode mode = BindingMode::OneWay;

    COMPAT_NODISCARD bool operator==(const DataBindingDescriptor& other) const noexcept {
        return propertyName == other.propertyName && sourcePath == other.sourcePath && mode == other.mode;
    }
};

} // namespace designer
} // namespace cppui
