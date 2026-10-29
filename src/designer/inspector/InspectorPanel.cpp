/**
 * @file InspectorPanel.cpp
 * @brief CPPUI Designer 屬性與事件檢查器主控制項實作
 */

#include "ui/designer/inspector/InspectorPanel.hpp"
#include "ui/core/LifetimeToken.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/designer/inspector/InheritanceMetadataRegistry.hpp"
#include "ui/designer/inspector/PropertyEditors.hpp"
#include "ui/designer/commands/PropertyCommands.hpp"
#include "ui/designer/commands/MacroCommand.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/input/FocusManager.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
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
    InspectorPanel* owner = nullptr;
    FormDocumentNode* document = nullptr;
    CommandHistoryManager* historyManager = nullptr;
    surface::SelectionModel* selectionModel = nullptr;

    InspectorTab currentTab = InspectorTab::Properties;
    std::string filterText;
    float scrollY = 0.0f;
    float fontScale = 1.0f;

    // 搜尋與行內文字編輯器控制項實例
    std::shared_ptr<controls::TextBox> searchTextBox;
    std::shared_ptr<controls::TextBox> inlineEditor;

    std::string editingPropName;
    bool isEditingAttached = false;
    core::Rect editingRect;

    // 列舉浮動下拉選單狀態
    struct ActiveDropdown {
        std::string propName;
        bool isAttached = false;
        core::Rect popupRect;
        std::vector<std::string> options;
    };
    compat::optional<ActiveDropdown> activeDropdown;

    void StartInlineEdit(const std::string& name, bool isAttached, const std::string& initialText, const core::Rect& rect);
    void CommitInlineEdit();
    void CancelInlineEdit() {
        editingPropName.clear();
        if (inlineEditor) {
            inlineEditor->SetVisible(false);
        }
        activeDropdown = compat::nullopt;
        if (owner) {
            owner->InvalidateVisual();
        }
    }

    DesignerEvent<PropertyChangePayload> propertyChangedSignal;
    DesignerEvent<EventChangePayload> eventChangedSignal;
    DesignerEvent<InspectorTab> tabChangedSignal;
    reactive::ScopedConnection selectionConn;

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
    m_impl->owner = this;
    m_impl->document = document;
    m_impl->historyManager = historyManager;
    m_impl->selectionModel = selectionModel;
    if (m_impl->selectionModel) {
        m_impl->selectionConn = m_impl->selectionModel->SelectionChangedSignal().Connect(GetLifetimeToken(), [this](const std::vector<std::string>&) {
            m_impl->scrollY = 0.0f;
            m_impl->CancelInlineEdit();
            InvalidateVisual();
        });
    }

    // 建立搜尋篩選文字框 (Real TextBox)
    m_impl->searchTextBox = std::make_shared<controls::TextBox>();
    m_impl->searchTextBox->SetPlaceholder("🔍 搜尋屬性或事件 (Filter)...");
    m_impl->searchTextBox->SetFontSize(12.0f);
    m_impl->searchTextBox->SetCornerRadius(3.0f);
    m_impl->searchTextBox->SetPadding(core::Thickness(6.0f, 3.0f, 6.0f, 3.0f));
    m_impl->searchTextBox->OnTextChanged([this](compat::string_view text) {
        if (m_impl->filterText != text) {
            m_impl->filterText = std::string(text);
            m_impl->scrollY = 0.0f;
            InvalidateVisual();
        }
    });
    AddVisualChild(m_impl->searchTextBox);

    // 建立行內屬性設定文字框 (Real TextBox with Selection, Caret & Clipboard support)
    m_impl->inlineEditor = std::make_shared<controls::TextBox>();
    m_impl->inlineEditor->SetVisible(false);
    m_impl->inlineEditor->SetFontSize(11.5f);
    m_impl->inlineEditor->SetCornerRadius(3.0f);
    m_impl->inlineEditor->SetPadding(core::Thickness(4.0f, 2.0f, 4.0f, 2.0f));
    m_impl->inlineEditor->OnKeyDown([this](input::KeyEventArgs& e) {
        auto k = e.GetKey();
        if (k == input::Key::Enter || k == input::Key::Return) {
            m_impl->CommitInlineEdit();
            e.SetHandled(true);
        } else if (k == input::Key::Escape) {
            m_impl->CancelInlineEdit();
            e.SetHandled(true);
        }
    });
    m_impl->inlineEditor->OnLostFocus([this](input::FocusEventArgs& /*e*/) {
        m_impl->CommitInlineEdit();
    });
    AddVisualChild(m_impl->inlineEditor);

    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    m_impl->searchTextBox->ApplyTheme(theme);
    m_impl->inlineEditor->ApplyTheme(theme);

    SetClipToBounds(true);
    SetFocusable(true);
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
    if (m_impl->selectionModel) {
        m_impl->selectionConn = m_impl->selectionModel->SelectionChangedSignal().Connect(GetLifetimeToken(), [this](const std::vector<std::string>&) {
            m_impl->scrollY = 0.0f;
            m_impl->CancelInlineEdit();
            InvalidateVisual();
        });
    }
}

surface::SelectionModel* InspectorPanel::GetSelectionModel() const noexcept {
    return m_impl->selectionModel;
}

