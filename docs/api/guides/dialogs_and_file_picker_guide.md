# 對話方塊與檔案選取器實戰指南 (Dialogs & File Pickers Guide)

本指南介紹如何在 CPPUI 應用程式中使用 **訊息方塊 (`MessageBox`)**、**開啟檔案對話方塊 (`OpenFileDialog`)** 與 **儲存檔案對話方塊 (`SaveFileDialog`)**。

---

## 1. 雙後端派送架構與設計概念

CPPUI 採用獨特的**雙後端混合架構 (Dual-Backend Hybrid Architecture)**：

```mermaid
graph TD
    App["應用程式程式碼"] --> Facade["對話框 API (MessageBox / FileDialog)"]
    Facade --> Route{"後端模式判斷 (Auto / Native / BuiltIn)"}
    Route -->|Auto (Windows) 或 Native| Win32["Win32 原生 COM 服務<br>(IFileOpenDialog, IFileSaveDialog, MessageBoxW)"]
    Route -->|Auto (Linux/WSL) 或 BuiltIn| BuiltIn["純 CPPUI Form 佈景主題視窗<br>(MessageBoxForm, FileDialogForm)"]
    BuiltIn --> Theme["ThemeManager 主題引擎<br>(自動同步深色/淺色模式色彩)"]
```

### 後端模式選項 (`DialogBackendMode`)
- **`Auto` (預設)**：
  - 在 Windows 桌面環境下優先使用作業系統原生對話框（COM 檔案總管與系統 MessageBox），具備完整的系統外觀、快顯捷徑、網路位置搜尋與提示音效。
  - 在 Linux/WSL、無原生桌面、或單元測試環境時，自動退回至純 CPPUI 主題 Form。
- **`Native`**：強制使用作業系統原生對話框。
- **`BuiltIn`**：強制使用 CPPUI 自繪之 Form 對話框，100% 依循應用程式自訂主題色彩與字型。

---

## 2. 訊息方塊 (MessageBox) 實戰

### 2.1 一般訊息通知
最簡便的呼叫方式，僅需傳入訊息文字與視窗標題：

```cpp
#include "ui/dialogs/MessageBox.hpp"

using namespace cppui::dialogs;

MessageBox::Show("資料庫快取已成功重建！", "通知");
```

### 2.2 多按鈕分支確認
透過傳入 `MessageBoxButtons` 與 `MessageBoxIcon`，接收使用者的決策並採取相應分支動作：

```cpp
auto result = MessageBox::Show(
    thisForm, // 主控擁有者視窗 shared_ptr (可為 nullptr)
    "您有未儲存的變更，是否要在關閉前儲存？",
    "未儲存的檔案",
    MessageBoxButtons::YesNoCancel,
    MessageBoxIcon::Question,
    MessageBoxDefaultButton::Button1 // 預設聚焦於「是」
);

switch (result) {
case cppui::app::DialogResult::Yes:
    SaveDocument();
    CloseWindow();
    break;
case cppui::app::DialogResult::No:
    CloseWindow(); // 捨棄變更
    break;
case cppui::app::DialogResult::Cancel:
    // 使用者取消，不做任何動作
    break;
default:
    break;
}
```

---

## 3. 開啟檔案 (OpenFileDialog) 實戰

### 3.1 單一檔案選取與篩選器設定
篩選器使用標準管道符號 `|` 與分號 `;` 語法：

```cpp
#include "ui/dialogs/OpenFileDialog.hpp"

using namespace cppui::dialogs;
using namespace cppui::app;

OpenFileDialog dialog;
dialog.SetTitle("請選擇要開啟的文字記錄");
dialog.SetInitialDirectory("C:/Logs");
dialog.SetFilter("文字日誌 (*.log;*.txt)|*.log;*.txt|所有檔案 (*.*)|*.*");
dialog.SetFilterIndex(1);

if (dialog.ShowDialog(thisForm) == DialogResult::OK) {
    compat::println("已選取檔案: {}", dialog.GetFileName());
    ReadFileContent(dialog.GetFileName());
}
```

