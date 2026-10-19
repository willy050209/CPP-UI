/**
 * @file FormDocumentNode.cpp
 * @brief FormDocumentNode 實作與反應式信號封裝
 */

#include "ui/designer/ast/FormDocumentNode.hpp"

namespace cppui {
namespace designer {

struct FormDocumentNode::Impl {
    FormSettings m_settings;
    std::unique_ptr<WidgetNode> m_root;

    DesignerEvent<NodeAddedPayload> m_nodeAdded;
    DesignerEvent<NodeRemovedPayload> m_nodeRemoved;
    DesignerEvent<NodeMovedPayload> m_nodeMoved;
    DesignerEvent<PropertyChangedPayload> m_propertyChanged;
    DesignerEvent<FormSettingsChangedPayload> m_settingsChanged;

    explicit Impl(FormSettings settings) : m_settings(std::move(settings)) {}
};

FormDocumentNode::FormDocumentNode()
    : m_impl(std::make_unique<Impl>(FormSettings{})) {}

FormDocumentNode::FormDocumentNode(FormSettings settings)
    : m_impl(std::make_unique<Impl>(std::move(settings))) {}

FormDocumentNode::~FormDocumentNode() = default;

FormDocumentNode::FormDocumentNode(FormDocumentNode&& other) noexcept = default;

FormDocumentNode& FormDocumentNode::operator=(FormDocumentNode&& other) noexcept = default;

const FormSettings& FormDocumentNode::GetSettings() const noexcept {
    return m_impl->m_settings;
}

void FormDocumentNode::SetSettings(FormSettings settings) {
    if (m_impl->m_settings != settings) {
        FormSettings old = m_impl->m_settings;
        m_impl->m_settings = settings;
        m_impl->m_settingsChanged.Notify({ old, std::move(settings) });
    }
}

WidgetNode* FormDocumentNode::GetRoot() noexcept {
    return m_impl->m_root.get();
}

const WidgetNode* FormDocumentNode::GetRoot() const noexcept {
    return m_impl->m_root.get();
}

void FormDocumentNode::SetRoot(std::unique_ptr<WidgetNode> root) {
    if (root) {
        root->SetParent(nullptr);
    }
    m_impl->m_root = std::move(root);
}

std::unique_ptr<WidgetNode> FormDocumentNode::TakeRoot() noexcept {
    return std::move(m_impl->m_root);
}

WidgetNode* FormDocumentNode::FindByUuid(compat::string_view uuid) noexcept {
    if (m_impl->m_root) {
        return m_impl->m_root->FindByUuid(uuid);
    }
    return nullptr;
}

const WidgetNode* FormDocumentNode::FindByUuid(compat::string_view uuid) const noexcept {
    if (m_impl->m_root) {
        return m_impl->m_root->FindByUuid(uuid);
    }
    return nullptr;
}

WidgetNode* FormDocumentNode::FindByName(compat::string_view name) noexcept {
    if (m_impl->m_root) {
        return m_impl->m_root->FindByName(name);
    }
    return nullptr;
}

const WidgetNode* FormDocumentNode::FindByName(compat::string_view name) const noexcept {
    if (m_impl->m_root) {
        return m_impl->m_root->FindByName(name);
    }
    return nullptr;
}

DesignerEvent<NodeAddedPayload>& FormDocumentNode::NodeAddedSignal() noexcept {
    return m_impl->m_nodeAdded;
}

DesignerEvent<NodeRemovedPayload>& FormDocumentNode::NodeRemovedSignal() noexcept {
    return m_impl->m_nodeRemoved;
}

DesignerEvent<NodeMovedPayload>& FormDocumentNode::NodeMovedSignal() noexcept {
    return m_impl->m_nodeMoved;
}

DesignerEvent<PropertyChangedPayload>& FormDocumentNode::PropertyChangedSignal() noexcept {
    return m_impl->m_propertyChanged;
}

DesignerEvent<FormSettingsChangedPayload>& FormDocumentNode::SettingsChangedSignal() noexcept {
    return m_impl->m_settingsChanged;
}

std::unique_ptr<FormDocumentNode> FormDocumentNode::Clone(bool generateNewUuids) const {
    auto cloneDoc = std::make_unique<FormDocumentNode>(m_impl->m_settings);
    if (m_impl->m_root) {
        cloneDoc->SetRoot(m_impl->m_root->Clone(generateNewUuids));
    }
    return cloneDoc;
}

} // namespace designer
} // namespace cppui
