/**
 * @file main.cpp
 * @brief CPPUI 彈出層與浮動控制項展示庫 (Popup Gallery)
 * 展示 OverlayLayer、Popup、ToolTip、ContextMenu 與 ComboBox 之智慧定位、
 * 自動邊界翻轉 (Auto-Flip) 及外部點擊收合 (Light-Dismiss)。
 */

#include "compat/Compat.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/Form.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/ComboBox.hpp"
#include "ui/controls/ContextMenu.hpp"
#include "ui/controls/MenuItem.hpp"
#include "ui/controls/Separator.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/controls/ToolTip.hpp"
#include "ui/controls/ToolTipService.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/theme/ThemeManager.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace {

/// <summary>
/// 彈出層展示視窗 (PopupGalleryForm)
/// 整合展示 ToolTip、ContextMenu 與 ComboBox 的互動操作。
/// </summary>
class PopupGalleryForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構子，初始化視窗與控制項。
    /// </summary>
    PopupGalleryForm() {
        SetTitle("CPPUI 彈出層與浮動控制項展示 (Popup & Floating Controls Gallery)");
        SetFormSize(820, 680);
        SetAutoSize(true);
        SetAutoSizeMode(cppui::app::AutoSizeMode::GrowOnly);
        SetStartPosition(cppui::app::FormStartPosition::CenterScreen);

        BuildUI();
    }

