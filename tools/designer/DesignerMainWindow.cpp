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

struct DesignerMainWindow::Impl {
    FormDocumentNode m_doc{ CreateInitialSettings() };
    CommandHistoryManager m_history;
    std::shared_ptr<surface::DesignSurface> m_surface;
    std::shared_ptr<toolbox::ToolboxPanel> m_toolbox;
    std::shared_ptr<preview::CodePreviewWindow> m_codeWindow;
    std::shared_ptr<inspector::InspectorPanel> m_inspector;
    std::shared_ptr<controls::Label> m_statusLabel;

    std::string m_saveBtnUuid;
    std::string m_cancelBtnUuid;
    std::string m_userInputUuid;

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

    // 實例化畫布與檢查器
    m_impl->m_surface = std::make_shared<surface::DesignSurface>(&m_impl->m_doc, &m_impl->m_history);
    m_impl->m_surface->SetWidth(680.0f);
    m_impl->m_surface->SetHeight(700.0f);

    m_impl->m_inspector = std::make_shared<inspector::InspectorPanel>(
        &m_impl->m_doc,
        &m_impl->m_history,
        &m_impl->m_surface->GetSelectionModel()
    );
    m_impl->m_inspector->SetWidth(320.0f);
    m_impl->m_inspector->SetHeight(700.0f);

    // 實例化垂直分類折疊式工具箱
    m_impl->m_toolbox = std::make_shared<toolbox::ToolboxPanel>();
    m_impl->m_toolbox->SetWidth(230.0f);
    m_impl->m_toolbox->SetHeight(400.0f);

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

    // 建立 AST 拓撲
    m_impl->BuildDefaultAst();

    // 1. 頂部工具列 (Top Toolbar)
    auto titleBlock = std::make_shared<controls::TextBlock>("💠 CPPUI Designer");
    titleBlock->SetFontSize(15.0f);
    titleBlock->SetFontWeight(text::FontWeight::Bold);
    titleBlock->SetForeground(core::Color(80, 160, 255));
    titleBlock->SetVerticalAlignment(layout::VerticalAlignment::Center);
    titleBlock->SetMargin(core::Thickness(8.0f, 0.0f, 16.0f, 0.0f));

    auto undoBtn = std::make_shared<controls::Button>("↶ 復原");
    undoBtn->SetPadding(core::Thickness(10.0f, 5.0f, 10.0f, 5.0f));
    undoBtn->SetMargin(core::Thickness(2.0f));
    undoBtn->OnClick([this]() { (void)m_impl->m_surface->Undo(); });

    auto redoBtn = std::make_shared<controls::Button>("↷ 重做");
    redoBtn->SetPadding(core::Thickness(10.0f, 5.0f, 10.0f, 5.0f));
    redoBtn->SetMargin(core::Thickness(2.0f));
    redoBtn->OnClick([this]() { (void)m_impl->m_surface->Redo(); });

    auto deleteBtn = std::make_shared<controls::Button>("🗑 刪除元件");
    deleteBtn->SetPadding(core::Thickness(10.0f, 5.0f, 10.0f, 5.0f));
    deleteBtn->SetMargin(core::Thickness(2.0f));
    deleteBtn->OnClick([this]() { (void)m_impl->m_surface->DeleteSelected(); });

    auto runBtn = std::make_shared<controls::Button>("▶ 預覽執行 (Live Run)");
    runBtn->SetPadding(core::Thickness(12.0f, 5.0f, 12.0f, 5.0f));
    runBtn->SetMargin(core::Thickness(8.0f, 2.0f, 2.0f, 2.0f));
    runBtn->SetBackground(core::Color(24, 134, 75));
    runBtn->SetForeground(core::Color(255, 255, 255));
    runBtn->OnClick([this]() { TriggerLiveRun(); });

    auto codeBtn = std::make_shared<controls::Button>("⚡ 程式碼預覽 (Code)");
    codeBtn->SetPadding(core::Thickness(12.0f, 5.0f, 12.0f, 5.0f));
    codeBtn->SetMargin(core::Thickness(2.0f));
    codeBtn->SetBackground(core::Color(0, 122, 204));
    codeBtn->SetForeground(core::Color(255, 255, 255));
    codeBtn->OnClick([this]() { TriggerCodePreview(); });

    auto themeBtn = std::make_shared<controls::Button>("🎨 切換主題");
    themeBtn->SetPadding(core::Thickness(10.0f, 5.0f, 10.0f, 5.0f));
    themeBtn->SetMargin(core::Thickness(2.0f));
    themeBtn->OnClick([this]() { ToggleTheme(); });

