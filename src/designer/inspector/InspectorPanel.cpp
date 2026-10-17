/**
 * @file InspectorPanel.cpp
 * @brief CPPUI Designer 屬性與事件檢查器主控制項實作
 */

#include "ui/designer/inspector/InspectorPanel.hpp"
#include "ui/designer/inspector/InheritanceMetadataRegistry.hpp"
#include "ui/designer/commands/PropertyCommands.hpp"
#include "ui/designer/commands/MacroCommand.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>

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

static std::string FormatValueString(const PropertyValue& val) {
    if (val.IsEmpty()) return "";
    if (auto b = val.Get<bool>()) return *b ? "True" : "False";
    if (auto i = val.Get<int32_t>()) return std::to_string(*i);
    if (auto d = val.Get<double>()) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f", *d);
        return buf;
    }
    if (auto s = val.Get<std::string>()) return *s;
    if (auto c = val.Get<core::Color>()) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", c->r, c->g, c->b);
        return buf;
    }
    if (auto t = val.Get<core::Thickness>()) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.0f, %.0f, %.0f, %.0f", t->left, t->top, t->right, t->bottom);
        return buf;
    }
    return "...";
}

static void DrawRectOutline(rendering::DrawingContext& dc, const core::Rect& rect, core::Color color, float stroke = 1.0f) {
    dc.DrawLine(core::Point(rect.x, rect.y), core::Point(rect.x + rect.width, rect.y), color, stroke);
    dc.DrawLine(core::Point(rect.x + rect.width, rect.y), core::Point(rect.x + rect.width, rect.y + rect.height), color, stroke);
    dc.DrawLine(core::Point(rect.x + rect.width, rect.y + rect.height), core::Point(rect.x, rect.y + rect.height), color, stroke);
    dc.DrawLine(core::Point(rect.x, rect.y + rect.height), core::Point(rect.x, rect.y), color, stroke);
}

