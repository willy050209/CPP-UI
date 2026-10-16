/**
 * @file WidgetNode.cpp
 * @brief WidgetNode 控制項樹節點實作 (Impl 封裝與深常數性)
 */

#include "ui/designer/ast/WidgetNode.hpp"
#include "ui/designer/ast/CardinalityValidator.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace cppui {
namespace designer {

namespace {

std::string GenerateUuidV4() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

    uint32_t data[4];
    data[0] = dis(gen);
    data[1] = dis(gen);
    data[2] = dis(gen);
    data[3] = dis(gen);

    // 設定 UUID v4 版本與變體
    data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;
    data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;

    std::ostringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(8) << data[0] << "-"
       << std::setw(4) << ((data[1] >> 16) & 0xFFFF) << "-"
       << std::setw(4) << (data[1] & 0xFFFF) << "-"
       << std::setw(4) << ((data[2] >> 16) & 0xFFFF) << "-"
       << std::setw(4) << (data[2] & 0xFFFF)
       << std::setw(8) << data[3];
    return ss.str();
}

} // anonymous namespace

struct WidgetNode::Impl {
    NodeType m_type;
    std::string m_uuid;
    std::string m_name;
    WidgetNode* m_parent = nullptr;

    std::unordered_map<std::string, PropertyValue> m_properties;
    std::unordered_map<std::string, PropertyValue> m_attachedProperties;
    std::vector<std::unique_ptr<WidgetNode>> m_children;

    std::vector<layout::RowDefinition> m_rowDefinitions;
    std::vector<layout::ColumnDefinition> m_columnDefinitions;

    std::vector<EventBindingDescriptor> m_eventBindings;
    std::vector<DataBindingDescriptor> m_dataBindings;

    Impl(NodeType type, std::string uuid, std::string name)
        : m_type(type), m_uuid(std::move(uuid)), m_name(std::move(name)) {}
};

WidgetNode::WidgetNode(NodeType type, std::string name)
    : m_impl(std::make_unique<Impl>(type, GenerateUuidV4(), std::move(name))) {}

WidgetNode::WidgetNode(NodeType type, std::string uuid, std::string name)
    : m_impl(std::make_unique<Impl>(type, std::move(uuid), std::move(name))) {}

WidgetNode::~WidgetNode() = default;

WidgetNode::WidgetNode(WidgetNode&& other) noexcept = default;

WidgetNode& WidgetNode::operator=(WidgetNode&& other) noexcept = default;

NodeType WidgetNode::GetType() const noexcept {
    return m_impl->m_type;
}

const std::string& WidgetNode::GetUuid() const noexcept {
    return m_impl->m_uuid;
}

const std::string& WidgetNode::GetName() const noexcept {
    return m_impl->m_name;
}

void WidgetNode::SetName(std::string name) {
    m_impl->m_name = std::move(name);
}

ContainerCategory WidgetNode::GetCategory() const noexcept {
    return GetContainerCategory(m_impl->m_type);
}

void WidgetNode::SetProperty(std::string name, PropertyValue value) {
    m_impl->m_properties[std::move(name)] = std::move(value);
}

compat::optional<PropertyValue> WidgetNode::GetProperty(compat::string_view name) const {
    auto it = m_impl->m_properties.find(std::string(name));
    if (it != m_impl->m_properties.end()) {
        return it->second;
    }
    return compat::nullopt;
}

bool WidgetNode::RemoveProperty(compat::string_view name) {
    return m_impl->m_properties.erase(std::string(name)) > 0;
}

const std::unordered_map<std::string, PropertyValue>& WidgetNode::GetAllProperties() const noexcept {
    return m_impl->m_properties;
}

void WidgetNode::SetAttachedProperty(std::string name, PropertyValue value) {
    m_impl->m_attachedProperties[std::move(name)] = std::move(value);
}

compat::optional<PropertyValue> WidgetNode::GetAttachedProperty(compat::string_view name) const {
    auto it = m_impl->m_attachedProperties.find(std::string(name));
    if (it != m_impl->m_attachedProperties.end()) {
        return it->second;
    }
    return compat::nullopt;
}

