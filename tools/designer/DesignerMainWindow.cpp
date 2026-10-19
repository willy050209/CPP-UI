/**
 * @file DesignerMainWindow.cpp
 * @brief CPPUI 官方獨立視覺化表單設計器主視窗實作
 */

#include "DesignerMainWindow.hpp"
#include "ui/Designer.hpp"
#include "ui/dsl/Fluent.hpp"
#include "ui/app/Application.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/controls/Label.hpp"
#include "ui/controls/CheckBox.hpp"
#include "ui/controls/ListBox.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/Grid.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/theme/ThemeManager.hpp"
#include "ui/designer/toolbox/ToolboxPanel.hpp"
#include "ui/designer/preview/CodePreviewWindow.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace cppui {
namespace designer {
namespace tool {

static FormSettings CreateInitialSettings() {
    FormSettings s;
    s.className = "AccountSettingsForm";
    s.targetNamespace = "EnterpriseApp";
    s.initialSize = core::Size(800.0f, 520.0f);
    s.title = "使用者帳戶與安全性設定";
    return s;
}

static const char* GetNodeIcon(NodeType type) {
    switch (type) {
    case NodeType::Canvas: return "🎨 ";
    case NodeType::Grid: return "⊞ ";
    case NodeType::StackPanel: return "☰ ";
    case NodeType::VirtualizingStackPanel: return "☰ ";
    case NodeType::Border: return "🔲 ";
    case NodeType::ScrollViewer: return "📜 ";
    case NodeType::Popup: return "💬 ";
    case NodeType::TreeView: return "🌲 ";
    case NodeType::ContextMenu: return "📑 ";
    case NodeType::MenuItem: return "📄 ";
    case NodeType::Button: return "🔘 ";
    case NodeType::TextBox: return "🔤 ";
    case NodeType::Label: return "🏷 ";
    case NodeType::TextBlock: return "📝 ";
    case NodeType::CheckBox: return "☑ ";
    case NodeType::RadioButton: return "⚪ ";
    case NodeType::ComboBox: return "▼ ";
    case NodeType::ListBox: return "📋 ";
    case NodeType::DataGrid: return "▦ ";
    case NodeType::PictureBox: return "🖼 ";
    case NodeType::ProgressBar: return "📊 ";
    case NodeType::Slider: return "🎚 ";
    case NodeType::Separator: return "➖ ";
    case NodeType::Timer: return "⏱ ";
    case NodeType::OpenFileDialog:
    case NodeType::SaveFileDialog: return "📁 ";
    case NodeType::MessageBox: return "💡 ";
    default: return "📦 ";
    }
}

static const char* GetNodeTypeName(NodeType type) {
    switch (type) {
    case NodeType::FormDocument: return "Form";
    case NodeType::Canvas: return "Canvas";
    case NodeType::Grid: return "Grid";
    case NodeType::StackPanel: return "StackPanel";
    case NodeType::VirtualizingStackPanel: return "VirtualizingStackPanel";
    case NodeType::Border: return "Border";
    case NodeType::ScrollViewer: return "ScrollViewer";
    case NodeType::Popup: return "Popup";
    case NodeType::TreeView: return "TreeView";
    case NodeType::ContextMenu: return "ContextMenu";
    case NodeType::MenuItem: return "MenuItem";
    case NodeType::Button: return "Button";
    case NodeType::TextBox: return "TextBox";
    case NodeType::TextBlock: return "TextBlock";
    case NodeType::Label: return "Label";
    case NodeType::CheckBox: return "CheckBox";
    case NodeType::RadioButton: return "RadioButton";
    case NodeType::ComboBox: return "ComboBox";
    case NodeType::ListBox: return "ListBox";
    case NodeType::DataGrid: return "DataGrid";
    case NodeType::PictureBox: return "PictureBox";
    case NodeType::ProgressBar: return "ProgressBar";
    case NodeType::Slider: return "Slider";
    case NodeType::Separator: return "Separator";
    case NodeType::Timer: return "Timer";
    case NodeType::OpenFileDialog: return "OpenFileDialog";
    case NodeType::SaveFileDialog: return "SaveFileDialog";
    case NodeType::MessageBox: return "MessageBox";
    default: return "Widget";
    }
}

struct DesignerMainWindow::Impl {
    FormDocumentNode m_doc{ CreateInitialSettings() };
    CommandHistoryManager m_history;
    std::shared_ptr<surface::DesignSurface> m_surface;
    std::shared_ptr<toolbox::ToolboxPanel> m_toolbox;
    std::shared_ptr<preview::CodePreviewWindow> m_codeWindow;
    std::shared_ptr<inspector::InspectorPanel> m_inspector;
    std::shared_ptr<controls::ListBox> m_hierarchyBox;
    std::vector<std::string> m_hierarchyUuids;
    std::string m_hierarchyFilter;
    bool m_isSyncingHierarchy = false;

    std::shared_ptr<controls::TextBlock> m_titleBlock;
    std::shared_ptr<controls::TextBlock> m_toolboxHeader;
    std::shared_ptr<controls::TextBlock> m_treeHeader;
    std::shared_ptr<controls::TextBlock> m_surfaceHeader;
    std::shared_ptr<controls::TextBlock> m_inspectorHeader;
    std::shared_ptr<controls::Label> m_statusLabel;
    std::shared_ptr<controls::TextBox> m_toolboxSearch;
    std::shared_ptr<controls::TextBox> m_treeSearch;

