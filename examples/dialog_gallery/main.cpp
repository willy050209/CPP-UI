/**
 * @file main.cpp
 * @brief CPPUI 對話框展示庫範例程式 (Dialog Gallery)
 * 展示 MessageBox、OpenFileDialog 與 SaveFileDialog 在 Auto / Native / BuiltIn 模式與深淺主題下的互動。
 */

#include "compat/compat.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/Form.hpp"
#include "ui/dialogs/DialogCommon.hpp"
#include "ui/dialogs/MessageBox.hpp"
#include "ui/dialogs/OpenFileDialog.hpp"
#include "ui/dialogs/SaveFileDialog.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/theme/ThemeManager.hpp"
#include <memory>
#include <string>

namespace {

class DialogGalleryForm : public cppui::app::Form {
public:
    DialogGalleryForm() {
        SetTitle("CPPUI 對話框展示庫 (Dialog Gallery)");
        SetFormSize(820, 620);
        SetAutoSize(true);
        SetAutoSizeMode(cppui::app::AutoSizeMode::GrowOnly);
        SetStartPosition(cppui::app::FormStartPosition::CenterScreen);

        BuildUI();
    }

private:
    void BuildUI() {
        using namespace cppui;
        using namespace cppui::controls;
        using namespace cppui::layout;
        using namespace cppui::dialogs;
        using namespace cppui::theme;

        const auto& theme = ThemeManager::Get().GetCurrentTheme();
        const auto& colors = theme.Colors();
        const auto& metrics = theme.Metrics();

        auto rootBorder = std::make_shared<Border>();
        rootBorder->SetBackground(colors.windowBackground);
        rootBorder->SetBorderBrush(colors.surfaceBorder);
        rootBorder->SetBorderThickness(metrics.borderThicknessDefault);
        rootBorder->SetPadding(core::Thickness(20.0f, 20.0f, 20.0f, 20.0f));

        auto mainStack = std::make_shared<StackPanel>(Orientation::Vertical);
        mainStack->SetSpacing(16.0f);

        // --- 標頭列與設定列 ---
        auto headerStack = std::make_shared<StackPanel>(Orientation::Horizontal);
        headerStack->SetSpacing(12.0f);

        auto titleBlock = std::make_shared<TextBlock>("對話框展示庫 (Dialog Gallery)");
        titleBlock->SetFontSize(20.0f);
        titleBlock->SetForeground(colors.textPrimary);
        titleBlock->SetWidth(360.0f);
        headerStack->AddChild(titleBlock);

        // 切換主題按鈕
        m_themeBtn = std::make_shared<Button>("切換主題 (Dark / Light)");
        m_themeBtn->SetWidth(200.0f);
        m_themeBtn->SetHeight(32.0f);
        m_themeBtn->OnClick([]() {
            auto currentMode = ThemeManager::Get().GetCurrentTheme().GetMode();
            if (currentMode == ThemeMode::Dark) {
                ThemeManager::Get().SetThemeMode(ThemeMode::Light);
            } else {
                ThemeManager::Get().SetThemeMode(ThemeMode::Dark);
            }
        });
        headerStack->AddChild(m_themeBtn);

        // 切換後端模式按鈕
        m_backendBtn = std::make_shared<Button>("後端模式: Auto");
        m_backendBtn->SetWidth(180.0f);
        m_backendBtn->SetHeight(32.0f);
        m_backendBtn->OnClick([this]() {
            if (m_currentMode == DialogBackendMode::Auto) {
                m_currentMode = DialogBackendMode::BuiltIn;
                m_backendBtn->SetContent("後端模式: BuiltIn (CPPUI)");
            } else if (m_currentMode == DialogBackendMode::BuiltIn) {
                m_currentMode = DialogBackendMode::Native;
                m_backendBtn->SetContent("後端模式: Native (OS)");
            } else {
                m_currentMode = DialogBackendMode::Auto;
                m_backendBtn->SetContent("後端模式: Auto");
            }
        });
        headerStack->AddChild(m_backendBtn);

        mainStack->AddChild(headerStack);

        // --- 第一區塊：MessageBox 展示 ---
        auto mbHeader = std::make_shared<TextBlock>("1. 模態訊息方塊 (MessageBox)");
        mbHeader->SetFontSize(16.0f);
        mbHeader->SetForeground(colors.accentPrimary);
        mainStack->AddChild(mbHeader);

        auto mbRow = std::make_shared<StackPanel>(Orientation::Horizontal);
        mbRow->SetSpacing(10.0f);

        auto infoBtn = std::make_shared<Button>("提示訊息 (Info)");
        infoBtn->SetWidth(140.0f);
        infoBtn->SetHeight(34.0f);
        infoBtn->OnClick([this]() {
            auto res = MessageBox::ShowWithMode(
                m_currentMode, nullptr,
                "歡迎使用 CPPUI 現代化桌面對話框！",
                "系統提示",
                MessageBoxButtons::OK,
                MessageBoxIcon::Information
            );
            UpdateResultText("MessageBox (Info)", res);
        });
        mbRow->AddChild(infoBtn);

        auto warnBtn = std::make_shared<Button>("警告訊息 (Warning)");
        warnBtn->SetWidth(140.0f);
        warnBtn->SetHeight(34.0f);
        warnBtn->OnClick([this]() {
            auto res = MessageBox::ShowWithMode(
                m_currentMode, nullptr,
                "磁碟空間低於 10%，請盡速清理。",
                "系統警告",
                MessageBoxButtons::OKCancel,
                MessageBoxIcon::Warning
            );
            UpdateResultText("MessageBox (Warning)", res);
        });
        mbRow->AddChild(warnBtn);

        auto errBtn = std::make_shared<Button>("錯誤訊息 (Error)");
        errBtn->SetWidth(140.0f);
        errBtn->SetHeight(34.0f);
        errBtn->OnClick([this]() {
            auto res = MessageBox::ShowWithMode(
                m_currentMode, nullptr,
                "連線逾時，遠端伺服器未回應！",
                "連線錯誤",
                MessageBoxButtons::RetryCancel,
                MessageBoxIcon::Error
            );
            UpdateResultText("MessageBox (Error)", res);
        });
        mbRow->AddChild(errBtn);

        auto qBtn = std::make_shared<Button>("確認方塊 (Question)");
        qBtn->SetWidth(150.0f);
        qBtn->SetHeight(34.0f);
        qBtn->OnClick([this]() {
            auto res = MessageBox::ShowWithMode(
                m_currentMode, nullptr,
                "即將套用變更並重新啟動，是否繼續？",
                "確認操作",
                MessageBoxButtons::YesNoCancel,
                MessageBoxIcon::Question
            );
            UpdateResultText("MessageBox (Question)", res);
        });
        mbRow->AddChild(qBtn);

        mainStack->AddChild(mbRow);

        // --- 第二區塊：檔案對話框展示 ---
        auto fdHeader = std::make_shared<TextBlock>("2. 檔案對話框 (OpenFileDialog & SaveFileDialog)");
        fdHeader->SetFontSize(16.0f);
        fdHeader->SetForeground(colors.accentPrimary);
        mainStack->AddChild(fdHeader);

        auto fdRow = std::make_shared<StackPanel>(Orientation::Horizontal);
        fdRow->SetSpacing(10.0f);

        auto openFileBtn = std::make_shared<Button>("開啟檔案 (單選)");
        openFileBtn->SetWidth(150.0f);
        openFileBtn->SetHeight(34.0f);
        openFileBtn->OnClick([this]() {
            OpenFileDialog ofd;
            ofd.SetTitle("請選取欲開啟的檔案");
            ofd.SetFilter("純文字檔 (*.txt)|*.txt|圖片檔案 (*.png;*.jpg)|*.png;*.jpg|所有檔案 (*.*)|*.*");
            ofd.SetBackendMode(m_currentMode);
            auto res = ofd.ShowDialog();
            if (res == app::DialogResult::OK) {
                UpdatePathText("開啟檔案", ofd.GetFileName());
            }
            UpdateResultText("OpenFileDialog (Single)", res);
        });
        fdRow->AddChild(openFileBtn);

        auto openMultiBtn = std::make_shared<Button>("開啟多檔 (Multiselect)");
        openMultiBtn->SetWidth(170.0f);
        openMultiBtn->SetHeight(34.0f);
        openMultiBtn->OnClick([this]() {
            OpenFileDialog ofd;
            ofd.SetTitle("請選取多個檔案");
            ofd.SetMultiselect(true);
            ofd.SetFilter("所有檔案 (*.*)|*.*");
            ofd.SetBackendMode(m_currentMode);
            auto res = ofd.ShowDialog();
            if (res == app::DialogResult::OK) {
                std::string combined;
                for (const auto& path : ofd.GetFileNames()) {
                    if (!combined.empty()) combined += ", ";
                    combined += path;
                }
                UpdatePathText("開啟多檔", combined);
            }
            UpdateResultText("OpenFileDialog (Multi)", res);
        });
        fdRow->AddChild(openMultiBtn);

        auto saveFileBtn = std::make_shared<Button>("儲存檔案 (SaveFileDialog)");
        saveFileBtn->SetWidth(180.0f);
        saveFileBtn->SetHeight(34.0f);
        saveFileBtn->OnClick([this]() {
            SaveFileDialog sfd;
            sfd.SetTitle("另存新檔");
            sfd.SetDefaultExt("txt");
            sfd.SetFilter("文字文件 (*.txt)|*.txt|所有檔案 (*.*)|*.*");
            sfd.SetOverwritePrompt(true);
            sfd.SetBackendMode(m_currentMode);
            auto res = sfd.ShowDialog();
            if (res == app::DialogResult::OK) {
                UpdatePathText("儲存檔案", sfd.GetFileName());
            }
            UpdateResultText("SaveFileDialog", res);
        });
        fdRow->AddChild(saveFileBtn);

        mainStack->AddChild(fdRow);

        // --- 第三區塊：結果狀態回顯區塊 ---
        auto resBorder = std::make_shared<Border>();
        resBorder->SetBackground(colors.surfaceBackground);
        resBorder->SetBorderBrush(colors.surfaceBorder);
        resBorder->SetBorderThickness(metrics.borderThicknessDefault);
        resBorder->SetCornerRadius(metrics.cornerRadiusSmall);
        resBorder->SetPadding(core::Thickness(12.0f, 12.0f, 12.0f, 12.0f));

        auto resStack = std::make_shared<StackPanel>(Orientation::Vertical);
        resStack->SetSpacing(8.0f);

        m_statusTextBlock = std::make_shared<TextBlock>("最近對話框回傳結果: (尚未觸發)");
        m_statusTextBlock->SetForeground(colors.textPrimary);
        resStack->AddChild(m_statusTextBlock);

        m_pathTextBlock = std::make_shared<TextBlock>("選取的路徑內容: (無)");
        m_pathTextBlock->SetForeground(colors.textSecondary);
        resStack->AddChild(m_pathTextBlock);

        resBorder->SetChild(resStack);
        mainStack->AddChild(resBorder);

        rootBorder->SetChild(mainStack);
        SetContent(rootBorder);
    }

