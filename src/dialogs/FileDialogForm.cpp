/**
 * @file FileDialogForm.cpp
 * @brief 實作純 CPPUI 主題渲染之檔案選擇對話方塊視窗 (FileDialogForm)
 */

#include "ui/dialogs/FileDialogForm.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/controls/ScrollViewer.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/dialogs/MessageBox.hpp"
#include "ui/theme/ThemeManager.hpp"
#include <system_error>

namespace cppui {
namespace dialogs {

FileDialogForm::FileDialogForm(FileDialogFormMode mode,
                               compat::string_view title,
                               compat::string_view initialDir,
                               compat::string_view fileName,
                               compat::string_view filter,
                               uint32_t filterIndex,
                               compat::string_view defaultExt,
                               bool multiselect,
                               bool overwritePrompt,
                               bool checkFileExists,
                               bool checkPathExists)
    : m_mode(mode)
    , m_selectedFileName(fileName)
    , m_defaultExt(defaultExt)
    , m_multiselect(multiselect)
    , m_overwritePrompt(overwritePrompt)
    , m_checkFileExists(checkFileExists)
    , m_checkPathExists(checkPathExists) {
    // 決定起始目錄
    std::error_code ec;
    if (!initialDir.empty() && compat::filesystem::exists(std::string(initialDir), ec)) {
        m_currentDir = compat::filesystem::path(std::string(initialDir));
    } else {
        m_currentDir = compat::filesystem::current_path(ec);
        if (ec) {
            m_currentDir = compat::filesystem::path(".");
        }
    }

    // 決定標題
    std::string defaultTitle = (m_mode == FileDialogFormMode::Open) ? "開啟檔案" : "儲存檔案";
    SetTitle(title.empty() ? defaultTitle : std::string(title));
    SetFormSize(720, 500);
    SetStartPosition(app::FormStartPosition::CenterScreen);

    // 解析篩選器
    m_filterItems = DialogUtilities::ParseFilterString(filter);
    if (m_filterItems.empty()) {
        FileFilterItem allFiles;
        allFiles.description = "所有檔案 (*.*)";
        allFiles.patterns = { "*.*" };
        m_filterItems.push_back(allFiles);
    }
    if (filterIndex > 0 && filterIndex <= m_filterItems.size()) {
        m_currentFilterIndex = filterIndex - 1;
    } else {
        m_currentFilterIndex = 0;
    }

    BuildUI();
}

void FileDialogForm::OnLoad() {
    app::Form::OnLoad();
    ApplyTheme(theme::ThemeManager::Get().GetCurrentTheme());
    RefreshDirectory();
}

void FileDialogForm::NavigateTo(const compat::filesystem::path& path) {
    std::error_code ec;
    if (compat::filesystem::is_directory(path, ec)) {
        m_currentDir = compat::filesystem::canonical(path, ec);
        if (ec) {
            m_currentDir = path;
        }
        if (m_pathBox) {
            m_pathBox->SetText(m_currentDir.string());
        }
        if (m_scrollViewer) {
            m_scrollViewer->SetVerticalOffset(0.0f);
        }
        RefreshDirectory();
    }
}

void FileDialogForm::RefreshDirectory() {
    PopulateDirectoryItems();
}

app::DialogResult FileDialogForm::SimulateConfirm(compat::string_view fileName) {
    if (m_fileNameBox) {
        m_fileNameBox->SetText(fileName);
    }
    OnConfirmAction();
    return GetDialogResult();
}

void FileDialogForm::SimulateCancel() {
    SetDialogResult(app::DialogResult::Cancel);
    Close();
}

void FileDialogForm::BuildUI() {
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    const auto& colors = theme.Colors();
    const auto& metrics = theme.Metrics();

    auto rootBorder = std::make_shared<layout::Border>();
    rootBorder->SetBackground(colors.windowBackground);
    rootBorder->SetBorderBrush(colors.surfaceBorder);
    rootBorder->SetBorderThickness(metrics.borderThicknessDefault);
    rootBorder->SetCornerRadius(metrics.cornerRadiusMedium);
    rootBorder->SetPadding(core::Thickness(12.0f, 12.0f, 12.0f, 12.0f));

    auto mainStack = std::make_shared<layout::StackPanel>(layout::Orientation::Vertical);
    mainStack->SetSpacing(10.0f);

    // --- 頂部導航列 ---
    auto navRow = std::make_shared<layout::StackPanel>(layout::Orientation::Horizontal);
    navRow->SetSpacing(8.0f);

    m_upButton = std::make_shared<controls::Button>("上一層 [..]");
    m_upButton->SetWidth(100.0f);
    m_upButton->SetHeight(30.0f);
    m_upButton->OnClick([this]() {
        if (m_currentDir.has_parent_path()) {
            NavigateTo(m_currentDir.parent_path());
        }
    });
    navRow->AddChild(m_upButton);

    m_pathBox = std::make_shared<controls::TextBox>(m_currentDir.string());
    m_pathBox->SetText(m_currentDir.string());
    m_pathBox->SetWidth(460.0f);
    m_pathBox->SetHeight(30.0f);
    navRow->AddChild(m_pathBox);

    m_refreshButton = std::make_shared<controls::Button>("重新整理");
    m_refreshButton->SetWidth(90.0f);
    m_refreshButton->SetHeight(30.0f);
    m_refreshButton->OnClick([this]() {
        std::string typedPath(m_pathBox->GetText());
        std::error_code ec;
        if (compat::filesystem::exists(typedPath, ec)) {
            NavigateTo(typedPath);
        } else {
            RefreshDirectory();
        }
    });
    navRow->AddChild(m_refreshButton);

    mainStack->AddChild(navRow);

    // --- 中央檔案清單容器 (含 ScrollViewer 滾動支援) ---
    auto listBorder = std::make_shared<layout::Border>();
    listBorder->SetBackground(colors.surfaceBackground);
    listBorder->SetBorderBrush(colors.surfaceBorder);
    listBorder->SetBorderThickness(metrics.borderThicknessDefault);
    listBorder->SetCornerRadius(metrics.cornerRadiusSmall);
    listBorder->SetPadding(core::Thickness(8.0f, 8.0f, 8.0f, 8.0f));
    listBorder->SetWidth(696.0f);
    listBorder->SetHeight(320.0f);

    m_fileListPanel = std::make_shared<layout::StackPanel>(layout::Orientation::Vertical);
    m_fileListPanel->SetSpacing(4.0f);

    m_scrollViewer = std::make_shared<controls::ScrollViewer>();
    m_scrollViewer->SetContent(m_fileListPanel);
    m_scrollViewer->SetWidth(680.0f);
    m_scrollViewer->SetHeight(304.0f);
    listBorder->SetChild(m_scrollViewer);

    mainStack->AddChild(listBorder);

    // --- 底部檔名與篩選條件列 ---
    auto bottomRow1 = std::make_shared<layout::StackPanel>(layout::Orientation::Horizontal);
    bottomRow1->SetSpacing(8.0f);
    bottomRow1->SetVerticalAlignment(layout::VerticalAlignment::Center);

    auto nameLabel = std::make_shared<controls::TextBlock>("檔案名稱:");
    nameLabel->SetForeground(colors.textPrimary);
    nameLabel->SetWidth(80.0f);
    nameLabel->SetVerticalAlignment(layout::VerticalAlignment::Center);
    bottomRow1->AddChild(nameLabel);

    m_fileNameBox = std::make_shared<controls::TextBox>("請輸入或選取檔案名稱");
    m_fileNameBox->SetText(m_selectedFileName);
    m_fileNameBox->SetWidth(380.0f);
    m_fileNameBox->SetHeight(30.0f);
    bottomRow1->AddChild(m_fileNameBox);

    std::string filterLabel = m_filterItems.empty() ? "*.*" : m_filterItems[m_currentFilterIndex].description;
    m_filterButton = std::make_shared<controls::Button>(filterLabel);
    m_filterButton->SetWidth(210.0f);
    m_filterButton->SetHeight(30.0f);
    m_filterButton->OnClick([this]() {
        if (!m_filterItems.empty()) {
            m_currentFilterIndex = (m_currentFilterIndex + 1) % m_filterItems.size();
            m_filterButton->SetContent(m_filterItems[m_currentFilterIndex].description);
            RefreshDirectory();
        }
    });
    bottomRow1->AddChild(m_filterButton);

    mainStack->AddChild(bottomRow1);

    // --- 底部動作按鈕列 (水平靠右) ---
    auto bottomRow2 = std::make_shared<layout::StackPanel>(layout::Orientation::Horizontal);
    bottomRow2->SetSpacing(12.0f);
    bottomRow2->SetHorizontalAlignment(layout::HorizontalAlignment::Right);

    std::string actionText = (m_mode == FileDialogFormMode::Open) ? "開啟" : "儲存";
    m_actionButton = std::make_shared<controls::Button>(actionText);
    m_actionButton->SetWidth(100.0f);
    m_actionButton->SetHeight(34.0f);
    m_actionButton->SetBackground(colors.accentPrimary);
    m_actionButton->SetHoverBackground(colors.accentHover);
    m_actionButton->SetPressedBackground(colors.accentPressed);
    m_actionButton->OnClick([this]() {
        OnConfirmAction();
    });
    bottomRow2->AddChild(m_actionButton);

    m_cancelButton = std::make_shared<controls::Button>("取消");
    m_cancelButton->SetWidth(100.0f);
    m_cancelButton->SetHeight(34.0f);
    m_cancelButton->SetBackground(colors.controlBackground);
    m_cancelButton->SetHoverBackground(colors.controlHoverBackground);
    m_cancelButton->SetPressedBackground(colors.controlPressedBackground);
    m_cancelButton->OnClick([this]() {
        SimulateCancel();
    });
    bottomRow2->AddChild(m_cancelButton);

    mainStack->AddChild(bottomRow2);

    rootBorder->SetChild(mainStack);
    SetContent(rootBorder);
}

void FileDialogForm::PopulateDirectoryItems() {
    if (!m_fileListPanel) return;

    m_fileListPanel->ClearChildren();
    const auto& theme = theme::ThemeManager::Get().GetCurrentTheme();
    const auto& colors = theme.Colors();

    std::error_code ec;
    std::vector<compat::filesystem::directory_entry> dirs;
    std::vector<compat::filesystem::directory_entry> files;

    if (compat::filesystem::exists(m_currentDir, ec) && compat::filesystem::is_directory(m_currentDir, ec)) {
        try {
            compat::filesystem::directory_iterator it(m_currentDir, compat::filesystem::directory_options::skip_permission_denied, ec);
            compat::filesystem::directory_iterator endIt;
            while (!ec && it != endIt) {
                const auto& entry = *it;
                std::error_code statusEc;
                const bool isDir = entry.is_directory(statusEc);
                if (!statusEc && isDir) {
                    dirs.push_back(entry);
                } else {
                    statusEc.clear();
                    const bool isFile = entry.is_regular_file(statusEc);
                    if (!statusEc && isFile) {
                        std::string fname = entry.path().filename().string();
                        if (m_filterItems.empty() || DialogUtilities::MatchesFilter(fname, m_filterItems[m_currentFilterIndex])) {
                            files.push_back(entry);
                        }
                    }
                }
                it.increment(ec);
            }
        } catch (...) {
            // 忽略權限不足或系統連接點等存取例外，保證介面不崩潰
        }
    }

    // 先顯示目錄項目
    for (const auto& d : dirs) {
        std::string dname = d.path().filename().string();
        auto btn = std::make_shared<controls::Button>("[目錄] " + dname);
        btn->SetWidth(660.0f);
        btn->SetHeight(28.0f);
        btn->SetPadding(core::Thickness(8.0f, 4.0f, 8.0f, 4.0f));
        btn->SetHorizontalContentAlignment(layout::HorizontalAlignment::Left);
        btn->SetVerticalContentAlignment(layout::VerticalAlignment::Center);
        btn->SetBackground(colors.surfaceBackground);
        btn->SetHoverBackground(colors.controlHoverBackground);
        btn->OnClick([this, p = d.path()]() {
            NavigateTo(p);
        });
        m_fileListPanel->AddChild(btn);
    }

    // 接著顯示檔案項目
    for (const auto& f : files) {
        std::string fname = f.path().filename().string();
        uint64_t fsize = f.file_size(ec);
        std::string sizeStr = ec ? "" : " (" + DialogUtilities::FormatFileSize(fsize) + ")";

        auto btn = std::make_shared<controls::Button>("[檔案] " + fname + sizeStr);
        btn->SetWidth(660.0f);
        btn->SetHeight(28.0f);
        btn->SetPadding(core::Thickness(8.0f, 4.0f, 8.0f, 4.0f));
        btn->SetHorizontalContentAlignment(layout::HorizontalAlignment::Left);
        btn->SetVerticalContentAlignment(layout::VerticalAlignment::Center);
        btn->SetBackground(colors.surfaceBackground);
        btn->SetHoverBackground(colors.controlHoverBackground);
        btn->OnClick([this, fname, fullPath = f.path()]() {
            if (m_fileNameBox) {
                m_fileNameBox->SetText(fname);
            }
            m_selectedFileName = fullPath.string();
            m_selectedFileNames = { m_selectedFileName };
        });
        m_fileListPanel->AddChild(btn);
    }
}

void FileDialogForm::OnConfirmAction() {
    std::string text(m_fileNameBox ? m_fileNameBox->GetText() : "");
    if (text.empty()) {
        return;
    }

    compat::filesystem::path target(text);
    if (!target.is_absolute()) {
        target = m_currentDir / target;
    }

    std::error_code ec;

    // 若使用者輸入的是資料夾路徑，則切換目錄
    if (compat::filesystem::is_directory(target, ec)) {
        NavigateTo(target);
        return;
    }

    if (m_mode == FileDialogFormMode::Open) {
        if (m_checkFileExists && !compat::filesystem::exists(target, ec)) {
            MessageBox::ShowWithMode(DialogBackendMode::BuiltIn, nullptr,
                                    "指定的檔案不存在，請確認路徑與檔名。",
                                    "開啟失敗",
                                    MessageBoxButtons::OK,
                                    MessageBoxIcon::Warning);
            return;
        }
        m_selectedFileName = target.string();
        m_selectedFileNames = { m_selectedFileName };
        SetDialogResult(app::DialogResult::OK);
        Close();
    } else {
        // 儲存模式
        std::string ext = m_defaultExt;
        if (!ext.empty()) {
            target = DialogUtilities::EnsureExtension(target.string(), ext);
        }

        if (m_overwritePrompt && compat::filesystem::exists(target, ec)) {
            auto res = MessageBox::ShowWithMode(DialogBackendMode::BuiltIn, nullptr,
                                               "目標檔案已存在，是否要覆寫？",
                                               "確認儲存",
                                               MessageBoxButtons::YesNo,
                                               MessageBoxIcon::Question);
            if (res != app::DialogResult::Yes) {
                return;
            }
        }
        m_selectedFileName = target.string();
        m_selectedFileNames = { m_selectedFileName };
        SetDialogResult(app::DialogResult::OK);
        Close();
    }
}

void FileDialogForm::ApplyTheme(const theme::Theme& theme) {
    app::Form::ApplyTheme(theme);
    const auto& colors = theme.Colors();

    if (auto border = std::dynamic_pointer_cast<layout::Border>(GetContent())) {
        border->SetBackground(colors.windowBackground);
        border->SetBorderBrush(colors.surfaceBorder);
    }
}

} // namespace dialogs
} // namespace cppui
