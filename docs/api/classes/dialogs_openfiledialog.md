# OpenFileDialog 開啟檔案對話方塊

- **命名空間**: `cppui::dialogs`
- **標頭檔**: `include/ui/dialogs/OpenFileDialog.hpp`
- **實作檔**: `src/dialogs/OpenFileDialog.cpp`
- **繼承體系**: `CommonDialog` -> `FileDialog` -> `OpenFileDialog`

---

## 1. 類別概述

`OpenFileDialog` 封裝開啟檔案時的使用者選取互動，支援單一檔案選取、多重檔案選取 (`Multiselect`)、唯讀選項與檔案存在性檢查。

具備**雙後端透明派送**：
- Windows 下優先使用 COM [`IFileOpenDialog`](file:///D:/program/C++/CPP-UI/src/dialogs/Win32DialogService.cpp)，支援系統捷徑與完整 Shell 整合。
- Linux / WSL / 測試環境自動退回純 CPPUI 主題視窗 [`FileDialogForm`](dialogs_filedialogform.md)。

---

## 2. API 宣告與專屬介面

```cpp
namespace cppui {
namespace dialogs {

class OpenFileDialog : public FileDialog {
public:
    OpenFileDialog();
    ~OpenFileDialog() override = default;

    // --- 多選支援 ---
    COMPAT_NODISCARD bool GetMultiselect() const noexcept;
    void SetMultiselect(bool multiselect) noexcept;

    COMPAT_NODISCARD const std::vector<std::string>& GetFileNames() const noexcept;

    // --- 唯讀核取方塊 ---
    COMPAT_NODISCARD bool GetShowReadOnly() const noexcept;
    void SetShowReadOnly(bool show) noexcept;

    COMPAT_NODISCARD bool GetReadOnlyChecked() const noexcept;
    void SetReadOnlyChecked(bool check) noexcept;

    // --- 模態顯示與重設 ---
    app::DialogResult ShowDialog(std::shared_ptr<app::Form> owner = nullptr) override;
    void Reset() override;
};

} // namespace dialogs
} // namespace cppui
```

---

## 3. 專屬屬性說明

| 屬性名稱 | 型別 | 預設值 | 說明 |
| :--- | :--- | :--- | :--- |
| `Multiselect` | `bool` | `false` | 是否允許多重選取檔案。啟用後，使用者可選取多個項目。 |
| `FileNames` | `const std::vector<std::string>&` | 空陣列 | 取得所有選取的檔案完整路徑清單（若為多選時包含所有選中檔案；單選時包含單一項目）。 |
| `ShowReadOnly` | `bool` | `false` | 是否在對話框中顯示「以唯讀方式開啟」核取方塊。 |
| `ReadOnlyChecked`| `bool` | `false` | 取得或設定「以唯讀方式開啟」是否處於勾選狀態。 |
| `CheckFileExists`| `bool` | `true` | 在 `OpenFileDialog` 中預設自動啟用檔案存在性驗證。 |

---

## 4. 程式碼使用範例

### 4.1 基本單選檔案
```cpp
#include "ui/dialogs/OpenFileDialog.hpp"

using namespace cppui::dialogs;
using namespace cppui::app;

OpenFileDialog dialog;
dialog.SetTitle("選取文字文件");
dialog.SetInitialDirectory("C:/Projects");
dialog.SetFilter("文字檔案 (*.txt)|*.txt|所有檔案 (*.*)|*.*");

if (dialog.ShowDialog() == DialogResult::OK) {
    std::string selectedPath(dialog.GetFileName());
    // 讀取檔案內容...
}
```

### 4.2 多重檔案選取
```cpp
OpenFileDialog dialog;
dialog.SetTitle("批次匯入影像");
dialog.SetFilter("圖片檔案 (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg|所有檔案 (*.*)|*.*");
dialog.SetMultiselect(true);

if (dialog.ShowDialog(ownerForm) == DialogResult::OK) {
    for (const auto& path : dialog.GetFileNames()) {
        LoadImageFile(path);
    }
}
```