void InspectorPanel::OnRender(rendering::DrawingContext& dc) {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    const auto& colors = theme.Colors();

    float w = GetWidth();
    float h = GetHeight();
    if (w <= 0.0f) w = 310.0f;
    if (h <= 0.0f) h = 720.0f;

    // 面板背景與左側邊框線
    dc.DrawRectangle(core::Rect(0.0f, 0.0f, w, h), colors.surfaceBackground);
    dc.DrawLine(core::Point(0.0f, 0.0f), core::Point(0.0f, h), colors.surfaceBorder, 1.0f);

    // 1. 選取元件標頭 (Selected Component Header)
    core::Rect headerRect(0.0f, 0.0f, w, 38.0f);
    dc.DrawRectangle(headerRect, colors.controlBackground);
    dc.DrawLine(core::Point(0.0f, 38.0f), core::Point(w, 38.0f), colors.surfaceBorder, 1.0f);

    auto selectedNodes = m_impl->GetSelectedNodes();
    if (selectedNodes.empty()) {
        dc.DrawText("(未選取任何控制項 / No Selection)", core::Point(10.0f, 11.0f),
                    text::FontDescriptor("Segoe UI", 12.0f), colors.textDisabled);
    } else {
        WidgetNode* primary = selectedNodes[0];
        std::string headerTitle = "🎯 " + primary->GetName() + " : " + std::string(NodeTypeName(primary->GetType()));
        if (selectedNodes.size() > 1) {
            headerTitle += " (+" + std::to_string(selectedNodes.size() - 1) + " 選取中)";
        }
        dc.DrawText(headerTitle, core::Point(10.0f, 10.0f),
                    text::FontDescriptor("Segoe UI", 12.5f, text::FontWeight::Bold), colors.textPrimary);
    }

    // 2. 屬性 / 事件分頁列 (Tabs)
    const float tabY = 42.0f;
    const float tabH = 28.0f;
    const float tabW = (w - 16.0f) / 2.0f;

    core::Rect propTabRect(8.0f, tabY, tabW, tabH);
    core::Rect eventTabRect(8.0f + tabW, tabY, tabW, tabH);

    bool isPropTab = (m_impl->currentTab == InspectorTab::Properties);
    dc.DrawRoundedRectangle(propTabRect, 3.0f, isPropTab ? colors.controlHoverBackground : colors.controlBackground);
    dc.DrawRoundedRectangle(eventTabRect, 3.0f, !isPropTab ? colors.controlHoverBackground : colors.controlBackground);

    if (isPropTab) {
        dc.DrawLine(core::Point(propTabRect.x, tabY + tabH - 2.0f),
                    core::Point(propTabRect.x + tabW, tabY + tabH - 2.0f),
                    colors.accentPrimary, 2.0f);
    } else {
        dc.DrawLine(core::Point(eventTabRect.x, tabY + tabH - 2.0f),
                    core::Point(eventTabRect.x + tabW, tabY + tabH - 2.0f),
                    colors.accentPrimary, 2.0f);
    }

    dc.DrawText("屬性 (Properties)", core::Point(propTabRect.x + 18.0f, tabY + 6.0f),
                text::FontDescriptor("Segoe UI", 11.5f, isPropTab ? text::FontWeight::Bold : text::FontWeight::Normal),
                isPropTab ? colors.accentPrimary : colors.textSecondary);

    dc.DrawText("事件 (Events)", core::Point(eventTabRect.x + 24.0f, tabY + 6.0f),
                text::FontDescriptor("Segoe UI", 11.5f, !isPropTab ? text::FontWeight::Bold : text::FontWeight::Normal),
                !isPropTab ? colors.accentPrimary : colors.textSecondary);

    // 3. 搜尋篩選列 (Filter Box)
    const float filterY = 74.0f;
    core::Rect filterRect(8.0f, filterY, w - 16.0f, 24.0f);
    dc.DrawRoundedRectangle(filterRect, 3.0f, colors.controlBackground);
    DrawRectOutline(dc, filterRect, colors.controlBorder, 1.0f);

    std::string filterDisp = m_impl->filterText.empty() ? "🔍 搜尋屬性或事件 (Filter)..." : ("🔍 " + m_impl->filterText);
    core::Color filterClr = m_impl->filterText.empty() ? colors.textPlaceholder : colors.textPrimary;
    dc.DrawText(filterDisp, core::Point(14.0f, filterY + 4.0f),
                text::FontDescriptor("Segoe UI", 11.0f), filterClr);

    // 4. 主要網格內容渲染 (Grid Content)
    float curY = 104.0f;
    const float rowHeight = 26.0f;
    const float catHeight = 24.0f;
    const float col1Width = 125.0f;

    if (isPropTab) {
        auto props = GetInspectedProperties();

        // 依分類組織
        struct CatGroup {
            PropertyCategory cat;
            const char* title;
        };
        const CatGroup catGroups[] = {
            { PropertyCategory::Common,     "一般屬性 (Common)" },
            { PropertyCategory::Layout,     "版面與尺寸 (Layout)" },
            { PropertyCategory::Appearance, "外觀與色彩 (Appearance)" },
            { PropertyCategory::Text,       "文字排版 (Text)" },
            { PropertyCategory::Behavior,   "行為狀態 (Behavior)" }
        };

        for (const auto& group : catGroups) {
            std::vector<const InspectedProperty*> groupProps;
            for (const auto& p : props) {
                if (p.category == group.cat) {
                    groupProps.push_back(&p);
                }
            }

            if (groupProps.empty()) continue;

            // 分類標頭
            core::Rect catRect(4.0f, curY, w - 8.0f, catHeight);
            dc.DrawRoundedRectangle(catRect, 2.0f, colors.controlBackground);
            std::string catTitle = std::string("▼ ") + group.title + " (" + std::to_string(groupProps.size()) + ")";
            dc.DrawText(catTitle, core::Point(8.0f, curY + 4.0f),
                        text::FontDescriptor("Segoe UI", 11.0f, text::FontWeight::Bold),
                        colors.textPrimary);
            curY += catHeight + 2.0f;

            // 逐筆屬性列
            for (const auto* p : groupProps) {
                if (curY + rowHeight > h) break;

                // 左欄：屬性名稱
                core::Rect nameRect(8.0f, curY, col1Width, rowHeight);
                std::string propName = p->name;
                if (p->isAttached) {
                    propName += " *";
                }
                dc.DrawText(propName, core::Point(nameRect.x + 2.0f, curY + 5.0f),
                            text::FontDescriptor("Segoe UI", 11.5f),
                            p->isAttached ? core::Color(78, 160, 220) : colors.textPrimary);

                // 右欄：屬性值方塊
                core::Rect valRect(col1Width + 12.0f, curY + 2.0f, w - col1Width - 20.0f, rowHeight - 4.0f);
                dc.DrawRoundedRectangle(valRect, 3.0f, colors.controlBackground);
                DrawRectOutline(dc, valRect, colors.controlBorder, 1.0f);

                if (p->IsMixed()) {
                    dc.DrawText("<Mixed Values>", core::Point(valRect.x + 6.0f, valRect.y + 3.0f),
                                text::FontDescriptor("Segoe UI", 11.0f), colors.textDisabled);
                } else if (p->currentValue.Is<core::Color>()) {
                    auto c = p->currentValue.Get<core::Color>();
                    if (c.has_value()) {
                        // 顏色預覽色塊
                        core::Rect swatch(valRect.x + 4.0f, valRect.y + 3.0f, 14.0f, 14.0f);
                        dc.DrawRectangle(swatch, *c);
                        DrawRectOutline(dc, swatch, colors.controlBorder, 1.0f);

                        char hexBuf[16];
                        std::snprintf(hexBuf, sizeof(hexBuf), "#%02X%02X%02X", c->r, c->g, c->b);
                        dc.DrawText(hexBuf, core::Point(valRect.x + 24.0f, valRect.y + 3.0f),
                                    text::FontDescriptor("Segoe UI", 11.0f), colors.textPrimary);
                    }
                } else if (p->currentValue.Is<bool>()) {
                    auto b = p->currentValue.Get<bool>();
                    bool val = b.has_value() && *b;
                    core::Rect checkSquare(valRect.x + 6.0f, valRect.y + 4.0f, 12.0f, 12.0f);
                    DrawRectOutline(dc, checkSquare, colors.controlBorder, 1.0f);
                    if (val) {
                        dc.DrawRoundedRectangle(core::Rect(checkSquare.x + 2.0f, checkSquare.y + 2.0f, 8.0f, 8.0f),
                                                1.0f, colors.accentPrimary);
                    }
                    dc.DrawText(val ? "True" : "False", core::Point(valRect.x + 24.0f, valRect.y + 3.0f),
                                text::FontDescriptor("Segoe UI", 11.0f), colors.textPrimary);
                } else {
                    std::string valStr = FormatValueString(p->currentValue);
                    dc.DrawText(valStr, core::Point(valRect.x + 6.0f, valRect.y + 3.0f),
                                text::FontDescriptor("Segoe UI", 11.0f), colors.textPrimary);
                }

                curY += rowHeight;
            }
            curY += 4.0f;
        }
    } else {
        // 事件分頁
        auto events = GetInspectedEvents();
        core::Rect catRect(4.0f, curY, w - 8.0f, catHeight);
        dc.DrawRoundedRectangle(catRect, 2.0f, colors.controlBackground);
        std::string catTitle = "▼ 控制項事件綁定 (Events & Commands - " + std::to_string(events.size()) + ")";
        dc.DrawText(catTitle, core::Point(8.0f, curY + 4.0f),
                    text::FontDescriptor("Segoe UI", 11.0f, text::FontWeight::Bold),
                    colors.textPrimary);
        curY += catHeight + 2.0f;

        for (const auto& evt : events) {
            if (curY + rowHeight > h) break;

            // 事件名稱
            dc.DrawText(evt.descriptor.eventName, core::Point(10.0f, curY + 5.0f),
                        text::FontDescriptor("Segoe UI", 11.5f), colors.textPrimary);

            // 處理常式名稱方塊
            core::Rect valRect(col1Width + 12.0f, curY + 2.0f, w - col1Width - 20.0f, rowHeight - 4.0f);
            dc.DrawRoundedRectangle(valRect, 3.0f, colors.controlBackground);
            DrawRectOutline(dc, valRect, colors.controlBorder, 1.0f);

            if (evt.isBound) {
                dc.DrawText("⚡ " + evt.boundHandlerOrCommand, core::Point(valRect.x + 6.0f, valRect.y + 3.0f),
                            text::FontDescriptor("Segoe UI", 11.0f, text::FontWeight::Bold),
                            core::Color(30, 160, 90));
            } else {
                dc.DrawText("[+ 點擊建立常式]", core::Point(valRect.x + 6.0f, valRect.y + 3.0f),
                            text::FontDescriptor("Segoe UI", 10.5f),
                            colors.accentPrimary);
            }

            curY += rowHeight;
        }
    }
}

