/**
 * @file InspectorPanel.cpp
 * @brief CPPUI Designer 屬性與事件檢查器主控制項實作
 */

#include "ui/designer/inspector/InspectorPanel.hpp"
#include "ui/designer/inspector/InheritanceMetadataRegistry.hpp"
#include "ui/designer/commands/PropertyCommands.hpp"
#include "ui/designer/commands/MacroCommand.hpp"
#include <algorithm>
#include <cctype>

namespace cppui {
namespace designer {
namespace inspector {

namespace {

std::string ToLower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

bool ContainsIgnoreCase(compat::string_view haystack, compat::string_view needle) {
    if (needle.empty()) return true;
    std::string h = ToLower(std::string(haystack));
    std::string n = ToLower(std::string(needle));
    return h.find(n) != std::string::npos;
}

} // anonymous namespace

struct InspectorPanel::Impl {
    FormDocumentNode* document = nullptr;
    CommandHistoryManager* historyManager = nullptr;
    surface::SelectionModel* selectionModel = nullptr;

    InspectorTab currentTab = InspectorTab::Properties;
    std::string filterText;

    DesignerEvent<PropertyChangePayload> propertyChangedSignal;
    DesignerEvent<EventChangePayload> eventChangedSignal;
    DesignerEvent<InspectorTab> tabChangedSignal;