    std::vector<reactive::Connection> m_connections;
    float m_fontScale = 1.0f;

    std::string m_saveBtnUuid;
    std::string m_cancelBtnUuid;
    std::string m_userInputUuid;

    void RebuildHierarchyTree() {
        if (!m_hierarchyBox) return;

        std::string currentSelectedUuid = "";
        if (m_surface) {
            auto sel = m_surface->GetSelectionModel().GetPrimarySelection();
            if (sel.has_value()) {
                currentSelectedUuid = sel.value();
            }
        }

        m_hierarchyBox->ClearItems();
        m_hierarchyUuids.clear();

        // 0: Form root document (empty UUID denotes FormDocument itself)
        std::string formTitle = "▾ 📄 " + m_doc.GetSettings().className + " (Form)";
        m_hierarchyBox->AddItem(formTitle);
        m_hierarchyUuids.push_back("");

        // Recursive walk helper
        std::function<void(const WidgetNode*, size_t)> walk = [&](const WidgetNode* node, size_t depth) {
            if (!node) return;

            std::string indent(depth * 3, ' ');
            std::string typeName = GetNodeTypeName(node->GetType());
            std::string itemText = indent + GetNodeIcon(node->GetType()) + node->GetName() + " (" + typeName + ")";

            bool matches = true;
            if (!m_hierarchyFilter.empty()) {
                std::string lowerFilter = m_hierarchyFilter;
                std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                std::string lowerText = itemText;
                std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                matches = (lowerText.find(lowerFilter) != std::string::npos);
            }

            if (matches) {
                m_hierarchyBox->AddItem(itemText);
                m_hierarchyUuids.push_back(node->GetUuid());
            }

            for (size_t i = 0; i < node->GetChildCount(); ++i) {
                walk(node->GetChild(i), depth + 1);
            }
        };

        if (m_doc.GetRoot()) {
            walk(m_doc.GetRoot(), 1);
        }

        // Restore selection in ListBox
        int32_t selectIdx = -1;
        if (currentSelectedUuid.empty()) {
            selectIdx = 0;
        } else {
            for (size_t i = 0; i < m_hierarchyUuids.size(); ++i) {
                if (m_hierarchyUuids[i] == currentSelectedUuid) {
                    selectIdx = static_cast<int32_t>(i);
                    break;
                }
            }
        }

        m_isSyncingHierarchy = true;
        m_hierarchyBox->SetSelectedIndex(selectIdx);
        m_isSyncingHierarchy = false;
    }

    void UpdateStatus() {
        if (!m_statusLabel || !m_surface) return;
        auto sel = m_surface->GetSelectionModel().GetPrimarySelection();
        if (!sel.has_value() || sel.value().empty()) {
            m_statusLabel->SetText(
                "狀態: 就緒 | 當前選取: " + m_doc.GetSettings().className +
                " (FormDocument) | 尺寸: " +
                std::to_string(static_cast<int32_t>(m_doc.GetSettings().initialSize.width)) + " x " +
                std::to_string(static_cast<int32_t>(m_doc.GetSettings().initialSize.height)) +
                " | 歷史: " + std::to_string(m_history.GetUndoCount()) + " 步"
            );
        } else {
            const std::string& uuid = sel.value();
            const auto* node = m_doc.FindByUuid(uuid);
            auto bounds = m_surface->GetNodeBounds(uuid);
            std::string name = node ? node->GetName() : uuid;
            std::string typeName = node ? GetNodeTypeName(node->GetType()) : "Widget";
            std::string boundsStr = " | 位置: (" + std::to_string(static_cast<int32_t>(bounds.x)) + ", " +
                                    std::to_string(static_cast<int32_t>(bounds.y)) + ") 尺寸: " +
                                    std::to_string(static_cast<int32_t>(bounds.width)) + " x " +
                                    std::to_string(static_cast<int32_t>(bounds.height));
            m_statusLabel->SetText(
                "狀態: 就緒 | 當前選取: " + name + " (" + typeName + ")" +
                boundsStr + " | 導引線: 啟用 | 歷史: " +
                std::to_string(m_history.GetUndoCount()) + " 步"
            );
        }
    }

