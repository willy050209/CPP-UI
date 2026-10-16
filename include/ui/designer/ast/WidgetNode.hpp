/**
 * @file WidgetNode.hpp
 * @brief CPPUI Designer AST 控制項樹節點公開宣告 (Pimpl 深常數性保證)
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "compat/Compat.hpp"
#include "compat/PropagateConst.hpp"
#include "ui/core/Exports.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/designer/ast/AstTypes.hpp"
#include "ui/designer/metadata/PropertyValue.hpp"

namespace cppui {
namespace designer {

/// <summary>
/// 控制項樹節點類別 (WidgetNode)
/// 封裝個別 UI 元件之不可變 UUID、C++ 變數名稱、屬性字典、附加屬性、事件/MVVM 綁定及子節點集合。
/// 採用現代 Pimpl 慣用法搭配 compat::propagate_const，維持嚴格 ABI 防火牆。
/// </summary>
class CPPUI_API WidgetNode {
public:
    /// <summary>
    /// 建構指定型別之節點，自動生成唯一 UUID。
    /// </summary>
    /// <param name="type">節點型別。</param>
    /// <param name="name">C++ 變數識別字名稱。</param>
    explicit WidgetNode(NodeType type, std::string name = "");

    /// <summary>
    /// 指定 UUID 與名稱建構節點 (用於反序列化與深層復原)。
    /// </summary>
    /// <param name="type">節點型別。</param>
    /// <param name="uuid">唯一 UUID 字串。</param>
    /// <param name="name">C++ 變數識別字名稱。</param>
    WidgetNode(NodeType type, std::string uuid, std::string name);

    /// <summary>解構子，於 .cpp 完整型別處實作。</summary>
    ~WidgetNode();

    /// <summary>移動建構子。</summary>
    WidgetNode(WidgetNode&& other) noexcept;

    /// <summary>移動賦值運算子。</summary>
    WidgetNode& operator=(WidgetNode&& other) noexcept;

    // 禁止直接拷貝，需透過 Clone() 明確進行深拷貝
    WidgetNode(const WidgetNode&) = delete;
    WidgetNode& operator=(const WidgetNode&) = delete;

    /// <summary>取得節點型別。</summary>
    COMPAT_NODISCARD NodeType GetType() const noexcept;

    /// <summary>取得節點不可變 UUID。</summary>
    COMPAT_NODISCARD const std::string& GetUuid() const noexcept;

    /// <summary>取得 C++ 變數名稱。</summary>
    COMPAT_NODISCARD const std::string& GetName() const noexcept;

    /// <summary>設定 C++ 變數名稱。</summary>
    /// <param name="name">新名稱。</param>
    void SetName(std::string name);

    /// <summary>取得容器分類。</summary>
    COMPAT_NODISCARD ContainerCategory GetCategory() const noexcept;

    // --- 屬性系統 (Properties) ---

    /// <summary>設定自身屬性。</summary>
    void SetProperty(std::string name, PropertyValue value);

    /// <summary>查詢自身屬性數值。</summary>
    COMPAT_NODISCARD compat::optional<PropertyValue> GetProperty(compat::string_view name) const;

    /// <summary>移除自身屬性。</summary>
    bool RemoveProperty(compat::string_view name);

    /// <summary>取得全體屬性字典視圖。</summary>
    COMPAT_NODISCARD const std::unordered_map<std::string, PropertyValue>& GetAllProperties() const noexcept;

    // --- 附加屬性系統 (Attached Properties) ---

    /// <summary>設定附加屬性 (例如 Grid.Row)。</summary>
    void SetAttachedProperty(std::string name, PropertyValue value);

    /// <summary>查詢附加屬性數值。</summary>
    COMPAT_NODISCARD compat::optional<PropertyValue> GetAttachedProperty(compat::string_view name) const;

    /// <summary>移除附加屬性。</summary>
    bool RemoveAttachedProperty(compat::string_view name);

    /// <summary>取得全體附加屬性字典視圖。</summary>
    COMPAT_NODISCARD const std::unordered_map<std::string, PropertyValue>& GetAllAttachedProperties() const noexcept;

    // --- 拓撲結構與子節點操作 (Hierarchy & Children) ---

    /// <summary>取得弱指向之父容器指標。</summary>
    COMPAT_NODISCARD WidgetNode* GetParent() noexcept;
    COMPAT_NODISCARD const WidgetNode* GetParent() const noexcept;

    /// <summary>設定父節點指標 (內部拓撲維護專用)。</summary>
    void SetParent(WidgetNode* parent) noexcept;

    /// <summary>取得目前子節點數量。</summary>
    COMPAT_NODISCARD size_t GetChildCount() const noexcept;

    /// <summary>依索引存取子節點。</summary>
    COMPAT_NODISCARD WidgetNode* GetChild(size_t index) noexcept;
    COMPAT_NODISCARD const WidgetNode* GetChild(size_t index) const noexcept;

    /// <summary>取得所有子節點之擁有權清單。</summary>
    COMPAT_NODISCARD const std::vector<std::unique_ptr<WidgetNode>>& GetChildren() const noexcept;

    /// <summary>於末端新增子節點 (自動觸發 CardinalityValidator 檢查)。</summary>
    compat::expected<void, DesignerError> AddChild(std::unique_ptr<WidgetNode> child);

    /// <summary>於指定索引插入子節點 (自動觸發 CardinalityValidator 檢查)。</summary>
    compat::expected<void, DesignerError> InsertChild(size_t index, std::unique_ptr<WidgetNode> child);

    /// <summary>依索引移除子節點並取回唯一擁有權。</summary>
    compat::expected<std::unique_ptr<WidgetNode>, DesignerError> RemoveChild(size_t index);

    /// <summary>依 UUID 移除子節點並取回唯一擁有權。</summary>
    compat::expected<std::unique_ptr<WidgetNode>, DesignerError> RemoveChildByUuid(compat::string_view uuid);

    /// <summary>查詢指定子節點之索引位置。</summary>
    COMPAT_NODISCARD compat::optional<size_t> IndexOfChild(const WidgetNode* child) const noexcept;

    // --- 網格專屬定義 (Grid Definitions) ---

    COMPAT_NODISCARD const std::vector<layout::RowDefinition>& GetRowDefinitions() const noexcept;
    void SetRowDefinitions(std::vector<layout::RowDefinition> defs);

    COMPAT_NODISCARD const std::vector<layout::ColumnDefinition>& GetColumnDefinitions() const noexcept;
    void SetColumnDefinitions(std::vector<layout::ColumnDefinition> defs);

    // --- 事件與資料綁定 (Bindings) ---

    COMPAT_NODISCARD const std::vector<EventBindingDescriptor>& GetEventBindings() const noexcept;
    void AddEventBinding(EventBindingDescriptor binding);
    bool RemoveEventBinding(compat::string_view eventName);

    COMPAT_NODISCARD const std::vector<DataBindingDescriptor>& GetDataBindings() const noexcept;
    void AddDataBinding(DataBindingDescriptor binding);
    bool RemoveDataBinding(compat::string_view propertyName);

    // --- 搜尋與複製 ---

    /// <summary>遞迴依 UUID 搜尋子孫節點。</summary>
    COMPAT_NODISCARD WidgetNode* FindByUuid(compat::string_view uuid) noexcept;
    COMPAT_NODISCARD const WidgetNode* FindByUuid(compat::string_view uuid) const noexcept;

    /// <summary>遞迴依名稱搜尋子孫節點。</summary>
    COMPAT_NODISCARD WidgetNode* FindByName(compat::string_view name) noexcept;
    COMPAT_NODISCARD const WidgetNode* FindByName(compat::string_view name) const noexcept;

    /// <summary>深層複製整棵節點子樹。</summary>
    /// <param name="generateNewUuids">若為 true 則為所有新節點產生全新 UUID，否則完全保留原始 UUID。</param>
    COMPAT_NODISCARD std::unique_ptr<WidgetNode> Clone(bool generateNewUuids = false) const;

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace designer
} // namespace cppui