    std::vector<WidgetNode*> GetSelectedNodes() const {
        if (!document || !selectionModel) {
            return {};
        }
        std::vector<WidgetNode*> nodes;
        for (const auto& uuid : selectionModel->GetSelectedUuids()) {
            if (auto* node = document->FindByUuid(uuid)) {
                nodes.push_back(node);
            }
        }
        return nodes;
    }
};

InspectorPanel::InspectorPanel(
    FormDocumentNode* document,
    CommandHistoryManager* historyManager,
    surface::SelectionModel* selectionModel
) : m_impl(std::make_unique<Impl>()) {
    m_impl->document = document;
    m_impl->historyManager = historyManager;
    m_impl->selectionModel = selectionModel;
}

InspectorPanel::~InspectorPanel() = default;

void InspectorPanel::SetDocument(FormDocumentNode* document) {
    m_impl->document = document;
}

FormDocumentNode* InspectorPanel::GetDocument() const noexcept {
    return m_impl->document;
}

void InspectorPanel::SetHistoryManager(CommandHistoryManager* historyManager) {
    m_impl->historyManager = historyManager;
}

CommandHistoryManager* InspectorPanel::GetHistoryManager() const noexcept {
    return m_impl->historyManager;
}

void InspectorPanel::SetSelectionModel(surface::SelectionModel* selectionModel) {
    m_impl->selectionModel = selectionModel;
}

surface::SelectionModel* InspectorPanel::GetSelectionModel() const noexcept {
    return m_impl->selectionModel;
}

void InspectorPanel::SetCurrentTab(InspectorTab tab) {
    if (m_impl->currentTab != tab) {
        m_impl->currentTab = tab;
        m_impl->tabChangedSignal.Notify(tab);
    }
}

InspectorTab InspectorPanel::GetCurrentTab() const noexcept {
    return m_impl->currentTab;
}

void InspectorPanel::SetFilterText(compat::string_view filter) {
    m_impl->filterText = std::string(filter);
}

const std::string& InspectorPanel::GetFilterText() const noexcept {
    return m_impl->filterText;
}

std::vector<InspectedProperty> InspectorPanel::GetInspectedProperties() const {
    if (!m_impl->document) {
        return {};
    }

    auto selectedNodes = m_impl->GetSelectedNodes();
    std::vector<InspectedProperty> results;

    // 1. 若尚未選取任何節點，呈現 FormDocument 根視窗屬性
    if (selectedNodes.empty()) {
        const auto& docDescriptors = InheritanceMetadataRegistry::GetDescriptorsForType(NodeType::FormDocument);
        for (const auto& desc : docDescriptors) {
            if (!m_impl->filterText.empty() && !ContainsIgnoreCase(desc.name, m_impl->filterText)) {
                continue;
            }
            InspectedProperty p;
            p.name = desc.name;
            p.displayName = desc.name;
            p.category = desc.category;
            p.typeName = InheritanceMetadataRegistry::GetFriendlyTypeName(desc.defaultValue);
            p.defaultValue = desc.defaultValue;
            p.isAttached = false;
            p.isReadOnly = desc.isReadOnly;
            p.valueState = MultiValueState::SingleValue;

            if (desc.name == "Title") {
                p.currentValue = PropertyValue(m_impl->document->GetSettings().title);
            } else if (desc.name == "Width") {
                p.currentValue = PropertyValue(static_cast<double>(m_impl->document->GetSettings().initialSize.width));
            } else if (desc.name == "Height") {
                p.currentValue = PropertyValue(static_cast<double>(m_impl->document->GetSettings().initialSize.height));
            } else {
                p.currentValue = desc.defaultValue;
            }

            results.push_back(std::move(p));
        }
        return results;
    }

    // 2. 收集所有選取節點之型別
    std::vector<NodeType> nodeTypes;
    nodeTypes.reserve(selectedNodes.size());
    for (const auto* n : selectedNodes) {
        nodeTypes.push_back(n->GetType());
    }

    // 3. 計算交集屬性
    std::vector<PropertyDescriptor> commonProps = InheritanceMetadataRegistry::ComputePropertyIntersection(nodeTypes);

    // 4. 動態感知父容器附加屬性 (Context-Aware Attached Properties)
    compat::optional<NodeType> commonParentType;
    bool allSameParentType = true;
    for (size_t i = 0; i < selectedNodes.size(); ++i) {
        const WidgetNode* parent = selectedNodes[i]->GetParent();
        compat::optional<NodeType> pType = parent ? compat::make_optional(parent->GetType()) : compat::nullopt;
        if (i == 0) {
            commonParentType = pType;
        } else if (commonParentType != pType) {
            allSameParentType = false;
            break;
        }
    }

    std::vector<PropertyDescriptor> attachedProps;
    if (allSameParentType) {
        attachedProps = InheritanceMetadataRegistry::GetAttachedPropertiesForParent(commonParentType);
    } else {
        attachedProps = InheritanceMetadataRegistry::GetAttachedPropertiesForParent(compat::nullopt);
    }

    // 5. 組合附加屬性 (置頂) 與固有屬性
    std::vector<std::pair<PropertyDescriptor, bool>> allDescriptors;
    for (auto&& ap : attachedProps) {
        allDescriptors.push_back({ std::move(ap), true });
    }
    for (auto&& cp : commonProps) {
        allDescriptors.push_back({ std::move(cp), false });
    }

    // 6. 遍歷並判斷多選數值一致性 (SingleValue vs Mixed)
    for (const auto& pair : allDescriptors) {
        const auto& desc = pair.first;
        bool isAttached = pair.second;

        if (!m_impl->filterText.empty() && !ContainsIgnoreCase(desc.name, m_impl->filterText)) {
            continue;
        }

        InspectedProperty ip;
        ip.name = desc.name;
        ip.displayName = desc.name;
        ip.category = desc.category;
        ip.typeName = InheritanceMetadataRegistry::GetFriendlyTypeName(desc.defaultValue);
        ip.defaultValue = desc.defaultValue;
        ip.isAttached = isAttached;
        ip.isReadOnly = desc.isReadOnly;
        ip.enumOptions = InheritanceMetadataRegistry::GetEnumOptions(desc.name);
        ip.themeTokenSuggestions = InheritanceMetadataRegistry::GetThemeTokenSuggestions(desc.name);

        // 取得第一個節點的數值
        PropertyValue firstVal;
        if (isAttached) {
            firstVal = selectedNodes[0]->GetAttachedProperty(desc.name).value_or(desc.defaultValue);
        } else {
            firstVal = selectedNodes[0]->GetProperty(desc.name).value_or(desc.defaultValue);
        }

        bool isMixed = false;
        for (size_t i = 1; i < selectedNodes.size(); ++i) {
            PropertyValue val;
            if (isAttached) {
                val = selectedNodes[i]->GetAttachedProperty(desc.name).value_or(desc.defaultValue);
            } else {
                val = selectedNodes[i]->GetProperty(desc.name).value_or(desc.defaultValue);
            }
            if (val != firstVal) {
                isMixed = true;
                break;
            }
        }

        ip.currentValue = firstVal;
        ip.valueState = isMixed ? MultiValueState::Mixed : MultiValueState::SingleValue;

        results.push_back(std::move(ip));
    }

    return results;
}

std::vector<EventBindingState> InspectorPanel::GetInspectedEvents() const {
    if (!m_impl->document) {
        return {};
    }

    auto selectedNodes = m_impl->GetSelectedNodes();
    if (selectedNodes.empty()) {
        return {};
    }

    std::vector<NodeType> nodeTypes;
    for (const auto* n : selectedNodes) {
        nodeTypes.push_back(n->GetType());
    }

    auto commonEvents = InheritanceMetadataRegistry::ComputeEventIntersection(nodeTypes);
    std::vector<EventBindingState> results;

    for (const auto& evDesc : commonEvents) {
        if (!m_impl->filterText.empty() && !ContainsIgnoreCase(evDesc.eventName, m_impl->filterText)) {
            continue;
        }

        EventBindingState state;
        state.descriptor = evDesc;
        state.isMvvmCommand = evDesc.isCommand;

        // 檢查第一個節點是否有綁定
        const auto& bindings = selectedNodes[0]->GetEventBindings();
        for (const auto& b : bindings) {
            if (b.eventName == evDesc.eventName) {
                state.boundHandlerOrCommand = b.handlerName;
                state.isBound = true;
                break;
            }
        }

        if (!state.isBound && evDesc.isCommand) {
            const auto& dataBindings = selectedNodes[0]->GetDataBindings();
            for (const auto& db : dataBindings) {
                if (db.propertyName == evDesc.eventName) {
                    state.boundHandlerOrCommand = db.sourcePath;
                    state.isBound = true;
                    state.isMvvmCommand = true;
                    break;
                }
            }
        }

        results.push_back(std::move(state));
    }

    return results;
}

compat::optional<InspectedProperty> InspectorPanel::FindProperty(compat::string_view name) const {
    auto props = GetInspectedProperties();
    for (const auto& p : props) {
        if (p.name == name) {
            return p;
        }
    }
    return compat::nullopt;
}

bool InspectorPanel::ChangeProperty(compat::string_view propName, const PropertyValue& newValue) {
    if (!m_impl->document) {
        return false;
    }

    auto selectedNodes = m_impl->GetSelectedNodes();

    // 1. 若無選取節點，修改 FormDocument 根視窗屬性
    if (selectedNodes.empty()) {
        if (propName == "Title" && newValue.Is<std::string>()) {
            auto s = m_impl->document->GetSettings();
            s.title = newValue.Get<std::string>().value();
            m_impl->document->SetSettings(s);
            m_impl->propertyChangedSignal.Notify({ std::string(propName), newValue });
            return true;
        }
        if (propName == "Width") {
            double w = 0.0;
            if (newValue.Is<double>()) w = newValue.Get<double>().value();
            else if (newValue.Is<int32_t>()) w = newValue.Get<int32_t>().value();
            else return false;

            if (w < 0.0) return false;
            auto s = m_impl->document->GetSettings();
            s.initialSize.width = static_cast<float>(w);
            m_impl->document->SetSettings(s);
            m_impl->propertyChangedSignal.Notify({ std::string(propName), newValue });
            return true;
        }
        if (propName == "Height") {
            double h = 0.0;
            if (newValue.Is<double>()) h = newValue.Get<double>().value();
            else if (newValue.Is<int32_t>()) h = newValue.Get<int32_t>().value();
            else return false;

            if (h < 0.0) return false;
            auto s = m_impl->document->GetSettings();
            s.initialSize.height = static_cast<float>(h);
            m_impl->document->SetSettings(s);
            m_impl->propertyChangedSignal.Notify({ std::string(propName), newValue });
            return true;
        }
        return false;
    }

    // 2. 單選節點模式
    if (selectedNodes.size() == 1) {
        auto cmd = std::make_unique<SetPropertyCommand>(
            selectedNodes[0]->GetUuid(),
            std::string(propName),
            newValue
        );
        if (m_impl->historyManager) {
            auto res = m_impl->historyManager->Execute(*m_impl->document, std::move(cmd));
            if (!res.has_value()) {
                return false;
            }
        } else {
            selectedNodes[0]->SetProperty(std::string(propName), newValue);
        }
        m_impl->propertyChangedSignal.Notify({ std::string(propName), newValue });
        return true;
    }

    // 3. 多選節點批次模式 (打包為單一 MacroCommand)
    auto macro = std::make_unique<MacroCommand>("Change Property " + std::string(propName));
    for (auto* node : selectedNodes) {
        macro->AddCommand(std::make_unique<SetPropertyCommand>(
            node->GetUuid(),
            std::string(propName),
            newValue
        ));
    }

    if (m_impl->historyManager) {
        auto res = m_impl->historyManager->Execute(*m_impl->document, std::move(macro));
        if (!res.has_value()) {
            return false;
        }
    } else {
        for (auto* node : selectedNodes) {
            node->SetProperty(std::string(propName), newValue);
        }
    }

    m_impl->propertyChangedSignal.Notify({ std::string(propName), newValue });
    return true;
}

bool InspectorPanel::ChangeAttachedProperty(compat::string_view propName, const PropertyValue& newValue) {
    if (!m_impl->document) {
        return false;
    }

    auto selectedNodes = m_impl->GetSelectedNodes();
    if (selectedNodes.empty()) {
        return false;
    }

    if (selectedNodes.size() == 1) {
        auto cmd = std::make_unique<SetAttachedPropertyCommand>(
            selectedNodes[0]->GetUuid(),
            std::string(propName),
            newValue
        );
        if (m_impl->historyManager) {
            auto res = m_impl->historyManager->Execute(*m_impl->document, std::move(cmd));
            if (!res.has_value()) {
                return false;
            }
        } else {
            selectedNodes[0]->SetAttachedProperty(std::string(propName), newValue);
        }
        m_impl->propertyChangedSignal.Notify({ std::string(propName), newValue });
        return true;
    }

    auto macro = std::make_unique<MacroCommand>("Change Attached Property " + std::string(propName));
    for (auto* node : selectedNodes) {
        macro->AddCommand(std::make_unique<SetAttachedPropertyCommand>(
            node->GetUuid(),
            std::string(propName),
            newValue
        ));
    }

    if (m_impl->historyManager) {
        auto res = m_impl->historyManager->Execute(*m_impl->document, std::move(macro));
        if (!res.has_value()) {
            return false;
        }
    } else {
        for (auto* node : selectedNodes) {
            node->SetAttachedProperty(std::string(propName), newValue);
        }
    }

    m_impl->propertyChangedSignal.Notify({ std::string(propName), newValue });
    return true;
}

bool InspectorPanel::ResetPropertyToDefault(compat::string_view propName) {
    auto propOpt = FindProperty(propName);
    if (!propOpt.has_value()) {
        return false;
    }

    if (propOpt->isAttached) {
        return ChangeAttachedProperty(propName, propOpt->defaultValue);
    }
    return ChangeProperty(propName, propOpt->defaultValue);
}

bool InspectorPanel::BindEvent(compat::string_view eventName, compat::string_view handlerName) {
    if (!m_impl->document) {
        return false;
    }

    auto selectedNodes = m_impl->GetSelectedNodes();
    if (selectedNodes.empty()) {
        return false;
    }

    for (auto* node : selectedNodes) {
        node->RemoveEventBinding(eventName);
        node->AddEventBinding(EventBindingDescriptor{
            std::string(eventName),
            std::string(handlerName)
        });
    }

    m_impl->eventChangedSignal.Notify({ std::string(eventName), std::string(handlerName) });
    return true;
}

bool InspectorPanel::UnbindEvent(compat::string_view eventName) {
    if (!m_impl->document) {
        return false;
    }

    auto selectedNodes = m_impl->GetSelectedNodes();
    if (selectedNodes.empty()) {
        return false;
    }

    for (auto* node : selectedNodes) {
        node->RemoveEventBinding(eventName);
    }

    m_impl->eventChangedSignal.Notify({ std::string(eventName), "" });
    return true;
}

std::string InspectorPanel::AutoBindEvent(compat::string_view eventName) {
    auto selectedNodes = m_impl->GetSelectedNodes();
    if (selectedNodes.empty()) {
        return "";
    }

    std::string handlerName = InheritanceMetadataRegistry::GenerateDefaultHandlerName(
        selectedNodes[0]->GetName(),
        eventName
    );

    BindEvent(eventName, handlerName);
    return handlerName;
}

DesignerEvent<PropertyChangePayload>& InspectorPanel::PropertyChangedSignal() noexcept {
    return m_impl->propertyChangedSignal;
}

DesignerEvent<EventChangePayload>& InspectorPanel::EventChangedSignal() noexcept {
    return m_impl->eventChangedSignal;
}

DesignerEvent<InspectorTab>& InspectorPanel::TabChangedSignal() noexcept {
    return m_impl->tabChangedSignal;
}

void InspectorPanel::OnRender(rendering::DrawingContext& dc) {
    controls::Control::OnRender(dc);
}

} // namespace inspector
} // namespace designer
} // namespace cppui