bool WidgetNode::RemoveAttachedProperty(compat::string_view name) {
    return m_impl->m_attachedProperties.erase(std::string(name)) > 0;
}

const std::unordered_map<std::string, PropertyValue>& WidgetNode::GetAllAttachedProperties() const noexcept {
    return m_impl->m_attachedProperties;
}

WidgetNode* WidgetNode::GetParent() noexcept {
    return m_impl->m_parent;
}

const WidgetNode* WidgetNode::GetParent() const noexcept {
    return m_impl->m_parent;
}

void WidgetNode::SetParent(WidgetNode* parent) noexcept {
    m_impl->m_parent = parent;
}

size_t WidgetNode::GetChildCount() const noexcept {
    return m_impl->m_children.size();
}

WidgetNode* WidgetNode::GetChild(size_t index) noexcept {
    if (index >= m_impl->m_children.size()) {
        return nullptr;
    }
    return m_impl->m_children[index].get();
}

const WidgetNode* WidgetNode::GetChild(size_t index) const noexcept {
    if (index >= m_impl->m_children.size()) {
        return nullptr;
    }
    return m_impl->m_children[index].get();
}

const std::vector<std::unique_ptr<WidgetNode>>& WidgetNode::GetChildren() const noexcept {
    return m_impl->m_children;
}

compat::expected<void, DesignerError> WidgetNode::AddChild(std::unique_ptr<WidgetNode> child) {
    if (!child) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidNode;
        err.message = "Child pointer cannot be null";
        return compat::unexpected(std::move(err));
    }
    auto validation = CardinalityValidator::CanAddChild(*this, child->GetType());
    if (!validation.has_value()) {
        return validation;
    }
    child->SetParent(this);
    m_impl->m_children.push_back(std::move(child));
    return {};
}

compat::expected<void, DesignerError> WidgetNode::InsertChild(size_t index, std::unique_ptr<WidgetNode> child) {
    if (!child) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidNode;
        err.message = "Child pointer cannot be null";
        return compat::unexpected(std::move(err));
    }
    auto validation = CardinalityValidator::CanAddChild(*this, child->GetType());
    if (!validation.has_value()) {
        return validation;
    }
    if (index > m_impl->m_children.size()) {
        index = m_impl->m_children.size();
    }
    child->SetParent(this);
    m_impl->m_children.insert(m_impl->m_children.begin() + index, std::move(child));
    return {};
}

compat::expected<std::unique_ptr<WidgetNode>, DesignerError> WidgetNode::RemoveChild(size_t index) {
    if (index >= m_impl->m_children.size()) {
        DesignerError err;
        err.code = DesignerErrorCode::NodeNotFound;
        err.message = "Child index out of bounds during RemoveChild";
        return compat::unexpected(std::move(err));
    }
    auto node = std::move(m_impl->m_children[index]);
    node->SetParent(nullptr);
    m_impl->m_children.erase(m_impl->m_children.begin() + index);
    return std::move(node);
}

compat::expected<std::unique_ptr<WidgetNode>, DesignerError> WidgetNode::RemoveChildByUuid(compat::string_view uuid) {
    for (size_t i = 0; i < m_impl->m_children.size(); ++i) {
        if (m_impl->m_children[i]->GetUuid() == uuid) {
            return RemoveChild(i);
        }
    }
    DesignerError err;
    err.code = DesignerErrorCode::NodeNotFound;
    err.message = "Node with specified UUID not found in children";
    return compat::unexpected(std::move(err));
}

compat::optional<size_t> WidgetNode::IndexOfChild(const WidgetNode* child) const noexcept {
    if (!child) {
        return compat::nullopt;
    }
    for (size_t i = 0; i < m_impl->m_children.size(); ++i) {
        if (m_impl->m_children[i].get() == child) {
            return i;
        }
    }
    return compat::nullopt;
}

const std::vector<layout::RowDefinition>& WidgetNode::GetRowDefinitions() const noexcept {
    return m_impl->m_rowDefinitions;
}

void WidgetNode::SetRowDefinitions(std::vector<layout::RowDefinition> defs) {
    m_impl->m_rowDefinitions = std::move(defs);
}