private:
    /// <summary>
    /// 構建展示介面與事件綁定。
    /// </summary>
    void BuildUI() {
        using namespace cppui;
        using namespace cppui::controls;
        using namespace cppui::layout;
        using namespace cppui::theme;

        const auto& theme = ThemeManager::Get().GetCurrentTheme();
        const auto& colors = theme.Colors();
        const auto& metrics = theme.Metrics();

        auto rootBorder = std::make_shared<Border>();
        rootBorder->SetBackground(colors.windowBackground);
        rootBorder->SetBorderBrush(colors.surfaceBorder);
        rootBorder->SetBorderThickness(metrics.borderThicknessDefault);
        rootBorder->SetPadding(core::Thickness(24.0f, 20.0f, 24.0f, 20.0f));

        auto mainStack = std::make_shared<StackPanel>(Orientation::Vertical);
        mainStack->SetSpacing(16.0f);

        // --- 1. 標頭與主題切換 ---
        auto headerStack = std::make_shared<StackPanel>(Orientation::Horizontal);
        headerStack->SetSpacing(12.0f);

        auto titleBlock = std::make_shared<TextBlock>("浮動彈出層展示 (Popup Gallery)");
        titleBlock->SetFontSize(20.0f);
        titleBlock->SetForeground(colors.textPrimary);
        titleBlock->SetWidth(450.0f);
        headerStack->AddChild(titleBlock);

        auto themeBtn = std::make_shared<Button>("切換主題 (Dark / Light)");
        themeBtn->SetWidth(200.0f);
        themeBtn->SetHeight(32.0f);
        themeBtn->OnClick([]() {
            auto currentMode = ThemeManager::Get().GetCurrentTheme().GetMode();
            if (currentMode == ThemeMode::Dark) {
                ThemeManager::Get().SetThemeMode(ThemeMode::Light);
            } else {
                ThemeManager::Get().SetThemeMode(ThemeMode::Dark);
            }
        });
        headerStack->AddChild(themeBtn);

        mainStack->AddChild(headerStack);

        // --- 2. 狀態回饋文字列 ---
        m_statusText = std::make_shared<TextBlock>("目前狀態：請操作下方控制項體驗浮層效果。");
        m_statusText->SetFontSize(13.0f);
        m_statusText->SetForeground(colors.textSecondary);
        mainStack->AddChild(m_statusText);

        auto sep1 = std::make_shared<Separator>();
        mainStack->AddChild(sep1);

        // --- 3. ToolTip 懸停提示展示 ---
        auto ttHeader = std::make_shared<TextBlock>("1. 懸停提示 (ToolTip & ToolTipService)");
        ttHeader->SetFontSize(15.0f);
        ttHeader->SetForeground(colors.textPrimary);
        mainStack->AddChild(ttHeader);

        auto ttRow = std::make_shared<StackPanel>(Orientation::Horizontal);
        ttRow->SetSpacing(16.0f);

        auto btnTt1 = std::make_shared<Button>("懸停以檢視提示 1");
        btnTt1->SetWidth(180.0f);
        btnTt1->SetHeight(32.0f);
        ToolTipService::SetToolTip(btnTt1.get(), "這是標準的 ToolTip 浮層提示訊息！");
        ttRow->AddChild(btnTt1);

        auto btnTt2 = std::make_shared<Button>("多行詳細提示 2");
        btnTt2->SetWidth(180.0f);
        btnTt2->SetHeight(32.0f);
        ToolTipService::SetToolTip(btnTt2.get(), "支援智慧邊界翻轉與自適應主題對比色的 ToolTip。\n點擊視窗任意處或移開滑鼠即可自動隱藏。");
        ttRow->AddChild(btnTt2);

        mainStack->AddChild(ttRow);

        auto sep2 = std::make_shared<Separator>();
        mainStack->AddChild(sep2);

        // --- 4. ContextMenu 右鍵選單與 Submenu 展示 ---
        auto cmHeader = std::make_shared<TextBlock>("2. 右鍵選單與巢狀次級選單 (ContextMenu & Submenu)");
        cmHeader->SetFontSize(15.0f);
        cmHeader->SetForeground(colors.textPrimary);
        mainStack->AddChild(cmHeader);

        auto cmRow = std::make_shared<StackPanel>(Orientation::Horizontal);
        cmRow->SetSpacing(16.0f);

        auto btnMenu = std::make_shared<Button>("在此按鈕按右鍵開啟上下文選單");
        btnMenu->SetWidth(280.0f);
        btnMenu->SetHeight(36.0f);

        // 構建 ContextMenu
        auto contextMenu = std::make_shared<ContextMenu>();

        auto itemNew = std::make_shared<MenuItem>("新增檔案 (New File)");
        itemNew->SetInputGestureText("Ctrl+N");
        itemNew->AddClickHandler([this](MenuItem&) {
            UpdateStatus("選單操作：觸發 [新增檔案]");
        });
        contextMenu->AddMenuItem(itemNew);

        auto itemOpen = std::make_shared<MenuItem>("開啟檔案 (Open...)");
        itemOpen->SetInputGestureText("Ctrl+O");
        itemOpen->AddClickHandler([this](MenuItem&) {
            UpdateStatus("選單操作：觸發 [開啟檔案]");
        });
        contextMenu->AddMenuItem(itemOpen);

        // 次級選單 (Submenu)
        auto itemExport = std::make_shared<MenuItem>("匯出為 (Export)");
        auto itemPng = std::make_shared<MenuItem>("PNG 點陣圖");
        itemPng->AddClickHandler([this](MenuItem&) {
            UpdateStatus("選單操作：匯出格式 -> [PNG 點陣圖]");
        });
        auto itemSvg = std::make_shared<MenuItem>("SVG 向量圖");
        itemSvg->AddClickHandler([this](MenuItem&) {
            UpdateStatus("選單操作：匯出格式 -> [SVG 向量圖]");
        });
        auto itemPdf = std::make_shared<MenuItem>("PDF 文件");
        itemPdf->AddClickHandler([this](MenuItem&) {
            UpdateStatus("選單操作：匯出格式 -> [PDF 文件]");
        });
        itemExport->AddMenuItem(itemPng);
        itemExport->AddMenuItem(itemSvg);
        itemExport->AddMenuItem(itemPdf);
        contextMenu->AddMenuItem(itemExport);

        auto itemExit = std::make_shared<MenuItem>("關閉選單 (Exit)");
        itemExit->AddClickHandler([this](MenuItem&) {
            UpdateStatus("選單操作：點擊 [關閉選單]");
        });
        contextMenu->AddMenuItem(itemExit);

        // 綁定至按鈕右鍵
        ContextMenuService::SetContextMenu(btnMenu.get(), contextMenu);
        cmRow->AddChild(btnMenu);

        mainStack->AddChild(cmRow);

        auto sep3 = std::make_shared<Separator>();
        mainStack->AddChild(sep3);

        // --- 5. ComboBox 下拉選單展示 ---
        auto cbHeader = std::make_shared<TextBlock>("3. 下拉選單 (ComboBox) 與智慧邊界防溢出 (Auto-Flip)");
        cbHeader->SetFontSize(15.0f);
        cbHeader->SetForeground(colors.textPrimary);
        mainStack->AddChild(cbHeader);

        auto cbRow = std::make_shared<StackPanel>(Orientation::Horizontal);
        cbRow->SetSpacing(20.0f);

        auto cbLabel1 = std::make_shared<TextBlock>("編譯標準：");
        cbLabel1->SetWidth(80.0f);
        cbRow->AddChild(cbLabel1);

        auto comboStd = std::make_shared<ComboBox>();
        comboStd->SetWidth(160.0f);
        comboStd->SetHeight(32.0f);
        comboStd->AddItem("ISO C++14");
        comboStd->AddItem("ISO C++17");
        comboStd->AddItem("ISO C++20");
        comboStd->AddItem("ISO C++23");
        comboStd->SetSelectedIndex(2);
        comboStd->AddSelectionChangedHandler([this](ComboBox&, int32_t index) {
            static const char* const s_names[] = { "ISO C++14", "ISO C++17", "ISO C++20", "ISO C++23" };
            if (index >= 0 && index < 4) {
                UpdateStatus(std::string("下拉選取變更：編譯標準 -> ") + s_names[index]);
            }
        });
        cbRow->AddChild(comboStd);

        mainStack->AddChild(cbRow);

        // 置底 ComboBox (測試在視窗下緣展開時自動向上翻轉)
        auto bottomSection = std::make_shared<StackPanel>(Orientation::Vertical);
        bottomSection->SetSpacing(8.0f);

        auto cbLabel2 = std::make_shared<TextBlock>("視窗底部下拉測試 (向下空間不足時自動向上展開)：");
        cbLabel2->SetForeground(colors.textSecondary);
        bottomSection->AddChild(cbLabel2);

        auto comboBottom = std::make_shared<ComboBox>();
        comboBottom->SetWidth(260.0f);
        comboBottom->SetHeight(32.0f);
        comboBottom->AddItem("最佳化等級: -O0 (無最佳化)");
        comboBottom->AddItem("最佳化等級: -O2 (生產發布)");
        comboBottom->AddItem("最佳化等級: -O3 (極限效能)");
        comboBottom->AddItem("最佳化等級: -Oz (體積最小化)");
        comboBottom->SetSelectedIndex(1);
        comboBottom->AddSelectionChangedHandler([this](ComboBox&, int32_t index) {
            UpdateStatus(std::string("置底選單變更：索引 ") + std::to_string(index));
        });
        bottomSection->AddChild(comboBottom);

        mainStack->AddChild(bottomSection);

        rootBorder->SetChild(mainStack);
        SetContent(rootBorder);
    }

    /// <summary>
    /// 更新狀態列字串。
    /// </summary>
    /// <param name="msg">狀態訊息字串視圖。</param>
    void UpdateStatus(compat::string_view msg) {
        if (m_statusText) {
            m_statusText->SetText(msg);
        }
    }

    std::shared_ptr<cppui::controls::TextBlock> m_statusText;
};

} // namespace

/// <summary>
/// 應用程式進入點。
/// </summary>
/// <param name="argc">引數個數。</param>
/// <param name="argv">引數字串陣列。</param>
/// <returns>處理程序結束代碼。</returns>
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    auto form = std::make_shared<PopupGalleryForm>();
    return cppui::app::Application::Run(form);
}
