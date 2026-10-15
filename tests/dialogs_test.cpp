/**
 * @file dialogs_test.cpp
 * @brief 對話框模組 (MessageBox, OpenFileDialog, SaveFileDialog) 單元測試
 */

#include "compat/Compat.hpp"
#include "ui/dialogs/DialogCommon.hpp"
#include "ui/dialogs/MessageBox.hpp"
#include "ui/dialogs/MessageBoxForm.hpp"
#include "ui/dialogs/FileDialog.hpp"
#include "ui/dialogs/OpenFileDialog.hpp"
#include "ui/dialogs/SaveFileDialog.hpp"
#include "ui/dialogs/FileDialogForm.hpp"
#include "ui/controls/Button.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/EventRoute.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace {

/// <summary>
/// 測試檔案篩選器字串解析演算法。
/// </summary>
static void TestFilterParser() {
    compat::println("[Test] 測試檔案篩選器字串解析 (ParseFilterString)...");

    using namespace cppui::dialogs;

    // 1. 空字串
    auto emptyRes = DialogUtilities::ParseFilterString("");
    assert(emptyRes.empty());

    // 2. 標準單一項目
    auto singleRes = DialogUtilities::ParseFilterString("Text Files (*.txt)|*.txt");
    assert(singleRes.size() == 1);
    assert(singleRes[0].description == "Text Files (*.txt)");
    assert(singleRes[0].patterns.size() == 1);
    assert(singleRes[0].patterns[0] == "*.txt");

    // 3. 多重篩選項目與多重副檔名
    auto multiRes = DialogUtilities::ParseFilterString("Text Files (*.txt)|*.txt|Image Files (*.png;*.jpg)|*.png;*.jpg|All Files (*.*)|*.*");
    assert(multiRes.size() == 3);
    assert(multiRes[0].description == "Text Files (*.txt)");
    assert(multiRes[0].patterns[0] == "*.txt");

    assert(multiRes[1].description == "Image Files (*.png;*.jpg)");
    assert(multiRes[1].patterns.size() == 2);
    assert(multiRes[1].patterns[0] == "*.png");
    assert(multiRes[1].patterns[1] == "*.jpg");

    assert(multiRes[2].description == "All Files (*.*)");
    assert(multiRes[2].patterns.size() == 1);
    assert(multiRes[2].patterns[0] == "*.*");

    // 4. 前後空白容錯
    auto spaceRes = DialogUtilities::ParseFilterString("  Logs (*.log)  |  *.log ; *.txt  ");
    assert(spaceRes.size() == 1);
    assert(spaceRes[0].description == "Logs (*.log)");
    assert(spaceRes[0].patterns.size() == 2);
    assert(spaceRes[0].patterns[0] == "*.log");
    assert(spaceRes[0].patterns[1] == "*.txt");

    compat::println(" -> 通過: 篩選器解析正確。");
}

/// <summary>
/// 測試檔名萬用字元與篩選條件匹配演算法。
/// </summary>
static void TestPatternAndFilterMatching() {
    compat::println("[Test] 測試萬用字元與樣式比對 (MatchesPattern & MatchesFilter)...");

    using namespace cppui::dialogs;

    // 1. *.* 與 *
    assert(DialogUtilities::MatchesPattern("foo.txt", "*.*") == true);
    assert(DialogUtilities::MatchesPattern("foo", "*") == true);
    assert(DialogUtilities::MatchesPattern("foo.tar.gz", "*.*") == true);

    // 2. 副檔名精確與大小寫不敏感匹配
    assert(DialogUtilities::MatchesPattern("document.txt", "*.txt") == true);
    assert(DialogUtilities::MatchesPattern("DOCUMENT.TXT", "*.txt") == true);
    assert(DialogUtilities::MatchesPattern("document.TXT", "*.txt") == true);
    assert(DialogUtilities::MatchesPattern("document.pdf", "*.txt") == false);

    // 3. 問號萬用字元
    assert(DialogUtilities::MatchesPattern("data1.log", "data?.log") == true);
    assert(DialogUtilities::MatchesPattern("data12.log", "data?.log") == false);

    // 4. 複合樣式篩選項檢驗
    FileFilterItem item;
    item.description = "Images";
    item.patterns = { "*.png", "*.jpg", "*.jpeg" };

    assert(DialogUtilities::MatchesFilter("pic.png", item) == true);
    assert(DialogUtilities::MatchesFilter("photo.JPG", item) == true);
    assert(DialogUtilities::MatchesFilter("banner.jpeg", item) == true);
    assert(DialogUtilities::MatchesFilter("vector.svg", item) == false);

    compat::println(" -> 通過: 樣式比對符合預期。");
}