const std::vector<layout::ColumnDefinition>& WidgetNode::GetColumnDefinitions() const noexcept {
    return m_impl->m_columnDefinitions;
}

void WidgetNode::SetColumnDefinitions(std::vector<layout::ColumnDefinition> defs) {
    m_impl->m_columnDefinitions = std::move(defs);
}

const std::vector<EventBindingDescriptor>& WidgetNode::GetEventBindings() const noexcept {
    return m_impl->m_eventBindings;
}

void WidgetNode::AddEventBinding(EventBindingDescriptor binding) {
    for (auto& b : m_impl->m_eventBindings) {
        if (b.eventName == binding.eventName) {
            b = std::move(binding);
            return;
        }
    }
    m_impl->m_eventBindings.push_back(std::move(binding));
}

bool WidgetNode::RemoveEventBinding(compat::string_view eventName) {
    for (auto it = m_impl->m_eventBindings.begin(); it != m_impl->m_eventBindings.end(); ++it) {
        if (it->eventName == eventName) {
            m_impl->m_eventBindings.erase(it);
            return true;
        }
    }
    return false;
}

const std::vector<DataBindingDescriptor>& WidgetNode::GetDataBindings() const noexcept {
    return m_impl->m_dataBindings;
}

void WidgetNode::AddDataBinding(DataBindingDescriptor binding) {
    for (auto& b : m_impl->m_dataBindings) {
        if (b.propertyName == binding.propertyName) {
            b = std::move(binding);
            return;
        }
    }
    m_impl->m_dataBindings.push_back(std::move(binding));
}

bool WidgetNode::RemoveDataBinding(compat::string_view propertyName) {
    for (auto it = m_impl->m_dataBindings.begin(); it != m_impl->m_dataBindings.end(); ++it) {
        if (it->propertyName == propertyName) {
            m_impl->m_dataBindings.erase(it);
            return true;
        }
    }
    return false;
}

WidgetNode* WidgetNode::FindByUuid(compat::string_view uuid) noexcept {
    if (m_impl->m_uuid == uuid) {
        return this;
    }
    for (auto& child : m_impl->m_children) {
        if (auto found = child->FindByUuid(uuid)) {
            return found;
        }
    }
    return nullptr;
}

const WidgetNode* WidgetNode::FindByUuid(compat::string_view uuid) const noexcept {
    if (m_impl->m_uuid == uuid) {
        return this;
    }
    for (const auto& child : m_impl->m_children) {
        if (const auto found = child->FindByUuid(uuid)) {
            return found;
        }
    }
    return nullptr;
}

WidgetNode* WidgetNode::FindByName(compat::string_view name) noexcept {
    if (m_impl->m_name == name) {
        return this;
    }
    for (auto& child : m_impl->m_children) {
        if (auto found = child->FindByName(name)) {
            return found;
        }
    }
    return nullptr;
}

const WidgetNode* WidgetNode::FindByName(compat::string_view name) const noexcept {
    if (m_impl->m_name == name) {
        return this;
    }
    for (const auto& child : m_impl->m_children) {
        if (const auto found = child->FindByName(name)) {
            return found;
        }
    }
    return nullptr;
}

std::unique_ptr<WidgetNode> WidgetNode::Clone(bool generateNewUuids) const {
    std::string newUuid = generateNewUuids ? GenerateUuidV4() : m_impl->m_uuid;
    auto clone = std::make_unique<WidgetNode>(m_impl->m_type, std::move(newUuid), m_impl->m_name);

    clone->m_impl->m_properties = m_impl->m_properties;
    clone->m_impl->m_attachedProperties = m_impl->m_attachedProperties;
    clone->m_impl->m_rowDefinitions = m_impl->m_rowDefinitions;
    clone->m_impl->m_columnDefinitions = m_impl->m_columnDefinitions;
    clone->m_impl->m_eventBindings = m_impl->m_eventBindings;
    clone->m_impl->m_dataBindings = m_impl->m_dataBindings;

    for (const auto& child : m_impl->m_children) {
        auto childClone = child->Clone(generateNewUuids);
        childClone->SetParent(clone.get());
        clone->m_impl->m_children.push_back(std::move(childClone));
    }

    return clone;
}

} // namespace designer
} // namespace cppui