void InspectorPanel::SetCurrentTab(InspectorTab tab) {
    if (m_impl->currentTab != tab) {
        m_impl->CommitInlineEdit();
        m_impl->currentTab = tab;
        m_impl->scrollY = 0.0f;
        m_impl->tabChangedSignal.Notify(tab);
    }
}

InspectorTab InspectorPanel::GetCurrentTab() const noexcept {
    return m_impl->currentTab;
}

void InspectorPanel::SetFilterText(compat::string_view filter) {
    m_impl->filterText = std::string(filter);
    if (m_impl->searchTextBox && m_impl->searchTextBox->GetText() != filter) {
        m_impl->searchTextBox->SetText(std::string(filter));
    }
    m_impl->scrollY = 0.0f;
    InvalidateVisual();
}

const std::string& InspectorPanel::GetFilterText() const noexcept {
    return m_impl->filterText;
}

void InspectorPanel::SetFontScale(float scale) {
    m_impl->fontScale = compat::clamp(scale, 0.5f, 5.0f);
    if (m_impl->searchTextBox) {
        m_impl->searchTextBox->SetFontSize(12.0f * m_impl->fontScale);
    }
    if (m_impl->inlineEditor) {
        m_impl->inlineEditor->SetFontSize(11.5f * m_impl->fontScale);
    }
    InvalidateVisual();
}

float InspectorPanel::GetFontScale() const noexcept {
    return m_impl->fontScale;
}

core::Size InspectorPanel::MeasureOverride(core::Size availableSize) {
    float w = availableSize.width;
    if (std::isinf(w) || w <= 0.0f) {
        w = (GetWidth() > 0.0f) ? GetWidth() : 310.0f;
    }
    if (m_impl->searchTextBox) {
        m_impl->searchTextBox->Measure(core::Size((std::max)(0.0f, w - 16.0f), 26.0f));
    }
    if (m_impl->inlineEditor && m_impl->inlineEditor->IsVisible()) {
        m_impl->inlineEditor->Measure(m_impl->editingRect.GetSize());
    }
    return availableSize;
}

void InspectorPanel::ArrangeOverride(core::Size finalSize) {
    if (m_impl->searchTextBox) {
        m_impl->searchTextBox->Arrange(core::Rect(8.0f, 74.0f, (std::max)(0.0f, finalSize.width - 16.0f), 26.0f));
    }
    if (m_impl->inlineEditor && m_impl->inlineEditor->IsVisible()) {
        m_impl->inlineEditor->Arrange(m_impl->editingRect);
    }
}

void InspectorPanel::ApplyTheme(const theme::Theme& theme) {
    Control::ApplyTheme(theme);
    if (m_impl->searchTextBox) {
        m_impl->searchTextBox->ApplyTheme(theme);
    }
    if (m_impl->inlineEditor) {
        m_impl->inlineEditor->ApplyTheme(theme);
    }
}

void InspectorPanel::Impl::StartInlineEdit(
    const std::string& name,
    bool isAttached,
    const std::string& initialText,
    const core::Rect& rect
) {
    CommitInlineEdit();
    editingPropName = name;
    isEditingAttached = isAttached;
    editingRect = rect;
    if (inlineEditor) {
        inlineEditor->SetText(initialText);
        inlineEditor->SetVisible(true);
        inlineEditor->Measure(rect.GetSize());
        inlineEditor->Arrange(rect);
        input::FocusManager::Get().SetFocus(inlineEditor.get());
        inlineEditor->SelectAll();
    }
    if (owner) {
        owner->InvalidateVisual();
    }
}