    void BuildDefaultAst() {
        auto rootGrid = std::make_unique<WidgetNode>(NodeType::Canvas, "mainCanvas");
        rootGrid->SetProperty("Background", PropertyValue(core::Color(28, 30, 36)));
        rootGrid->SetProperty("Padding", PropertyValue(core::Thickness(16.0f)));

        // 1. 標題文字
        auto titleLabel = std::make_unique<WidgetNode>(NodeType::Label, "titleLabel");
        titleLabel->SetProperty("Content", PropertyValue(std::string("使用者帳戶與安全性設定")));
        titleLabel->SetProperty("FontSize", PropertyValue(18.0f));
        titleLabel->SetProperty("Foreground", PropertyValue(core::Color(255, 255, 255)));
        titleLabel->SetAttachedProperty("Canvas.Left", PropertyValue(20.0));
        titleLabel->SetAttachedProperty("Canvas.Top", PropertyValue(20.0));
        std::string titleUuid = titleLabel->GetUuid();
        (void)rootGrid->AddChild(std::move(titleLabel));

        // 2. 帳號名稱標籤與輸入框
        auto userLabel = std::make_unique<WidgetNode>(NodeType::Label, "userLabel");
        userLabel->SetProperty("Content", PropertyValue(std::string("帳號名稱 (Username):")));
        userLabel->SetProperty("FontSize", PropertyValue(12.0f));
        userLabel->SetAttachedProperty("Canvas.Left", PropertyValue(20.0));
        userLabel->SetAttachedProperty("Canvas.Top", PropertyValue(65.0));
        std::string userLabelUuid = userLabel->GetUuid();
        (void)rootGrid->AddChild(std::move(userLabel));

        auto userInput = std::make_unique<WidgetNode>(NodeType::TextBox, "usernameInput");
        userInput->SetProperty("Text", PropertyValue(std::string("administrator")));
        userInput->SetProperty("Width", PropertyValue(260.0f));
        userInput->SetProperty("Height", PropertyValue(32.0f));
        userInput->SetAttachedProperty("Canvas.Left", PropertyValue(170.0));
        userInput->SetAttachedProperty("Canvas.Top", PropertyValue(62.0));
        m_userInputUuid = userInput->GetUuid();
        (void)rootGrid->AddChild(std::move(userInput));

        // 3. 電子郵件標籤與輸入框
        auto emailLabel = std::make_unique<WidgetNode>(NodeType::Label, "emailLabel");
        emailLabel->SetProperty("Content", PropertyValue(std::string("電子郵件 (Email):")));
        emailLabel->SetProperty("FontSize", PropertyValue(12.0f));
        emailLabel->SetAttachedProperty("Canvas.Left", PropertyValue(20.0));
        emailLabel->SetAttachedProperty("Canvas.Top", PropertyValue(105.0));
        std::string emailLabelUuid = emailLabel->GetUuid();
        (void)rootGrid->AddChild(std::move(emailLabel));

        auto emailInput = std::make_unique<WidgetNode>(NodeType::TextBox, "emailInput");
        emailInput->SetProperty("Text", PropertyValue(std::string("admin@enterprise.internal")));
        emailInput->SetProperty("Width", PropertyValue(260.0f));
        emailInput->SetProperty("Height", PropertyValue(32.0f));
        emailInput->SetAttachedProperty("Canvas.Left", PropertyValue(170.0));
        emailInput->SetAttachedProperty("Canvas.Top", PropertyValue(102.0));
        std::string emailInputUuid = emailInput->GetUuid();
        (void)rootGrid->AddChild(std::move(emailInput));

        // 4. 雙重驗證核取方塊
        auto authCheck = std::make_unique<WidgetNode>(NodeType::CheckBox, "authCheck");
        authCheck->SetProperty("Content", PropertyValue(std::string("強制啟用雙重驗證 (2FA Authentication)")));
        authCheck->SetProperty("IsChecked", PropertyValue(true));
        authCheck->SetAttachedProperty("Canvas.Left", PropertyValue(170.0));
        authCheck->SetAttachedProperty("Canvas.Top", PropertyValue(145.0));
        std::string authCheckUuid = authCheck->GetUuid();
        (void)rootGrid->AddChild(std::move(authCheck));

        // 5. 儲存設定按鈕
        auto saveBtn = std::make_unique<WidgetNode>(NodeType::Button, "saveBtn");
        saveBtn->SetProperty("Content", PropertyValue(std::string("儲存設定 (Save)")));
        saveBtn->SetProperty("Width", PropertyValue(130.0f));
        saveBtn->SetProperty("Height", PropertyValue(34.0f));
        saveBtn->SetProperty("Background", PropertyValue(core::Color(14, 110, 210)));
        saveBtn->SetProperty("Foreground", PropertyValue(core::Color(255, 255, 255)));
        saveBtn->SetAttachedProperty("Canvas.Left", PropertyValue(170.0));
        saveBtn->SetAttachedProperty("Canvas.Top", PropertyValue(190.0));
        saveBtn->AddEventBinding(EventBindingDescriptor{ "Click", "OnSaveSettingsClick" });
        m_saveBtnUuid = saveBtn->GetUuid();
        (void)rootGrid->AddChild(std::move(saveBtn));

        // 6. 取消按鈕
        auto cancelBtn = std::make_unique<WidgetNode>(NodeType::Button, "cancelBtn");
        cancelBtn->SetProperty("Content", PropertyValue(std::string("取消 (Cancel)")));
        cancelBtn->SetProperty("Width", PropertyValue(100.0f));
        cancelBtn->SetProperty("Height", PropertyValue(34.0f));
        cancelBtn->SetProperty("Background", PropertyValue(core::Color(60, 64, 75)));
        cancelBtn->SetAttachedProperty("Canvas.Left", PropertyValue(310.0));
        cancelBtn->SetAttachedProperty("Canvas.Top", PropertyValue(190.0));
        cancelBtn->AddEventBinding(EventBindingDescriptor{ "Click", "OnCancelClick" });
        m_cancelBtnUuid = cancelBtn->GetUuid();
        (void)rootGrid->AddChild(std::move(cancelBtn));

        std::string rootUuid = rootGrid->GetUuid();
        m_doc.SetRoot(std::move(rootGrid));

        // 佈局座標快取映射 (提供畫布渲染與選取)
        m_surface->SetNodeBounds(rootUuid, core::Rect(0.0f, 0.0f, 540.0f, 360.0f));
        m_surface->SetNodeBounds(titleUuid, core::Rect(20.0f, 20.0f, 320.0f, 28.0f));
        m_surface->SetNodeBounds(userLabelUuid, core::Rect(20.0f, 65.0f, 140.0f, 24.0f));
        m_surface->SetNodeBounds(m_userInputUuid, core::Rect(170.0f, 62.0f, 260.0f, 30.0f));
        m_surface->SetNodeBounds(emailLabelUuid, core::Rect(20.0f, 105.0f, 140.0f, 24.0f));
        m_surface->SetNodeBounds(emailInputUuid, core::Rect(170.0f, 102.0f, 260.0f, 30.0f));
        m_surface->SetNodeBounds(authCheckUuid, core::Rect(170.0f, 145.0f, 280.0f, 26.0f));
        m_surface->SetNodeBounds(m_saveBtnUuid, core::Rect(170.0f, 190.0f, 130.0f, 34.0f));
        m_surface->SetNodeBounds(m_cancelBtnUuid, core::Rect(310.0f, 190.0f, 100.0f, 34.0f));

        // 預設選取「儲存設定 (saveBtn)」元件，立即呈現 8 向控制點與導引線
        m_surface->GetSelectionModel().SetSelection(m_saveBtnUuid);
    }
};

DesignerMainWindow::DesignerMainWindow()
    : m_impl(std::make_unique<Impl>()) {
    using namespace cppui::dsl;

    SetTitle("CPPUI Visual Form Designer - 視覺化表單設計器 (v1.0)");
    SetFormSize(1280, 820);
    SetWindowState(app::WindowState::Maximized);

    // 實例化畫布與檢查器 (使用 Stretch 自適應展開)
    m_impl->m_surface = std::make_shared<surface::DesignSurface>(&m_impl->m_doc, &m_impl->m_history);
    m_impl->m_surface->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    m_impl->m_surface->SetVerticalAlignment(layout::VerticalAlignment::Stretch);

    m_impl->m_inspector = std::make_shared<inspector::InspectorPanel>(
        &m_impl->m_doc,
        &m_impl->m_history,
        &m_impl->m_surface->GetSelectionModel()
    );
    m_impl->m_inspector->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    m_impl->m_inspector->SetVerticalAlignment(layout::VerticalAlignment::Stretch);

    // 實例化垂直分類折疊式工具箱
    m_impl->m_toolbox = std::make_shared<toolbox::ToolboxPanel>();
    m_impl->m_toolbox->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    m_impl->m_toolbox->SetVerticalAlignment(layout::VerticalAlignment::Stretch);

    // 雙擊項目或拖曳放置新增至畫布
    m_impl->m_toolbox->OnItemDoubleClicked([this](NodeType type) {
        if (m_impl->m_surface) {
            (void)m_impl->m_surface->AddNewWidget(type, core::Point(60.0f, 60.0f));
        }
    });
    m_impl->m_toolbox->OnItemDropped([this](NodeType type, const core::Point& pt) {
        if (m_impl->m_surface) {
            (void)m_impl->m_surface->AddNewWidget(type, pt);
        }
    });

    // 實例化元件層級樹 ListBox
    m_impl->m_hierarchyBox = std::make_shared<controls::ListBox>();
    m_impl->m_hierarchyBox->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    m_impl->m_hierarchyBox->SetVerticalAlignment(layout::VerticalAlignment::Stretch);
    m_impl->m_hierarchyBox->SetItemHeight(24.0f);

    m_impl->m_hierarchyBox->OnSelectedIndexChanged([this](int32_t index) {
        if (m_impl->m_isSyncingHierarchy) return;
        if (index >= 0 && static_cast<size_t>(index) < m_impl->m_hierarchyUuids.size()) {
            m_impl->m_isSyncingHierarchy = true;
            const std::string& uuid = m_impl->m_hierarchyUuids[static_cast<size_t>(index)];
            if (uuid.empty()) {
                // 表單根節點
                m_impl->m_surface->GetSelectionModel().ClearSelection();
            } else {
                m_impl->m_surface->GetSelectionModel().SetSelection(uuid);
            }
            m_impl->m_isSyncingHierarchy = false;
            m_impl->UpdateStatus();
        }
    });

    // 建立 AST 拓撲
    m_impl->BuildDefaultAst();

    // 訂閱 AST 反應式信號通道 (動態維護階層樹與狀態列)
    m_impl->m_connections.push_back(
        m_impl->m_doc.NodeAddedSignal().Connect([this](const NodeAddedPayload&) {
            m_impl->RebuildHierarchyTree();
            m_impl->UpdateStatus();
        })
    );
    m_impl->m_connections.push_back(
        m_impl->m_doc.NodeRemovedSignal().Connect([this](const NodeRemovedPayload&) {
            m_impl->RebuildHierarchyTree();
            m_impl->UpdateStatus();
        })
    );
    m_impl->m_connections.push_back(
        m_impl->m_doc.NodeMovedSignal().Connect([this](const NodeMovedPayload&) {
            m_impl->RebuildHierarchyTree();
            m_impl->UpdateStatus();
        })
    );
    m_impl->m_connections.push_back(
        m_impl->m_doc.PropertyChangedSignal().Connect([this](const PropertyChangedPayload& p) {
            if (p.propertyName == "Name") {
                m_impl->RebuildHierarchyTree();
            }
            m_impl->UpdateStatus();
        })
    );
    m_impl->m_connections.push_back(
        m_impl->m_doc.SettingsChangedSignal().Connect([this](const FormSettingsChangedPayload&) {
            m_impl->RebuildHierarchyTree();
            m_impl->UpdateStatus();
        })
    );

    // 訂閱 SelectionModel 變更信號 (雙向同步至 ListBox)
    m_impl->m_connections.push_back(
        m_impl->m_surface->GetSelectionModel().SelectionChangedSignal().Connect([this](const std::vector<std::string>& selected) {
            if (m_impl->m_isSyncingHierarchy) return;
            m_impl->m_isSyncingHierarchy = true;
            if (selected.empty()) {
                m_impl->m_hierarchyBox->SetSelectedIndex(0);
            } else {
                const std::string& primaryUuid = selected.front();
                int32_t foundIdx = -1;
                for (size_t i = 0; i < m_impl->m_hierarchyUuids.size(); ++i) {
                    if (m_impl->m_hierarchyUuids[i] == primaryUuid) {
                        foundIdx = static_cast<int32_t>(i);
                        break;
                    }
                }
                m_impl->m_hierarchyBox->SetSelectedIndex(foundIdx);
            }
            m_impl->m_isSyncingHierarchy = false;
            m_impl->UpdateStatus();
        })
    );

    // 初次建置階層樹與狀態列
    m_impl->RebuildHierarchyTree();

    // 1. 頂部工具列 (Top Toolbar)
    m_impl->m_titleBlock = std::make_shared<controls::TextBlock>("💠 CPPUI Designer");
    m_impl->m_titleBlock->SetFontSize(16.0f);
    m_impl->m_titleBlock->SetFontWeight(text::FontWeight::Bold);
    m_impl->m_titleBlock->SetForeground(core::Color(80, 160, 255));
    m_impl->m_titleBlock->SetVerticalAlignment(layout::VerticalAlignment::Center);
    m_impl->m_titleBlock->SetMargin(core::Thickness(8.0f, 0.0f, 16.0f, 0.0f));

    auto undoBtn = std::make_shared<controls::Button>("↶ 復原");
    undoBtn->SetPadding(core::Thickness(10.0f, 5.0f, 10.0f, 5.0f));
    undoBtn->SetMargin(core::Thickness(2.0f));
    undoBtn->OnClick([this]() { (void)m_impl->m_surface->Undo(); });

    auto redoBtn = std::make_shared<controls::Button>("↷ 重做");
    redoBtn->SetPadding(core::Thickness(10.0f, 5.0f, 10.0f, 5.0f));
    redoBtn->SetMargin(core::Thickness(2.0f));
    redoBtn->OnClick([this]() { (void)m_impl->m_surface->Redo(); });

    auto deleteBtn = std::make_shared<controls::Button>("🗑 刪除");
    deleteBtn->SetPadding(core::Thickness(10.0f, 5.0f, 10.0f, 5.0f));
    deleteBtn->SetMargin(core::Thickness(2.0f));
    deleteBtn->OnClick([this]() { (void)m_impl->m_surface->DeleteSelected(); });

    // 字型縮放控制組 (A- / 100% / A+)
    auto fontMinusBtn = std::make_shared<controls::Button>("A-");
    fontMinusBtn->SetPadding(core::Thickness(8.0f, 5.0f, 8.0f, 5.0f));
    fontMinusBtn->SetMargin(core::Thickness(2.0f));

    auto fontResetBtn = std::make_shared<controls::Button>("100%");
    fontResetBtn->SetPadding(core::Thickness(8.0f, 5.0f, 8.0f, 5.0f));
    fontResetBtn->SetMargin(core::Thickness(2.0f));

    auto fontPlusBtn = std::make_shared<controls::Button>("A+");
    fontPlusBtn->SetPadding(core::Thickness(8.0f, 5.0f, 8.0f, 5.0f));
    fontPlusBtn->SetMargin(core::Thickness(2.0f));

    auto updateFontZoom = [this, fontResetBtn](float newScale) {
        m_impl->m_fontScale = compat::clamp(newScale, 0.8f, 1.5f);
        int32_t pct = static_cast<int32_t>(std::round(m_impl->m_fontScale * 100.0f));
        fontResetBtn->SetContent(std::to_string(pct) + "%");

        if (m_impl->m_toolbox) {
            m_impl->m_toolbox->SetFontScale(m_impl->m_fontScale);
        }
        if (m_impl->m_inspector) {
            m_impl->m_inspector->SetFontScale(m_impl->m_fontScale);
        }
        if (m_impl->m_surface) {
            m_impl->m_surface->SetFontScale(m_impl->m_fontScale);
        }
        if (m_impl->m_hierarchyBox) {
            m_impl->m_hierarchyBox->SetItemHeight(24.0f * m_impl->m_fontScale);
            m_impl->m_hierarchyBox->SetFontSize(12.0f * m_impl->m_fontScale);
        }
        if (m_impl->m_titleBlock) {
            m_impl->m_titleBlock->SetFontSize(16.0f * m_impl->m_fontScale);
        }
        if (m_impl->m_toolboxHeader) {
            m_impl->m_toolboxHeader->SetFontSize(14.5f * m_impl->m_fontScale);
        }
        if (m_impl->m_treeHeader) {
            m_impl->m_treeHeader->SetFontSize(14.5f * m_impl->m_fontScale);
        }
        if (m_impl->m_surfaceHeader) {
            m_impl->m_surfaceHeader->SetFontSize(14.5f * m_impl->m_fontScale);
        }
        if (m_impl->m_inspectorHeader) {
            m_impl->m_inspectorHeader->SetFontSize(14.5f * m_impl->m_fontScale);
        }
        if (m_impl->m_statusLabel) {
            m_impl->m_statusLabel->SetFontSize(12.0f * m_impl->m_fontScale);
        }
        if (m_impl->m_toolboxSearch) {
            m_impl->m_toolboxSearch->SetFontSize(12.0f * m_impl->m_fontScale);
            m_impl->m_toolboxSearch->SetHeight(28.0f * m_impl->m_fontScale);
        }
        if (m_impl->m_treeSearch) {
            m_impl->m_treeSearch->SetFontSize(12.0f * m_impl->m_fontScale);
            m_impl->m_treeSearch->SetHeight(28.0f * m_impl->m_fontScale);
        }

        InvalidateVisual();
        InvalidateForm();
    };

    fontMinusBtn->OnClick([this, updateFontZoom]() {
        updateFontZoom(m_impl->m_fontScale - 0.1f);
    });
    fontResetBtn->OnClick([updateFontZoom]() {
        updateFontZoom(1.0f);
    });
    fontPlusBtn->OnClick([this, updateFontZoom]() {
        updateFontZoom(m_impl->m_fontScale + 0.1f);
    });

    auto runBtn = std::make_shared<controls::Button>("▶ 預覽執行 (Live Run)");
    runBtn->SetPadding(core::Thickness(12.0f, 5.0f, 12.0f, 5.0f));
    runBtn->SetMargin(core::Thickness(6.0f, 2.0f, 2.0f, 2.0f));
    runBtn->SetBackground(core::Color(24, 134, 75));
    runBtn->SetForeground(core::Color(255, 255, 255));
    runBtn->OnClick([this]() { TriggerLiveRun(); });

    auto codeBtn = std::make_shared<controls::Button>("⚡ 程式碼預覽 (Code)");
    codeBtn->SetPadding(core::Thickness(12.0f, 5.0f, 12.0f, 5.0f));
    codeBtn->SetMargin(core::Thickness(2.0f));
    codeBtn->SetBackground(core::Color(0, 122, 204));
    codeBtn->SetForeground(core::Color(255, 255, 255));
    codeBtn->OnClick([this]() { TriggerCodePreview(); });

    auto themeBtn = std::make_shared<controls::Button>("🎨 主題");
    themeBtn->SetPadding(core::Thickness(10.0f, 5.0f, 10.0f, 5.0f));
    themeBtn->SetMargin(core::Thickness(2.0f));
    themeBtn->OnClick([this]() { ToggleTheme(); });

    auto exportBtn = std::make_shared<controls::Button>("📦 匯出 CMake");
    exportBtn->SetPadding(core::Thickness(12.0f, 5.0f, 12.0f, 5.0f));
    exportBtn->SetMargin(core::Thickness(2.0f));
    exportBtn->SetBackground(core::Color(130, 70, 210));
    exportBtn->SetForeground(core::Color(255, 255, 255));
    exportBtn->OnClick([this]() { TriggerExportProject(); });

    auto toolbar = HStack({
        m_impl->m_titleBlock,
        undoBtn,
        redoBtn,
        deleteBtn,
        fontMinusBtn,
        fontResetBtn,
        fontPlusBtn,
        runBtn,
        codeBtn,
        themeBtn,
        exportBtn
    })
    .Spacing(3)
    .Height(44)
    .Padding(8, 4)
    .Get();

    // 2. 左側欄 (工具箱與元件層級樹)
    m_impl->m_toolboxHeader = std::make_shared<controls::TextBlock>("🛠 控制項工具箱 (Toolbox)");
    m_impl->m_toolboxHeader->SetFontSize(14.5f);
    m_impl->m_toolboxHeader->SetFontWeight(text::FontWeight::Bold);
    m_impl->m_toolboxHeader->SetMargin(core::Thickness(4.0f, 6.0f, 4.0f, 2.0f));

    m_impl->m_toolboxSearch = std::make_shared<controls::TextBox>();
    m_impl->m_toolboxSearch->SetPlaceholder("🔍 搜尋控制項...");
    m_impl->m_toolboxSearch->SetFontSize(12.0f);
    m_impl->m_toolboxSearch->SetHeight(28.0f);
    m_impl->m_toolboxSearch->SetMargin(core::Thickness(2.0f, 2.0f, 2.0f, 4.0f));
    m_impl->m_toolboxSearch->OnTextChanged([this](compat::string_view text) {
        if (m_impl->m_toolbox) {
            m_impl->m_toolbox->SetFilterText(text);
        }
    });

    m_impl->m_treeHeader = std::make_shared<controls::TextBlock>("🌳 元件層級樹 (Hierarchy)");
    m_impl->m_treeHeader->SetFontSize(14.5f);
    m_impl->m_treeHeader->SetFontWeight(text::FontWeight::Bold);
    m_impl->m_treeHeader->SetMargin(core::Thickness(4.0f, 6.0f, 4.0f, 2.0f));

    m_impl->m_treeSearch = std::make_shared<controls::TextBox>();
    m_impl->m_treeSearch->SetPlaceholder("🔍 篩選元件樹...");
    m_impl->m_treeSearch->SetFontSize(12.0f);
    m_impl->m_treeSearch->SetHeight(28.0f);
    m_impl->m_treeSearch->SetMargin(core::Thickness(2.0f, 2.0f, 2.0f, 4.0f));
    m_impl->m_treeSearch->OnTextChanged([this](compat::string_view text) {
        m_impl->m_hierarchyFilter = std::string(text);
        m_impl->RebuildHierarchyTree();
    });

    auto leftSidebar = std::make_shared<layout::Grid>();
    leftSidebar->SetWidth(240.0f);
    leftSidebar->SetHorizontalAlignment(layout::HorizontalAlignment::Left);
    leftSidebar->SetVerticalAlignment(layout::VerticalAlignment::Stretch);
    leftSidebar->AddRow(layout::RowDefinition(layout::GridLength::Pixel(26.0f)));
    leftSidebar->AddRow(layout::RowDefinition(layout::GridLength::Pixel(30.0f)));
    leftSidebar->AddRow(layout::RowDefinition(layout::GridLength::Star(1.1f)));
    leftSidebar->AddRow(layout::RowDefinition(layout::GridLength::Pixel(26.0f)));
    leftSidebar->AddRow(layout::RowDefinition(layout::GridLength::Pixel(30.0f)));
    leftSidebar->AddRow(layout::RowDefinition(layout::GridLength::Star(0.9f)));

    layout::Grid::SetRow(*m_impl->m_toolboxHeader, 0);
    layout::Grid::SetRow(*m_impl->m_toolboxSearch, 1);
    layout::Grid::SetRow(*m_impl->m_toolbox, 2);
    layout::Grid::SetRow(*m_impl->m_treeHeader, 3);
    layout::Grid::SetRow(*m_impl->m_treeSearch, 4);
    layout::Grid::SetRow(*m_impl->m_hierarchyBox, 5);

    leftSidebar->AddChild(m_impl->m_toolboxHeader);
    leftSidebar->AddChild(m_impl->m_toolboxSearch);
    leftSidebar->AddChild(m_impl->m_toolbox);
    leftSidebar->AddChild(m_impl->m_treeHeader);
    leftSidebar->AddChild(m_impl->m_treeSearch);
    leftSidebar->AddChild(m_impl->m_hierarchyBox);

    // 3. 中央主工作區 (純畫布表面 - 完整開展自適應展開)
    m_impl->m_surfaceHeader = std::make_shared<controls::TextBlock>("🎨 設計畫布表面 (Design Surface - 800 x 520)");
    m_impl->m_surfaceHeader->SetFontSize(14.5f);
    m_impl->m_surfaceHeader->SetFontWeight(text::FontWeight::Bold);
    m_impl->m_surfaceHeader->SetMargin(core::Thickness(4.0f, 6.0f, 4.0f, 4.0f));

    auto centerArea = std::make_shared<layout::Grid>();
    centerArea->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    centerArea->SetVerticalAlignment(layout::VerticalAlignment::Stretch);
    centerArea->AddRow(layout::RowDefinition(layout::GridLength::Pixel(26.0f)));
    centerArea->AddRow(layout::RowDefinition(layout::GridLength::Star(1.0f)));

    layout::Grid::SetRow(*m_impl->m_surfaceHeader, 0);
    layout::Grid::SetRow(*m_impl->m_surface, 1);

    centerArea->AddChild(m_impl->m_surfaceHeader);
    centerArea->AddChild(m_impl->m_surface);

    // 4. 右側欄 (屬性與事件檢查器)
    m_impl->m_inspectorHeader = std::make_shared<controls::TextBlock>("⚙ 屬性與事件檢查器 (Inspector)");
    m_impl->m_inspectorHeader->SetFontSize(14.5f);
    m_impl->m_inspectorHeader->SetFontWeight(text::FontWeight::Bold);
    m_impl->m_inspectorHeader->SetMargin(core::Thickness(4.0f, 6.0f, 4.0f, 4.0f));

    auto rightSidebar = std::make_shared<layout::Grid>();
    rightSidebar->SetWidth(310.0f);
    rightSidebar->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    rightSidebar->SetVerticalAlignment(layout::VerticalAlignment::Stretch);
    rightSidebar->AddRow(layout::RowDefinition(layout::GridLength::Pixel(26.0f)));
    rightSidebar->AddRow(layout::RowDefinition(layout::GridLength::Star(1.0f)));

    layout::Grid::SetRow(*m_impl->m_inspectorHeader, 0);
    layout::Grid::SetRow(*m_impl->m_inspector, 1);

    rightSidebar->AddChild(m_impl->m_inspectorHeader);
    rightSidebar->AddChild(m_impl->m_inspector);

    // 5. 底部狀態列 (Bottom Status Bar)
    m_impl->m_statusLabel = std::make_shared<controls::Label>("");
    m_impl->m_statusLabel->SetFontSize(12.0f);
    m_impl->m_statusLabel->SetForeground(core::Color(160, 164, 175));
    m_impl->m_statusLabel->SetMargin(core::Thickness(12.0f, 4.0f, 12.0f, 4.0f));
    m_impl->UpdateStatus();

    // 主工作區 (三欄水平網格排版：左欄 240px，中欄 1* Star，右欄 310px)
    auto mainWorkspace = std::make_shared<layout::Grid>();
    mainWorkspace->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    mainWorkspace->SetVerticalAlignment(layout::VerticalAlignment::Stretch);
    mainWorkspace->AddColumn(layout::ColumnDefinition(layout::GridLength::Pixel(240.0f)));
    mainWorkspace->AddColumn(layout::ColumnDefinition(layout::GridLength::Star(1.0f)));
    mainWorkspace->AddColumn(layout::ColumnDefinition(layout::GridLength::Pixel(310.0f)));
    mainWorkspace->AddRow(layout::RowDefinition(layout::GridLength::Star(1.0f)));

    layout::Grid::SetColumn(*leftSidebar, 0);
    layout::Grid::SetColumn(*centerArea, 1);
    layout::Grid::SetColumn(*rightSidebar, 2);

    mainWorkspace->AddChild(leftSidebar);
    mainWorkspace->AddChild(centerArea);
    mainWorkspace->AddChild(rightSidebar);

    // 全視窗垂直網格排版 (頂部工具列 44px，中間工作區 1*，底部狀態列 28px)
    auto rootGrid = std::make_shared<layout::Grid>();
    rootGrid->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    rootGrid->SetVerticalAlignment(layout::VerticalAlignment::Stretch);
    rootGrid->AddRow(layout::RowDefinition(layout::GridLength::Pixel(44.0f)));
    rootGrid->AddRow(layout::RowDefinition(layout::GridLength::Star(1.0f)));
    rootGrid->AddRow(layout::RowDefinition(layout::GridLength::Pixel(28.0f)));

    layout::Grid::SetRow(*toolbar, 0);
    layout::Grid::SetRow(*mainWorkspace, 1);
    layout::Grid::SetRow(*m_impl->m_statusLabel, 2);

    rootGrid->AddChild(toolbar);
    rootGrid->AddChild(mainWorkspace);
    rootGrid->AddChild(m_impl->m_statusLabel);

    SetContent(rootGrid);
}

DesignerMainWindow::~DesignerMainWindow() = default;

FormDocumentNode& DesignerMainWindow::GetDocument() noexcept {
    return m_impl->m_doc;
}

void DesignerMainWindow::TriggerLiveRun() {
    auto liveForm = preview::LiveFormRunner::CreateForm(m_impl->m_doc);
    if (liveForm) {
        liveForm->SetTitle("【試運行 (Live Run)】" + m_impl->m_doc.GetSettings().className);
        liveForm->Show();
    }
}

void DesignerMainWindow::TriggerCodePreview() {
    if (!m_impl->m_codeWindow) {
        m_impl->m_codeWindow = std::make_shared<preview::CodePreviewWindow>(
            &m_impl->m_doc,
            &m_impl->m_surface->GetSelectionModel()
        );
    }
    m_impl->m_codeWindow->RefreshCode();
    m_impl->m_codeWindow->Show();
}

void DesignerMainWindow::ToggleTheme() {
    auto currentMode = theme::ThemeManager::Get().GetCurrentTheme().GetMode();
    if (currentMode == theme::ThemeMode::Dark) {
        theme::ThemeManager::Get().SetThemeMode(theme::ThemeMode::Light);
    } else {
        theme::ThemeManager::Get().SetThemeMode(theme::ThemeMode::Dark);
    }
}

void DesignerMainWindow::TriggerExportProject() {
    exporter::ProjectExportOptions opts;
    opts.outputDirectory = "./exported_designer_project";
    opts.projectName = "AccountSettingsSystem";
    opts.formClassName = m_impl->m_doc.GetSettings().className;
    opts.targetNamespace = m_impl->m_doc.GetSettings().targetNamespace;
    opts.incrementalUserCode = true;

    auto result = exporter::ProjectExporter::ExportProject(m_impl->m_doc, opts);
    if (result.success && m_impl->m_statusLabel) {
        m_impl->m_statusLabel->SetText("專案匯出成功！目標路徑: " + opts.outputDirectory);
    }
}

} // namespace tool
} // namespace designer
} // namespace cppui