void InspectorPanel::OnPointerPressedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);
    float w = GetWidth();
    if (w <= 0.0f) w = 310.0f;

    // 1. 分頁切換
    const float tabY = 42.0f;
    const float tabH = 28.0f;
    const float tabW = (w - 16.0f) / 2.0f;

    core::Rect propTabRect(8.0f, tabY, tabW, tabH);
    core::Rect eventTabRect(8.0f + tabW, tabY, tabW, tabH);

    if (propTabRect.Contains(pt)) {
        SetCurrentTab(InspectorTab::Properties);
        InvalidateVisual();
        e.SetHandled(true);
        return;
    }
    if (eventTabRect.Contains(pt)) {
        SetCurrentTab(InspectorTab::Events);
        InvalidateVisual();
        e.SetHandled(true);
        return;
    }

    // 2. 點擊事件自動綁定
    if (m_impl->currentTab == InspectorTab::Events) {
        auto events = GetInspectedEvents();
        float curY = 104.0f + 26.0f; // 略過標頭
        const float rowHeight = 26.0f;
        const float col1Width = 125.0f;

        for (const auto& evt : events) {
            core::Rect valRect(col1Width + 12.0f, curY + 2.0f, w - col1Width - 20.0f, rowHeight - 4.0f);
            if (valRect.Contains(pt)) {
                if (!evt.isBound) {
                    AutoBindEvent(evt.descriptor.eventName);
                    InvalidateVisual();
                }
                e.SetHandled(true);
                return;
            }
            curY += rowHeight;
        }
    }
}

} // namespace inspector
} // namespace designer
} // namespace cppui