    void UpdateResultText(compat::string_view from, cppui::app::DialogResult res) {
        std::string resStr;
        switch (res) {
        case cppui::app::DialogResult::OK:     resStr = "OK (確定)"; break;
        case cppui::app::DialogResult::Cancel: resStr = "Cancel (取消)"; break;
        case cppui::app::DialogResult::Yes:    resStr = "Yes (是)"; break;
        case cppui::app::DialogResult::No:     resStr = "No (否)"; break;
        case cppui::app::DialogResult::Retry:  resStr = "Retry (重試)"; break;
        case cppui::app::DialogResult::Abort:  resStr = "Abort (中止)"; break;
        case cppui::app::DialogResult::Ignore: resStr = "Ignore (略過)"; break;
        default:                               resStr = "None"; break;
        }

        std::string text = "最近操作: [" + std::string(from) + "] => 回傳結果: " + resStr;
        if (m_statusTextBlock) {
            m_statusTextBlock->SetText(text);
        }
    }

    void UpdatePathText(compat::string_view action, compat::string_view path) {
        std::string text = "選取結果 (" + std::string(action) + "): " + std::string(path);
        if (m_pathTextBlock) {
            m_pathTextBlock->SetText(text);
        }
    }

    cppui::dialogs::DialogBackendMode m_currentMode = cppui::dialogs::DialogBackendMode::Auto;
    std::shared_ptr<cppui::controls::Button> m_themeBtn;
    std::shared_ptr<cppui::controls::Button> m_backendBtn;
    std::shared_ptr<cppui::controls::TextBlock> m_statusTextBlock;
    std::shared_ptr<cppui::controls::TextBlock> m_pathTextBlock;
};

} // namespace

int main() {
    return cppui::app::Application::Run<DialogGalleryForm>();
}