    auto exportBtn = std::make_shared<controls::Button>("📦 匯出 CMake 專案");
    exportBtn->SetPadding(core::Thickness(12.0f, 5.0f, 12.0f, 5.0f));
    exportBtn->SetMargin(core::Thickness(2.0f));
    exportBtn->SetBackground(core::Color(130, 70, 210));
    exportBtn->SetForeground(core::Color(255, 255, 255));
    exportBtn->OnClick([this]() { TriggerExportProject(); });

    auto toolbar = HStack({
        titleBlock,
        undoBtn,
        redoBtn,
        deleteBtn,
        runBtn,
        codeBtn,
        themeBtn,
        exportBtn
    })
    .Spacing(4)
    .Height(42)
    .Padding(8, 4)
    .Get();

    // 2. 左側欄 (工具箱與元件層級樹)
    auto toolboxHeader = std::make_shared<controls::TextBlock>("🛠 控制項工具箱 (Toolbox)");
    toolboxHeader->SetFontSize(13.0f);
    toolboxHeader->SetFontWeight(text::FontWeight::Bold);
    toolboxHeader->SetMargin(core::Thickness(4.0f, 6.0f, 4.0f, 4.0f));

    auto treeHeader = std::make_shared<controls::TextBlock>("🌳 元件層級樹 (Hierarchy)");
    treeHeader->SetFontSize(13.0f);
    treeHeader->SetFontWeight(text::FontWeight::Bold);
    treeHeader->SetMargin(core::Thickness(4.0f, 10.0f, 4.0f, 4.0f));

    auto hierarchyBox = std::make_shared<controls::ListBox>();
    hierarchyBox->SetHeight(270.0f);
    hierarchyBox->AddItem("▾ 📄 AccountSettingsForm (Form)");
    hierarchyBox->AddItem("   ▾ 🎨 mainCanvas (Canvas)");
    hierarchyBox->AddItem("      🏷 titleLabel (Label)");
    hierarchyBox->AddItem("      🏷 userLabel (Label)");
    hierarchyBox->AddItem("      🔤 usernameInput (TextBox)");
    hierarchyBox->AddItem("      🏷 emailLabel (Label)");
    hierarchyBox->AddItem("      🔤 emailInput (TextBox)");
    hierarchyBox->AddItem("      ☑ authCheck (CheckBox)");
    hierarchyBox->AddItem("      🔘 saveBtn (Button)  ★ 作用中");
    hierarchyBox->AddItem("      🔘 cancelBtn (Button)");
    hierarchyBox->SetSelectedIndex(8);

    auto leftSidebar = VStack({
        toolboxHeader,
        m_impl->m_toolbox,
        treeHeader,
        hierarchyBox
    })
    .Width(230)
    .Padding(6)
    .Get();

    // 3. 中央主工作區 (純畫布表面 - 完整開展不被預覽壓縮)
    auto surfaceHeader = std::make_shared<controls::TextBlock>("🎨 設計畫布表面 (Design Surface - 800 x 520)");
    surfaceHeader->SetFontSize(13.0f);
    surfaceHeader->SetFontWeight(text::FontWeight::Bold);
    surfaceHeader->SetMargin(core::Thickness(4.0f, 6.0f, 4.0f, 4.0f));

    auto centerArea = VStack({
        surfaceHeader,
        m_impl->m_surface
    })
    .Width(680)
    .Padding(6)
    .Get();

    // 4. 右側欄 (屬性與事件檢查器)
    auto inspectorHeader = std::make_shared<controls::TextBlock>("⚙ 屬性與事件檢查器 (Inspector)");
    inspectorHeader->SetFontSize(13.0f);
    inspectorHeader->SetFontWeight(text::FontWeight::Bold);
    inspectorHeader->SetMargin(core::Thickness(4.0f, 6.0f, 4.0f, 4.0f));

    auto rightSidebar = VStack({
        inspectorHeader,
        m_impl->m_inspector
    })
    .Width(320)
    .Padding(6)
    .Get();

    // 5. 底部狀態列 (Bottom Status Bar)
    m_impl->m_statusLabel = std::make_shared<controls::Label>(
        "狀態: 就緒 | 當前選取: saveBtn (Button) | 位置: (170, 190) 尺寸: 130 x 34 | 導引線: 啟用 | 歷史: 0 步"
    );
    m_impl->m_statusLabel->SetFontSize(11.0f);
    m_impl->m_statusLabel->SetForeground(core::Color(160, 164, 175));
    m_impl->m_statusLabel->SetMargin(core::Thickness(12.0f, 4.0f, 12.0f, 4.0f));

    // 主工作區 (三欄水平排版)
    auto mainWorkspace = HStack({
        leftSidebar,
        centerArea,
        rightSidebar
    })
    .Get();

    // 全視窗垂直堆疊排版
    SetContent(
        VStack({
            toolbar,
            mainWorkspace,
            m_impl->m_statusLabel
        })
        .Get()
    );
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
