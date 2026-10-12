# SaveFileDialog 儲存檔案對話方塊

- **命名空間**: `cppui::dialogs`
- **標頭檔**: `include/ui/dialogs/SaveFileDialog.hpp`
- **實作檔**: `src/dialogs/SaveFileDialog.cpp`
- **繼承體系**: `CommonDialog` -> `FileDialog` -> `SaveFileDialog`

---

## 1. 類別概述

`SaveFileDialog` 封裝儲存檔案時的使用者路徑選擇互動，支援預設副檔名自動補齊 (`DefaultExt`)、檔案已存在時之覆寫確認提示 (`OverwritePrompt`) 與新建提示。

底層透明銜接：
- Windows 下優先使用 COM [`IFileSaveDialog`](../../../../src/dialogs/Win32DialogService.cpp)，原生支援 `FOS_OVERWRITEPROMPT`。
- Linux / WSL / 測試環境自動退回純 CPPUI 主題視窗 [`FileDialogForm`](dialogs_filedialogform.md)，在使用者嘗試覆寫時透過主題 [`MessageBoxForm`](dialogs_messagebox.md#純-cppui-主題視窗-messageboxform) 詢問確認。

---

## 2. API 宣告與專屬介面

```cpp
namespace cppui {
namespace dialogs {

class SaveFileDialog : public FileDialog {
public:
    SaveFileDialog();
    ~SaveFileDialog() override = default;

    // --- 提示行為控制 ---
    COMPAT_NODISCARD bool GetOverwritePrompt() const noexcept;
    void SetOverwritePrompt(bool prompt) noexcept;

    COMPAT_NODISCARD bool GetCreatePrompt() const noexcept;
    void SetCreatePrompt(bool prompt) noexcept;

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
| `OverwritePrompt` | `bool` | `true` | 當使用者選取已存在的檔案時，是否自動彈出確認視窗詢問是否覆寫。 |
| `CreatePrompt` | `bool` | `false` | 當使用者輸入不存在的檔案時，是否提示即將建立新檔案。 |
| `DefaultExt` | `compat::string_view` | `""` | 預設副檔名（繼承自 `FileDialog`）。若使用者輸入之路徑無副檔名，將自動追加此副檔名。 |

---

## 4. 程式碼使用範例

### 4.1 儲存檔案與副檔名自動補齊
```cpp
#include "ui/dialogs/SaveFileDialog.hpp"

using namespace cppui::dialogs;
using namespace cppui::app;

SaveFileDialog dialog;
dialog.SetTitle("另存新檔");
dialog.SetDefaultExt("json");
dialog.SetFilter("JSON 設定檔 (*.json)|*.json|所有檔案 (*.*)|*.*");
dialog.SetOverwritePrompt(true);

if (dialog.ShowDialog() == DialogResult::OK) {
    std::string savePath(dialog.GetFileName());
    // 寫入檔案...
}
```

### 4.2 指定 BuiltIn 佈景主題儲存
```cpp
SaveFileDialog dialog;
dialog.SetTitle("匯出報表");
dialog.SetDefaultExt("csv");
dialog.SetBackendMode(DialogBackendMode::BuiltIn); // 強制純 CPPUI Form 主題外觀

if (dialog.ShowDialog(ownerForm) == DialogResult::OK) {
    ExportReportToCsv(dialog.GetFileName());
}
```