void InspectorPanel::Impl::CommitInlineEdit() {
    if (editingPropName.empty() || !owner) {
        return;
    }
    std::string name = editingPropName;
    bool attached = isEditingAttached;
    std::string buf;
    if (inlineEditor) {
        buf = inlineEditor->GetText();
        inlineEditor->SetVisible(false);
    }
    editingPropName.clear();

    auto p = owner->FindProperty(name);
    if (!p) {
        if (owner) owner->InvalidateVisual();
        return;
    }

    if (p->currentValue.Is<double>()) {
        try {
            double val = std::stod(buf);
            if (attached) {
                (void)owner->ChangeAttachedProperty(name, PropertyValue(val));
            } else {
                (void)owner->ChangeProperty(name, PropertyValue(val));
            }
        } catch (...) {}
    } else if (p->currentValue.Is<int32_t>()) {
        try {
            int32_t val = std::stoi(buf);
            if (attached) {
                (void)owner->ChangeAttachedProperty(name, PropertyValue(val));
            } else {
                (void)owner->ChangeProperty(name, PropertyValue(val));
            }
        } catch (...) {}
    } else if (p->currentValue.Is<bool>()) {
        auto res = PropertyEditorModel::ParseBool(buf);
        if (res.has_value()) {
            if (attached) {
                (void)owner->ChangeAttachedProperty(name, res.value());
            } else {
                (void)owner->ChangeProperty(name, res.value());
            }
        }
    } else if (p->currentValue.Is<core::Thickness>()) {
        auto res = PropertyEditorModel::ParseThickness(buf);
        if (res.has_value()) {
            if (attached) {
                (void)owner->ChangeAttachedProperty(name, res.value());
            } else {
                (void)owner->ChangeProperty(name, res.value());
            }
        }
    } else if (p->currentValue.Is<core::Color>()) {
        auto res = PropertyEditorModel::ParseColor(buf);
        if (res.has_value()) {
            if (attached) {
                (void)owner->ChangeAttachedProperty(name, res.value());
            } else {
                (void)owner->ChangeProperty(name, res.value());
            }
        }
    } else if (!p->enumOptions.empty()) {
        auto res = PropertyEditorModel::ParseEnum(name, buf);
        if (res.has_value()) {
            if (attached) {
                (void)owner->ChangeAttachedProperty(name, res.value());
            } else {
                (void)owner->ChangeProperty(name, res.value());
            }
        } else {
            if (attached) {
                (void)owner->ChangeAttachedProperty(name, PropertyValue(buf));
            } else {
                (void)owner->ChangeProperty(name, PropertyValue(buf));
            }
        }
    } else {
        if (attached) {
            (void)owner->ChangeAttachedProperty(name, PropertyValue(buf));
        } else {
            (void)owner->ChangeProperty(name, PropertyValue(buf));
        }
    }
    if (owner) {
        owner->InvalidateVisual();
    }
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
            p.enumOptions = InheritanceMetadataRegistry::GetEnumOptions(desc.name);

            const auto& s = m_impl->document->GetSettings();
            if (desc.name == "ClassName") {
                p.currentValue = PropertyValue(s.className);
            } else if (desc.name == "TargetNamespace") {
                p.currentValue = PropertyValue(s.targetNamespace);
            } else if (desc.name == "Title") {
                p.currentValue = PropertyValue(s.title);
            } else if (desc.name == "Width") {
                p.currentValue = PropertyValue(static_cast<double>(s.initialSize.width));
            } else if (desc.name == "Height") {
                p.currentValue = PropertyValue(static_cast<double>(s.initialSize.height));
            } else if (desc.name == "DefaultTheme") {
                p.currentValue = PropertyValue(s.defaultTheme);
            } else if (desc.name == "UseMvvm") {
                p.currentValue = PropertyValue(s.useMvvm);
            } else if (desc.name == "UseIoC") {
                p.currentValue = PropertyValue(s.useIoC);
            } else if (desc.name == "Background") {
                if (m_impl->document->GetRoot()) {
                    p.currentValue = m_impl->document->GetRoot()->GetProperty("Background").value_or(desc.defaultValue);
                } else {
                    p.currentValue = desc.defaultValue;
                }
            } else if (desc.name == "Padding") {
                if (m_impl->document->GetRoot()) {
                    p.currentValue = m_impl->document->GetRoot()->GetProperty("Padding").value_or(desc.defaultValue);
                } else {
                    p.currentValue = desc.defaultValue;
                }
            } else if (desc.name == "StartPosition") {
                p.currentValue = PropertyValue(s.startPosition);
            } else if (desc.name == "WindowState") {
                p.currentValue = PropertyValue(s.windowState);
            } else if (desc.name == "Topmost") {
                p.currentValue = PropertyValue(s.topmost);
            } else if (desc.name == "ShowInTaskbar") {
                p.currentValue = PropertyValue(s.showInTaskbar);
            } else if (desc.name == "AutoSize") {
                p.currentValue = PropertyValue(s.autoSize);
            } else if (desc.name == "AutoSizeMode") {
                p.currentValue = PropertyValue(s.autoSizeMode);
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
        const auto& formEvents = InheritanceMetadataRegistry::GetEventDescriptorsForType(NodeType::FormDocument);
        std::vector<EventBindingState> results;
        for (const auto& evDesc : formEvents) {
            if (!m_impl->filterText.empty() && !ContainsIgnoreCase(evDesc.eventName, m_impl->filterText)) {
                continue;
            }

            EventBindingState state;
            state.descriptor = evDesc;
            state.isMvvmCommand = false;

            if (m_impl->document->GetRoot()) {
                const auto& bindings = m_impl->document->GetRoot()->GetEventBindings();
                for (const auto& b : bindings) {
                    if (b.eventName == evDesc.eventName) {
                        state.boundHandlerOrCommand = b.handlerName;
                        state.isBound = true;
                        break;
                    }
                }
            }

            results.push_back(std::move(state));
        }
        return results;
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
        auto s = m_impl->document->GetSettings();
        bool handled = false;
        if (propName == "ClassName" && newValue.Is<std::string>()) {
            s.className = newValue.Get<std::string>().value();
            handled = true;
        } else if (propName == "TargetNamespace" && newValue.Is<std::string>()) {
            s.targetNamespace = newValue.Get<std::string>().value();
            handled = true;
        } else if (propName == "Title" && newValue.Is<std::string>()) {
            s.title = newValue.Get<std::string>().value();
            handled = true;
        } else if (propName == "Width") {
            double w = 0.0;
            if (newValue.Is<double>()) w = newValue.Get<double>().value();
            else if (newValue.Is<int32_t>()) w = newValue.Get<int32_t>().value();
            else return false;

            if (w < 0.0) return false;
            s.initialSize.width = static_cast<float>(w);
            handled = true;
        } else if (propName == "Height") {
            double h = 0.0;
            if (newValue.Is<double>()) h = newValue.Get<double>().value();
            else if (newValue.Is<int32_t>()) h = newValue.Get<int32_t>().value();
            else return false;

            if (h < 0.0) return false;
            s.initialSize.height = static_cast<float>(h);
            handled = true;
        } else if (propName == "DefaultTheme" && newValue.Is<std::string>()) {
            s.defaultTheme = newValue.Get<std::string>().value();
            handled = true;
        } else if (propName == "UseMvvm" && newValue.Is<bool>()) {
            s.useMvvm = newValue.Get<bool>().value_or(false);
            handled = true;
        } else if (propName == "UseIoC" && newValue.Is<bool>()) {
            s.useIoC = newValue.Get<bool>().value_or(false);
            handled = true;
        } else if (propName == "Background" && m_impl->document->GetRoot()) {
            m_impl->document->GetRoot()->SetProperty("Background", newValue);
            handled = true;
        } else if (propName == "Padding" && m_impl->document->GetRoot()) {
            m_impl->document->GetRoot()->SetProperty("Padding", newValue);
            handled = true;
        } else if (propName == "StartPosition" && newValue.Is<std::string>()) {
            s.startPosition = newValue.Get<std::string>().value_or("");
            handled = true;
        } else if (propName == "WindowState" && newValue.Is<std::string>()) {
            s.windowState = newValue.Get<std::string>().value_or("");
            handled = true;
        } else if (propName == "Topmost" && newValue.Is<bool>()) {
            s.topmost = newValue.Get<bool>().value_or(false);
            handled = true;
        } else if (propName == "ShowInTaskbar" && newValue.Is<bool>()) {
            s.showInTaskbar = newValue.Get<bool>().value_or(false);
            handled = true;
        } else if (propName == "AutoSize" && newValue.Is<bool>()) {
            s.autoSize = newValue.Get<bool>().value_or(false);
            handled = true;
        } else if (propName == "AutoSizeMode" && newValue.Is<std::string>()) {
            s.autoSizeMode = newValue.Get<std::string>().value_or("GrowAndShrink");
            handled = true;
        }

        if (handled) {
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
        if (m_impl->document->GetRoot()) {
            m_impl->document->GetRoot()->RemoveEventBinding(eventName);
            m_impl->document->GetRoot()->AddEventBinding(EventBindingDescriptor{
                std::string(eventName),
                std::string(handlerName)
            });
            m_impl->eventChangedSignal.Notify({ std::string(eventName), std::string(handlerName) });
            return true;
        }
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
        if (m_impl->document->GetRoot()) {
            m_impl->document->GetRoot()->RemoveEventBinding(eventName);
            m_impl->eventChangedSignal.Notify({ std::string(eventName), "" });
            return true;
        }
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
        std::string handlerName = "On" + m_impl->document->GetSettings().className + std::string(eventName);
        if (m_impl->document->GetRoot()) {
            m_impl->document->GetRoot()->RemoveEventBinding(eventName);
            m_impl->document->GetRoot()->AddEventBinding(EventBindingDescriptor{
                std::string(eventName),
                handlerName
            });
            m_impl->eventChangedSignal.Notify({ std::string(eventName), handlerName });
        }
        return handlerName;
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

static std::string FormatValueString(const PropertyValue& val, compat::string_view propName = "") {
    if (val.IsEmpty()) return "";
    if (auto v = val.Get<layout::Visibility>()) {
        switch (*v) {
            case layout::Visibility::Visible:   return "Visible";
            case layout::Visibility::Hidden:    return "Hidden";
            case layout::Visibility::Collapsed: return "Collapsed";
        }
    }
    if (auto h = val.Get<layout::HorizontalAlignment>()) {
        switch (*h) {
            case layout::HorizontalAlignment::Left:    return "Left";
            case layout::HorizontalAlignment::Center:  return "Center";
            case layout::HorizontalAlignment::Right:   return "Right";
            case layout::HorizontalAlignment::Stretch: return "Stretch";
        }
    }
    if (auto va = val.Get<layout::VerticalAlignment>()) {
        switch (*va) {
            case layout::VerticalAlignment::Top:     return "Top";
            case layout::VerticalAlignment::Center:  return "Center";
            case layout::VerticalAlignment::Bottom:  return "Bottom";
            case layout::VerticalAlignment::Stretch: return "Stretch";
        }
    }
    if (auto o = val.Get<layout::Orientation>()) {
        switch (*o) {
            case layout::Orientation::Horizontal: return "Horizontal";
            case layout::Orientation::Vertical:   return "Vertical";
        }
    }
    if (auto b = val.Get<bool>()) return *b ? "True" : "False";
    if (auto i = val.Get<int32_t>()) {
        if (propName == "FontWeight") {
            switch (*i) {
                case 100: return "Thin";
                case 200: return "ExtraLight";
                case 300: return "Light";
                case 400: return "Normal";
                case 500: return "Medium";
                case 600: return "SemiBold";
                case 700: return "Bold";
                case 800: return "ExtraBold";
                case 900: return "Black";
                default: break;
            }
        }
        return std::to_string(*i);
    }
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

    float w = GetActualSize().width > 0.0f ? GetActualSize().width : (GetWidth() > 0.0f ? GetWidth() : 310.0f);
    float h = GetActualSize().height > 0.0f ? GetActualSize().height : (GetHeight() > 0.0f ? GetHeight() : 720.0f);

    // 面板背景與左側邊框線
    dc.DrawRectangle(core::Rect(0.0f, 0.0f, w, h), colors.surfaceBackground);
    dc.DrawLine(core::Point(0.0f, 0.0f), core::Point(0.0f, h), colors.surfaceBorder, 1.0f);

    const float contentTop = 104.0f;
    const float rowHeight = 28.0f;
    const float catHeight = 26.0f;
    const float col1Width = 125.0f;

    float curY = contentTop - m_impl->scrollY;
    float totalContentH = 0.0f;
    bool isPropTab = (m_impl->currentTab == InspectorTab::Properties);

    if (isPropTab) {
        auto props = GetInspectedProperties();

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
            if (curY + catHeight >= contentTop && curY <= h) {
                core::Rect catRect(4.0f, curY, w - 8.0f, catHeight);
                dc.DrawRoundedRectangle(catRect, 2.0f, colors.controlBackground);
                std::string catTitle = std::string("▼ ") + group.title + " (" + std::to_string(groupProps.size()) + ")";
                dc.DrawText(catTitle, core::Point(8.0f, curY + 5.0f),
                            text::FontDescriptor("Segoe UI", 12.0f, text::FontWeight::Bold),
                            colors.textPrimary);
            }
            curY += catHeight + 2.0f;

            // 逐筆屬性列
            for (const auto* p : groupProps) {
                if (curY + rowHeight >= contentTop && curY <= h) {
                    // 左欄：屬性名稱
                    core::Rect nameRect(8.0f, curY, col1Width, rowHeight);
                    std::string propName = p->name;
                    if (p->isAttached) {
                        propName += " *";
                    }
                    dc.DrawText(propName, core::Point(nameRect.x + 2.0f, curY + 6.0f * m_impl->fontScale),
                                text::FontDescriptor("Segoe UI", 12.0f * m_impl->fontScale),
                                p->isAttached ? core::Color(78, 160, 220) : colors.textPrimary);

                    // 右欄：屬性值方塊
                    core::Rect valRect(col1Width + 12.0f, curY + 2.0f, w - col1Width - 24.0f, rowHeight - 4.0f);
                    dc.DrawRoundedRectangle(valRect, 3.0f, colors.controlBackground);

                    if (m_impl->editingPropName == p->name) {
                        // 作用中行內編輯態：由 m_impl->inlineEditor 子元件負責渲染
                    } else {
                        DrawRectOutline(dc, valRect, colors.controlBorder, 1.0f);

                        if (p->IsMixed()) {
                            dc.DrawText("<Mixed Values>", core::Point(valRect.x + 6.0f, valRect.y + 4.0f * m_impl->fontScale),
                                        text::FontDescriptor("Segoe UI", 11.5f * m_impl->fontScale), colors.textDisabled);
                        } else if (p->currentValue.Is<core::Color>()) {
                            auto c = p->currentValue.Get<core::Color>();
                            if (c.has_value()) {
                                // 顏色預覽色塊
                                core::Rect swatch(valRect.x + 4.0f, valRect.y + 4.0f, 16.0f, 16.0f);
                                dc.DrawRectangle(swatch, *c);
                                DrawRectOutline(dc, swatch, colors.controlBorder, 1.0f);

                                char hexBuf[16];
                                std::snprintf(hexBuf, sizeof(hexBuf), "#%02X%02X%02X", c->r, c->g, c->b);
                                dc.DrawText(hexBuf, core::Point(valRect.x + 26.0f, valRect.y + 4.0f * m_impl->fontScale),
                                            text::FontDescriptor("Segoe UI", 11.5f * m_impl->fontScale), colors.textPrimary);
                            }
                        } else if (p->currentValue.Is<bool>()) {
                            auto b = p->currentValue.Get<bool>();
                            bool val = b.has_value() && *b;
                            core::Rect checkSquare(valRect.x + 6.0f, valRect.y + 5.0f, 14.0f, 14.0f);
                            DrawRectOutline(dc, checkSquare, colors.controlBorder, 1.0f);
                            if (val) {
                                dc.DrawRoundedRectangle(core::Rect(checkSquare.x + 2.0f, checkSquare.y + 2.0f, 10.0f, 10.0f),
                                                        1.0f, colors.accentPrimary);
                            }
                            dc.DrawText(val ? "True (點擊切換)" : "False (點擊切換)", core::Point(valRect.x + 26.0f, valRect.y + 4.0f * m_impl->fontScale),
                                        text::FontDescriptor("Segoe UI", 11.5f * m_impl->fontScale), colors.textPrimary);
                        } else if (!p->enumOptions.empty()) {
                            std::string valStr = FormatValueString(p->currentValue, p->name);
                            dc.DrawText(valStr, core::Point(valRect.x + 6.0f, valRect.y + 4.0f * m_impl->fontScale),
                                        text::FontDescriptor("Segoe UI", 11.5f * m_impl->fontScale), colors.textPrimary);
                            dc.DrawText("▼", core::Point(valRect.x + valRect.width - 16.0f, valRect.y + 4.0f * m_impl->fontScale),
                                        text::FontDescriptor("Segoe UI", 9.0f * m_impl->fontScale), colors.textSecondary);
                        } else {
                            std::string valStr = FormatValueString(p->currentValue, p->name);
                            dc.DrawText(valStr, core::Point(valRect.x + 6.0f, valRect.y + 4.0f * m_impl->fontScale),
                                        text::FontDescriptor("Segoe UI", 11.5f * m_impl->fontScale), colors.textPrimary);
                        }
                    }
                }
                curY += rowHeight;
            }
            curY += 4.0f;
        }
        totalContentH = (curY + m_impl->scrollY) - contentTop;
    } else {
        // 事件分頁
        auto events = GetInspectedEvents();
        if (curY + catHeight >= contentTop && curY <= h) {
            core::Rect catRect(4.0f, curY, w - 8.0f, catHeight);
            dc.DrawRoundedRectangle(catRect, 2.0f, colors.controlBackground);
            std::string catTitle = "▼ 控制項事件綁定 (Events & Commands - " + std::to_string(events.size()) + ")";
            dc.DrawText(catTitle, core::Point(8.0f, curY + 5.0f),
                        text::FontDescriptor("Segoe UI", 12.0f, text::FontWeight::Bold),
                        colors.textPrimary);
        }
        curY += catHeight + 2.0f;

        for (const auto& evt : events) {
            if (curY + rowHeight >= contentTop && curY <= h) {
                // 事件名稱
                dc.DrawText(evt.descriptor.eventName, core::Point(10.0f, curY + 6.0f),
                            text::FontDescriptor("Segoe UI", 12.0f), colors.textPrimary);

                // 處理常式名稱方塊
                core::Rect valRect(col1Width + 12.0f, curY + 2.0f, w - col1Width - 24.0f, rowHeight - 4.0f);
                dc.DrawRoundedRectangle(valRect, 3.0f, colors.controlBackground);
                DrawRectOutline(dc, valRect, colors.controlBorder, 1.0f);

                if (evt.isBound) {
                    dc.DrawText("⚡ " + evt.boundHandlerOrCommand, core::Point(valRect.x + 6.0f, valRect.y + 4.0f),
                                text::FontDescriptor("Segoe UI", 11.5f, text::FontWeight::Bold),
                                core::Color(30, 160, 90));
                    // 繪製解除綁定按鈕 [✕]
                    core::Rect delRect(valRect.x + valRect.width - 30.0f, valRect.y + 2.0f, 26.0f, valRect.height - 4.0f);
                    dc.DrawRoundedRectangle(delRect, 2.0f, core::Color(65, 30, 30));
                    DrawRectOutline(dc, delRect, core::Color(180, 50, 50), 1.0f);
                    dc.DrawText("✕", core::Point(delRect.x + 8.0f, delRect.y + 2.0f),
                                text::FontDescriptor("Segoe UI", 11.0f, text::FontWeight::Bold),
                                core::Color(255, 100, 100));
                } else {
                    dc.DrawText("[+ 點擊建立常式]", core::Point(valRect.x + 6.0f, valRect.y + 4.0f),
                                text::FontDescriptor("Segoe UI", 11.0f),
                                colors.accentPrimary);
                }
            }
            curY += rowHeight;
        }
        totalContentH = (curY + m_impl->scrollY) - contentTop;
    }

    // 右側垂直滾動條 (Scrollbar)
    const float scrollTrackH = h - contentTop - 2.0f;
    if (totalContentH > scrollTrackH && scrollTrackH > 20.0f) {
        const float barW = 6.0f;
        float thumbH = (std::max)(24.0f, (scrollTrackH / totalContentH) * scrollTrackH);
        float maxScroll = totalContentH - scrollTrackH;
        float ratio = m_impl->scrollY / maxScroll;
        ratio = compat::clamp(ratio, 0.0f, 1.0f);
        float thumbY = contentTop + ratio * (scrollTrackH - thumbH);
        core::Rect thumbRect(w - barW - 2.0f, thumbY, barW, thumbH);
        dc.DrawRoundedRectangle(thumbRect, 3.0f, core::Color(100, 100, 105));
    }

    // 頂部固定區域遮罩與渲染 (防止滾動內容滲透標頭)
    dc.DrawRectangle(core::Rect(0.0f, 0.0f, w, contentTop), colors.surfaceBackground);

    // 1. 選取元件標頭 (Selected Component Header)
    core::Rect headerRect(0.0f, 0.0f, w, 38.0f);
    dc.DrawRectangle(headerRect, colors.controlBackground);
    dc.DrawLine(core::Point(0.0f, 38.0f), core::Point(w, 38.0f), colors.surfaceBorder, 1.0f);

    auto selectedNodes = m_impl->GetSelectedNodes();
    if (selectedNodes.empty()) {
        std::string formName = (m_impl->document ? m_impl->document->GetSettings().className : "Form");
        std::string headerTitle = "📄 " + formName + " : FormDocument";
        dc.DrawText(headerTitle, core::Point(10.0f, 10.0f),
                    text::FontDescriptor("Segoe UI", 12.5f, text::FontWeight::Bold), colors.textPrimary);
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
                text::FontDescriptor("Segoe UI", 12.0f, isPropTab ? text::FontWeight::Bold : text::FontWeight::Normal),
                isPropTab ? colors.accentPrimary : colors.textSecondary);

    dc.DrawText("事件 (Events)", core::Point(eventTabRect.x + 24.0f, tabY + 6.0f),
                text::FontDescriptor("Segoe UI", 12.0f, !isPropTab ? text::FontWeight::Bold : text::FontWeight::Normal),
                !isPropTab ? colors.accentPrimary : colors.textSecondary);

    // 3. 搜尋篩選列 (Filter Box) 由 m_impl->searchTextBox 子元件自行繪製

    // 4. 浮動下拉選單 (Dropdown Overlay)
    if (m_impl->activeDropdown.has_value()) {
        const auto& dd = m_impl->activeDropdown.value();
        dc.DrawRectangle(dd.popupRect, colors.cardBackground);
        DrawRectOutline(dc, dd.popupRect, colors.accentPrimary, 1.5f);

        float itemY = dd.popupRect.y + 2.0f;
        float itemH = 24.0f * m_impl->fontScale;
        for (const auto& opt : dd.options) {
            core::Rect itemRect(dd.popupRect.x + 2.0f, itemY, dd.popupRect.width - 4.0f, itemH);
            dc.DrawText(opt, core::Point(itemRect.x + 8.0f, itemRect.y + 3.0f * m_impl->fontScale),
                        text::FontDescriptor("Segoe UI", 11.5f * m_impl->fontScale), colors.textPrimary);
            dc.DrawLine(core::Point(itemRect.x + 4.0f, itemY + itemH),
                        core::Point(itemRect.x + itemRect.width - 4.0f, itemY + itemH),
                        colors.controlBorder, 0.5f);
            itemY += itemH;
        }
    }
}

void InspectorPanel::OnPointerWheelCore(input::PointerEventArgs& e) {
    int32_t delta = e.GetWheelDeltaY();
    if (delta == 0) return;

    m_impl->CommitInlineEdit();

    float h = GetHeight();
    if (h <= 0.0f) h = 720.0f;
    const float contentTop = 104.0f;
    const float scrollTrackH = h - contentTop - 2.0f;

    float totalContentH = 0.0f;
    if (m_impl->currentTab == InspectorTab::Properties) {
        auto props = GetInspectedProperties();
        totalContentH = static_cast<float>(props.size()) * 28.0f + 5.0f * 26.0f + 20.0f;
    } else {
        auto events = GetInspectedEvents();
        totalContentH = static_cast<float>(events.size()) * 28.0f + 30.0f;
    }

    float maxScroll = (std::max)(0.0f, totalContentH - scrollTrackH);
    float step = 28.0f * 2.0f;

    if (delta > 0) {
        m_impl->scrollY = (std::max)(0.0f, m_impl->scrollY - step);
    } else {
        m_impl->scrollY = (std::min)(maxScroll, m_impl->scrollY + step);
    }
    InvalidateVisual();
    e.SetHandled(true);
}

void InspectorPanel::OnPointerPressedCore(input::PointerEventArgs& e) {
    if (e.IsHandled()) {
        return;
    }
    core::Point pt = e.GetPosition(this);
    float w = GetActualSize().width > 0.0f ? GetActualSize().width : (GetWidth() > 0.0f ? GetWidth() : 310.0f);

    // 0. 浮動下拉選單選項點擊攔截
    if (m_impl->activeDropdown.has_value()) {
        auto dd = m_impl->activeDropdown.value();
        m_impl->activeDropdown = compat::nullopt; // 關閉下拉選單
        if (dd.popupRect.Contains(pt)) {
            float itemH = 24.0f * m_impl->fontScale;
            float relY = pt.y - dd.popupRect.y - 2.0f;
            if (relY >= 0.0f) {
                size_t selIdx = static_cast<size_t>(relY / itemH);
                if (selIdx < dd.options.size()) {
                    const std::string& chosen = dd.options[selIdx];
                    auto parsedEnum = PropertyEditorModel::ParseEnum(dd.propName, chosen);
                    PropertyValue valToSet = parsedEnum.has_value() ? parsedEnum.value() : PropertyValue(chosen);
                    if (dd.isAttached) {
                        ChangeAttachedProperty(dd.propName, valToSet);
                    } else {
                        ChangeProperty(dd.propName, valToSet);
                    }
                }
            }
            InvalidateVisual();
            e.SetHandled(true);
            return;
        }
        InvalidateVisual();
    }

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

    const float contentTop = 104.0f;
    const float rowHeight = 28.0f;
    const float catHeight = 26.0f;
    const float col1Width = 125.0f;

    // 2. 屬性互動與就地修改 (Inline Editing & Toggling)
    if (m_impl->currentTab == InspectorTab::Properties) {
        auto props = GetInspectedProperties();
        float curY = contentTop - m_impl->scrollY;

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

            curY += catHeight + 2.0f;

            for (const auto* p : groupProps) {
                core::Rect rowRect(0.0f, curY, w, rowHeight);
                core::Rect valRect(col1Width + 12.0f, curY + 2.0f, w - col1Width - 24.0f, rowHeight - 4.0f);

                if (rowRect.Contains(pt)) {
                    // 布林值屬性支援整列點擊切換
                    if (p->currentValue.Is<bool>()) {
                        m_impl->CommitInlineEdit();
                        bool curVal = p->currentValue.Get<bool>().value_or(false);
                        if (p->isAttached) {
                            ChangeAttachedProperty(p->name, PropertyValue(!curVal));
                        } else {
                            ChangeProperty(p->name, PropertyValue(!curVal));
                        }
                        InvalidateVisual();
                        e.SetHandled(true);
                        return;
                    }

                    if (valRect.Contains(pt)) {
                        if (!p->enumOptions.empty()) {
                            m_impl->CommitInlineEdit();
                            // 展開或收合浮動下拉選單
                            Impl::ActiveDropdown dd;
                            dd.propName = p->name;
                            dd.isAttached = p->isAttached;
                            dd.options = p->enumOptions;
                            float popH = static_cast<float>(p->enumOptions.size()) * 24.0f * m_impl->fontScale + 4.0f;
                            dd.popupRect = core::Rect(valRect.x, valRect.y + valRect.height + 2.0f, valRect.width, popH);
                            m_impl->activeDropdown = std::move(dd);
                        } else if (p->currentValue.Is<core::Color>()) {
                            core::Rect swatch(valRect.x + 4.0f, valRect.y + 4.0f, 16.0f, 16.0f);
                            if (swatch.Contains(pt)) {
                                m_impl->CommitInlineEdit();
                                static const core::Color s_presetColors[] = {
                                    core::Color(14, 110, 210),
                                    core::Color(24, 134, 75),
                                    core::Color(210, 50, 45),
                                    core::Color(130, 70, 210),
                                    core::Color(60, 64, 75)
                                };
                                static size_t s_colorIdx = 0;
                                s_colorIdx = (s_colorIdx + 1) % 5;
                                if (p->isAttached) {
                                    ChangeAttachedProperty(p->name, PropertyValue(s_presetColors[s_colorIdx]));
                                } else {
                                    ChangeProperty(p->name, PropertyValue(s_presetColors[s_colorIdx]));
                                }
                            } else {
                                // 點擊色塊右側文字區域：啟動行內文字編輯模式 (#RRGGBB)
                                core::Rect editRect(valRect.x + 24.0f, valRect.y, valRect.width - 24.0f, valRect.height);
                                m_impl->StartInlineEdit(p->name, p->isAttached, FormatValueString(p->currentValue, p->name), editRect);
                            }
                        } else {
                            // 啟動行內文字編輯模式
                            m_impl->StartInlineEdit(p->name, p->isAttached, FormatValueString(p->currentValue, p->name), valRect);
                        }
                        InvalidateVisual();
                        e.SetHandled(true);
                        return;
                    }
                }
                curY += rowHeight;
            }
            curY += 4.0f;
        }

        // 點擊其他空白處，提交當前文字編輯
        m_impl->CommitInlineEdit();
        InvalidateVisual();
    }

    // 3. 點擊事件自動綁定與解除
    if (m_impl->currentTab == InspectorTab::Events) {
        auto events = GetInspectedEvents();
        float curY = contentTop + catHeight + 2.0f - m_impl->scrollY;

        for (const auto& evt : events) {
            core::Rect valRect(col1Width + 12.0f, curY + 2.0f, w - col1Width - 24.0f, rowHeight - 4.0f);
            if (valRect.Contains(pt)) {
                if (evt.isBound) {
                    // 擴大解除綁定按鈕命中熱區 (覆蓋右側 40px 及完整列高)
                    core::Rect delHitRect(valRect.x + valRect.width - 40.0f, curY, 40.0f, rowHeight);
                    if (delHitRect.Contains(pt)) {
                        UnbindEvent(evt.descriptor.eventName);
                        InvalidateVisual();
                        e.SetHandled(true);
                        return;
                    }
                } else {
                    AutoBindEvent(evt.descriptor.eventName);
                    InvalidateVisual();
                    e.SetHandled(true);
                    return;
                }
            }
            curY += rowHeight;
        }
    }
}

void InspectorPanel::OnKeyDownCore(input::KeyEventArgs& e) {
    if (e.GetKey() == input::Key::Escape) {
        if (m_impl->activeDropdown.has_value()) {
            m_impl->activeDropdown = compat::nullopt;
            InvalidateVisual();
            e.SetHandled(true);
            return;
        }
        if (!m_impl->editingPropName.empty()) {
            m_impl->CancelInlineEdit();
            InvalidateVisual();
            e.SetHandled(true);
            return;
        }
    } else if (e.GetKey() == input::Key::Enter || e.GetKey() == input::Key::Return) {
        if (!m_impl->editingPropName.empty()) {
            m_impl->CommitInlineEdit();
            InvalidateVisual();
            e.SetHandled(true);
            return;
        }
    }
}

void InspectorPanel::OnTextInputCore(input::TextInputEventArgs& e) {
    (void)e;
}

} // namespace inspector
} // namespace designer
} // namespace cppui