/// <summary>
/// 測試路徑預設副檔名補齊純函數。
/// </summary>
static void TestEnsureExtension() {
    compat::println("[Test] 測試預設副檔名補齊 (EnsureExtension)...");

    using namespace cppui::dialogs;

    assert(DialogUtilities::EnsureExtension("test", "txt") == "test.txt");
    assert(DialogUtilities::EnsureExtension("test", ".txt") == "test.txt");
    assert(DialogUtilities::EnsureExtension("test.txt", "txt") == "test.txt");
    assert(DialogUtilities::EnsureExtension("test.dat", "txt") == "test.dat");
    assert(DialogUtilities::EnsureExtension("dir.sub/file", "txt") == "dir.sub/file.txt");
    assert(DialogUtilities::EnsureExtension("dir.sub/file.png", "txt") == "dir.sub/file.png");

    compat::println(" -> 通過: 副檔名補齊正確。");
}

/// <summary>
/// 測試檔案大小易讀字串格式化。
/// </summary>
static void TestFormatFileSize() {
    compat::println("[Test] 測試檔案大小格式化 (FormatFileSize)...");

    using namespace cppui::dialogs;

    assert(DialogUtilities::FormatFileSize(512ULL) == "512 B");
    assert(DialogUtilities::FormatFileSize(1024ULL) == "1.0 KB");
    assert(DialogUtilities::FormatFileSize(1024ULL * 1024ULL * 2ULL) == "2.00 MB");
    assert(DialogUtilities::FormatFileSize(1024ULL * 1024ULL * 1024ULL * 3ULL) == "3.00 GB");

    compat::println(" -> 通過: 大小格式化符合預期。");
}

/// <summary>
/// 測試 FileDialog, OpenFileDialog 與 SaveFileDialog 基本屬性與重設契約。
/// </summary>
static void TestFileDialogPropertiesAndReset() {
    compat::println("[Test] 測試 FileDialog 家族屬性與 Reset 契約...");

    using namespace cppui::dialogs;

    // OpenFileDialog
    OpenFileDialog openDlg;
    assert(openDlg.GetCheckFileExists() == true);
    assert(openDlg.GetMultiselect() == false);
    assert(openDlg.GetFilterIndex() == 1);
    assert(openDlg.GetBackendMode() == DialogBackendMode::Auto);

    openDlg.SetTitle("自訂開檔標題");
    openDlg.SetInitialDirectory("/custom/dir");
    openDlg.SetFileName("my_file.txt");
    openDlg.SetFilter("Text Files (*.txt)|*.txt");
    openDlg.SetFilterIndex(2);
    openDlg.SetMultiselect(true);
    openDlg.SetBackendMode(DialogBackendMode::BuiltIn);

    assert(openDlg.GetTitle() == "自訂開檔標題");
    assert(openDlg.GetInitialDirectory() == "/custom/dir");
    assert(openDlg.GetFileName() == "my_file.txt");
    assert(openDlg.GetFilter() == "Text Files (*.txt)|*.txt");
    assert(openDlg.GetFilterIndex() == 2);
    assert(openDlg.GetMultiselect() == true);
    assert(openDlg.GetBackendMode() == DialogBackendMode::BuiltIn);

    openDlg.Reset();
    assert(openDlg.GetTitle().empty());
    assert(openDlg.GetInitialDirectory().empty());
    assert(openDlg.GetFileName().empty());
    assert(openDlg.GetFilter().empty());
    assert(openDlg.GetFilterIndex() == 1);
    assert(openDlg.GetMultiselect() == false);
    assert(openDlg.GetCheckFileExists() == true);
    assert(openDlg.GetBackendMode() == DialogBackendMode::Auto);

    // SaveFileDialog
    SaveFileDialog saveDlg;
    assert(saveDlg.GetOverwritePrompt() == true);
    assert(saveDlg.GetCreatePrompt() == false);
    assert(saveDlg.GetFilterIndex() == 1);

    saveDlg.SetDefaultExt("json");
    saveDlg.SetOverwritePrompt(false);
    saveDlg.SetCreatePrompt(true);

    assert(saveDlg.GetDefaultExt() == "json");
    assert(saveDlg.GetOverwritePrompt() == false);
    assert(saveDlg.GetCreatePrompt() == true);

    saveDlg.Reset();
    assert(saveDlg.GetDefaultExt().empty());
    assert(saveDlg.GetOverwritePrompt() == true);
    assert(saveDlg.GetCreatePrompt() == false);

    compat::println(" -> 通過: 屬性與 Reset 重設行為完全正確。");
}