### 3.2 批次多重檔案選取 (`Multiselect`)
將 `SetMultiselect(true)` 設為 true 後，可透過 `GetFileNames()` 取得選取之清單：

```cpp
OpenFileDialog dialog;
dialog.SetTitle("選取多張圖片進行批次處理");
dialog.SetFilter("圖片檔案 (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg");
dialog.SetMultiselect(true);

if (dialog.ShowDialog() == DialogResult::OK) {
    for (const auto& filePath : dialog.GetFileNames()) {
        ProcessImage(filePath);
    }
}
```

---

## 4. 儲存檔案 (SaveFileDialog) 實戰

### 4.1 儲存檔案與副檔名自動補齊
當使用者未在檔名輸入框鍵入副檔名時，`SaveFileDialog` 會自動依據 `DefaultExt` 補齊：

```cpp
#include "ui/dialogs/SaveFileDialog.hpp"

using namespace cppui::dialogs;
using namespace cppui::app;

SaveFileDialog dialog;
dialog.SetTitle("另存新專案");
dialog.SetDefaultExt("json"); // 自動補齊 .json
dialog.SetFilter("專案設定檔 (*.json)|*.json|所有檔案 (*.*)|*.*");
dialog.SetOverwritePrompt(true); // 若檔案已存在自動提示確認覆寫

if (dialog.ShowDialog(thisForm) == DialogResult::OK) {
    SaveProjectConfig(dialog.GetFileName());
}
```

---

## 5. 執行期後端模式切換

應用程式可在執行期全域切換，或在單次呼叫時指定後端模式：

```cpp
// 1. 全域設定所有對話框使用純 CPPUI Form 佈景主題外觀
DialogManager::SetDefaultBackendMode(DialogBackendMode::BuiltIn);

// 2. 針對個別對話框單獨設定
OpenFileDialog ofd;
ofd.SetBackendMode(DialogBackendMode::Native); // 強制該對話框走 Win32 原生

// 3. MessageBox 帶有模式的顯式呼叫
MessageBox::ShowWithMode(
    DialogBackendMode::BuiltIn,
    thisForm,
    "這是一則純 CPPUI 主題繪製的訊息！",
    "主題展示",
    MessageBoxButtons::OK,
    MessageBoxIcon::Information
);
```

---

## 6. 單元測試與 CI 自動化策略

原生 OS 對話框（如 Win32 `MessageBoxW`）在無人點擊時會阻塞主執行緒，導致 CI 掛起超時。CPPUI 提供雙重機制徹底解決此痛點：

1. **在 CI / 單元測試中設置 `BuiltIn` 模式**：
   ```cpp
   DialogManager::SetDefaultBackendMode(DialogBackendMode::BuiltIn);
   ```
2. **使用表單程式化模擬介面 (`SimulateConfirm` / `SimulateButtonClick`)**：
   ```cpp
   // 測試 MessageBoxForm
   MessageBoxForm box("測試訊息", "標題", MessageBoxButtons::YesNo);
   box.SimulateButtonClick(DialogResult::Yes);
   assert(box.GetDialogResult() == DialogResult::Yes);

   // 測試 FileDialogForm
   FileDialogForm fileForm(FileDialogFormMode::Open, "開檔", "/tmp", "", "*.txt", 1, "txt");
   auto result = fileForm.SimulateConfirm("test.txt");
   assert(result == DialogResult::OK);
   ```

---

## 7. 範例展示程式 (`dialog_gallery`)

請參見 [`examples/dialog_gallery/main.cpp`](file:///D:/program/C++/CPP-UI/examples/dialog_gallery/main.cpp)，可直接建置並啟動此完整展示專案，體驗包含訊息方塊、開檔、多選、存檔、即時主題切換與後端模式動態切換之所有特性。
