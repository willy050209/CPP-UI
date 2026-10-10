# FileDialogForm 純 CPPUI 檔案選擇視窗

- **命名空間**: `cppui::dialogs`
- **標頭檔**: `include/ui/dialogs/FileDialogForm.hpp`
- **實作檔**: `src/dialogs/FileDialogForm.cpp`
- **繼承體系**: `Visual` -> `UIElement` -> `Control` -> `ContentControl` -> `Form` -> `FileDialogForm`

---

## 1. 類別概述

`FileDialogForm` 是完全由純 C++ 與 CPPUI 核心控制項組裝而成的跨平台檔案總管視窗。專為在 Linux/WSL 環境、自訂佈景主題應用與 Headless CI 自動化測試所打造。

### 主要功能
- **導航工具列**：包含返回上一層資料夾按鈕 `[..]`、路徑手動輸入/跳轉文字框與重新整理按鈕。
- **檔案瀏覽清單**：採用 `std::filesystem` 走訪目錄，優先列出資料夾項目，隨後列出符合篩選器之檔案，並顯示由 `DialogUtilities::FormatFileSize` 格式化的檔案大小。
- **篩選條件切換**：點擊篩選條件按鈕可輪流切換設定的過濾模式（例如 `*.txt`、`*.png`、`*.*`），並即時刷新檔案清單。
- **覆寫保護與確認**：若為儲存模式且目標檔案已存在，自動使用 `MessageBox::Show` 觸發覆寫確認。
- **深淺主題適配**：直接繼承自 `Form`，自動向 `ThemeManager` 監聽並套用主題色彩變更。
- **無阻塞程式化模擬**：提供 `SimulateConfirm` 與 `SimulateCancel` 介面，供單元測試與 CI 在無需實體顯卡/視窗的情況下全自動驗證。

---

## 2. API 宣告

```cpp
namespace cppui {
namespace dialogs {

enum class FileDialogFormMode : uint8_t {
    Open,
    Save
};

class FileDialogForm : public app::Form {
public:
    FileDialogForm(FileDialogFormMode mode,
                   compat::string_view title,
                   compat::string_view initialDir,
                   compat::string_view fileName,
                   compat::string_view filter,
                   uint32_t filterIndex,
                   compat::string_view defaultExt,
                   bool multiselect = false,
                   bool overwritePrompt = true,
                   bool checkFileExists = false,
                   bool checkPathExists = true);

    COMPAT_NODISCARD compat::string_view GetSelectedFileName() const noexcept;
    COMPAT_NODISCARD const std::vector<std::string>& GetSelectedFileNames() const noexcept;
    COMPAT_NODISCARD const std::filesystem::path& GetCurrentDirectory() const noexcept;
    COMPAT_NODISCARD bool GetMultiselect() const noexcept;
    COMPAT_NODISCARD bool GetCheckPathExists() const noexcept;

    void NavigateTo(const std::filesystem::path& path);
    void RefreshDirectory();

    // 程式化模擬介面 (供單元測試與自動化腳本)
    app::DialogResult SimulateConfirm(compat::string_view fileName);
    void SimulateCancel();

protected:
    void OnLoad() override;
    void ApplyTheme(const theme::Theme& theme) override;
};

} // namespace dialogs
} // namespace cppui
```

---

## 3. 程式化測試範例

```cpp
#include "ui/dialogs/FileDialogForm.hpp"

using namespace cppui::dialogs;
using namespace cppui::app;

// 在單元測試中直接建構並模擬使用者選定檔案行為
FileDialogForm form(
    FileDialogFormMode::Open,
    "測試開檔",
    "/tmp/test_dir",
    "",
    "Text Files (*.txt)|*.txt",
    1,
    "txt"
);

auto result = form.SimulateConfirm("data.txt");
assert(result == DialogResult::OK);
assert(form.GetSelectedFileName() == "/tmp/test_dir/data.txt");
```