/// <summary>
/// 測試對話框後端模式切換機制。
/// </summary>
static void TestDialogBackendModeSwitching() {
    compat::println("[Test] 測試後端切換 (DialogManager & MessageBox 後端)...");

    using namespace cppui::dialogs;

    // 全域模式設定
    assert(DialogManager::GetDefaultBackendMode() == DialogBackendMode::Auto);
    DialogManager::SetDefaultBackendMode(DialogBackendMode::BuiltIn);
    assert(DialogManager::GetDefaultBackendMode() == DialogBackendMode::BuiltIn);
    DialogManager::SetDefaultBackendMode(DialogBackendMode::Native);
    assert(DialogManager::GetDefaultBackendMode() == DialogBackendMode::Native);
    DialogManager::SetDefaultBackendMode(DialogBackendMode::Auto);

    // MessageBox 後端設定
    assert(MessageBox::GetBackendMode() == DialogBackendMode::Auto);
    MessageBox::SetBackendMode(DialogBackendMode::BuiltIn);
    assert(MessageBox::GetBackendMode() == DialogBackendMode::BuiltIn);
    MessageBox::SetBackendMode(DialogBackendMode::Auto);

    compat::println(" -> 通過: 後端模式切換正確無誤。");
}

/// <summary>
/// 測試純 CPPUI 主題 MessageBoxForm 視窗建構與模擬按鈕點擊結果。
/// </summary>
static void TestMessageBoxFormBuiltIn() {
    compat::println("[Test] 測試純 CPPUI MessageBoxForm 建立與按鈕操作...");

    using namespace cppui::dialogs;
    using namespace cppui::app;

    // 1. OK 模式
    MessageBoxForm okBox("操作已成功完成！", "通知", MessageBoxButtons::OK, MessageBoxIcon::Information);
    assert(okBox.GetMessageText() == "操作已成功完成！");
    assert(okBox.GetButtons() == MessageBoxButtons::OK);
    assert(okBox.GetIcon() == MessageBoxIcon::Information);
    assert(okBox.GetDialogResult() == DialogResult::None);

    okBox.SimulateButtonClick(DialogResult::OK);
    assert(okBox.GetDialogResult() == DialogResult::OK);

    // 2. YesNo 模式
    MessageBoxForm yesNoBox("是否確定要刪除此檔案？", "確認", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
    assert(yesNoBox.GetButtons() == MessageBoxButtons::YesNo);
    yesNoBox.SimulateButtonClick(DialogResult::No);
    assert(yesNoBox.GetDialogResult() == DialogResult::No);

    // 3. RetryCancel 模式
    MessageBoxForm retryBox("連線失敗，是否重試？", "錯誤", MessageBoxButtons::RetryCancel, MessageBoxIcon::Error);
    retryBox.SimulateButtonClick(DialogResult::Retry);
    assert(retryBox.GetDialogResult() == DialogResult::Retry);

    compat::println(" -> 通過: MessageBoxForm 程式化行為正確。");
}

/// <summary>
/// 測試純 CPPUI 主題 FileDialogForm 檔案瀏覽與操作結果。
/// </summary>
static void TestFileDialogFormBuiltIn() {
    compat::println("[Test] 測試純 CPPUI FileDialogForm 目錄走訪與檔案選取...");

    using namespace cppui::dialogs;
    using namespace cppui::app;

    // 建立臨時測試環境
    compat::filesystem::path tempDir = compat::filesystem::temp_directory_path() / "cppui_dialog_test";
    std::error_code ec;
    compat::filesystem::create_directories(tempDir, ec);

    compat::filesystem::path file1 = tempDir / "doc1.txt";
    compat::filesystem::path file2 = tempDir / "photo.png";

    {
        std::ofstream ofs1(file1.string());
        ofs1 << "Hello CPPUI File Dialog!";
        std::ofstream ofs2(file2.string());
        ofs2 << "Binary PNG Data";
    }

    // 1. 開啟檔案模式 (Open)
    FileDialogForm openForm(
        FileDialogFormMode::Open,
        "測試開啟",
        tempDir.string(),
        "",
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
        1,
        "txt",
        false,
        false,
        true, // checkFileExists
        true
    );

    assert(openForm.GetCurrentDirectory() == tempDir);

    // 模擬確認現有檔案
    auto openRes = openForm.SimulateConfirm("doc1.txt");
    assert(openRes == DialogResult::OK);
    (void)openRes;
    assert(openForm.GetSelectedFileName() == file1.string());
    assert(openForm.GetSelectedFileNames().size() == 1);
    assert(openForm.GetSelectedFileNames()[0] == file1.string());

    // 2. 儲存檔案模式 (Save) - 自動補齊副檔名
    FileDialogForm saveForm(
        FileDialogFormMode::Save,
        "測試儲存",
        tempDir.string(),
        "",
        "Text Files (*.txt)|*.txt",
        1,
        "txt",
        false,
        false, // 不提示覆寫以供直接驗證
        false,
        true
    );

    auto saveRes = saveForm.SimulateConfirm("new_document");
    assert(saveRes == DialogResult::OK);
    (void)saveRes;
    compat::filesystem::path expectedSavePath = tempDir / "new_document.txt";
    assert(saveForm.GetSelectedFileName() == expectedSavePath.string());

    // 3. 取消動作
    FileDialogForm cancelForm(
        FileDialogFormMode::Open,
        "測試取消",
        tempDir.string(),
        "",
        "*.*",
        1,
        ""
    );
    cancelForm.SimulateCancel();
    assert(cancelForm.GetDialogResult() == DialogResult::Cancel);

    // 清理臨時目錄
    compat::filesystem::remove_all(tempDir, ec);

    compat::println(" -> 通過: FileDialogForm 程式化走訪與選取完全正確。");
}

/// <summary>
/// 測試資料夾瀏覽切換與按鈕點擊解構安全性 (防範 UAF)。
/// </summary>
static void TestFolderNavigationAndClickSafety() {
    compat::println("[Test] 測試資料夾切換導航與 Click 回呼解構安全性 (防範 UAF)...");

    using namespace cppui::dialogs;
    using namespace cppui::controls;
    using namespace cppui::layout;
    using namespace cppui::input;
    using namespace cppui::core;

    compat::filesystem::path tempDir = compat::filesystem::temp_directory_path() / "cppui_folder_nav_test";
    compat::filesystem::path subDir = tempDir / "child_folder";
    std::error_code ec;
    compat::filesystem::create_directories(subDir, ec);

    std::ofstream((subDir / "nested.txt").string()) << "Nested Content";

    FileDialogForm form(
        FileDialogFormMode::Open,
        "導航測試",
        tempDir.string(),
        "",
        "*.*"
    );

    assert(form.GetCurrentDirectory() == tempDir);

    // 1. 切換至子目錄
    form.NavigateTo(subDir);
    assert(form.GetCurrentDirectory() == subDir);

    // 2. 切換回父目錄
    form.NavigateTo(tempDir);
    assert(form.GetCurrentDirectory() == tempDir);

    // 3. 測試按鈕於 OnClick 中清除自身所屬容器子元件 (模擬原先引發 UAF 的時序)
    auto panel = std::make_shared<StackPanel>(Orientation::Vertical);
    auto btn = std::make_shared<Button>("SelfDestruct");
    btn->SetWidth(100.0f);
    btn->SetHeight(30.0f);
    panel->AddChild(btn);

    panel->Measure(Size(200.0f, 200.0f));
    panel->Arrange(Rect(0.0f, 0.0f, 200.0f, 200.0f));

    bool clickInvoked = false;
    btn->OnClick([&panel, &clickInvoked]() {
        clickInvoked = true;
        panel->ClearChildren(); // 清空按鈕！如果 Button 在 RaiseClick 後存取 this，將會引發 UAF 崩潰
    });

    // 模擬滑鼠按下
    PointerEventArgs pressArgs(
        StandardEvents::PointerPressed,
        btn.get(),
        Point(10.0f, 10.0f),
        cppui::pal::RawMouseButton::Left,
        cppui::pal::KeyModifiers::None
    );
    EventRoute pressRoute(btn.get());
    pressRoute.Dispatch(StandardEvents::PreviewPointerPressed, StandardEvents::PointerPressed, pressArgs);

    // 模擬滑鼠放開
    PointerEventArgs releaseArgs(
        StandardEvents::PointerReleased,
        btn.get(),
        Point(10.0f, 10.0f),
        cppui::pal::RawMouseButton::Left,
        cppui::pal::KeyModifiers::None
    );
    EventRoute releaseRoute(btn.get());
    releaseRoute.Dispatch(StandardEvents::PreviewPointerReleased, StandardEvents::PointerReleased, releaseArgs);

    assert(clickInvoked == true);
    assert(panel->GetChildrenCount() == 0);

    compat::filesystem::remove_all(tempDir, ec);

    compat::println(" -> 通過: 資料夾切換與 Button 於 Click 中安全解構無 UAF。");
}

} // namespace

int main() {
    compat::println("=================================================");
    compat::println("      CPPUI 對話框系統模組單元測試套件          ");
    compat::println("=================================================");

    TestFilterParser();
    TestPatternAndFilterMatching();
    TestEnsureExtension();
    TestFormatFileSize();
    TestFileDialogPropertiesAndReset();
    TestDialogBackendModeSwitching();
    TestMessageBoxFormBuiltIn();
    TestFileDialogFormBuiltIn();
    TestFolderNavigationAndClickSafety();

    compat::println("=================================================");
    compat::println("所有對話框系統單元測試均成功通過 (100% Passed)！");
    compat::println("=================================================");
    return 0;
}
