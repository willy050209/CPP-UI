# cppui::dialogs 命名空間

- **標頭檔目錄**: `include/ui/dialogs/`

---

## 概述

`cppui::dialogs` 命名空間提供現代桌面應用程式不可或缺的互動對話方塊系統，涵蓋**模態訊息方塊 (`MessageBox`)**、**開啟檔案對話方塊 (`OpenFileDialog`)** 與**儲存檔案對話方塊 (`SaveFileDialog`)**。

架構採用先進的**雙後端混合派送 (Dual-Backend Hybrid Architecture)** 機制：
1. **原生作業系統後端 (Native OS Dialogs)**：在 Windows 平台上優先呼叫系統 COM 介面（`IFileOpenDialog`、`IFileSaveDialog`）與 Win32 `MessageBoxW`，呈現完全原生的檔案總管與系統通知體驗。
2. **純 CPPUI 佈景主題後端 (BuiltIn Form Dialogs)**：在 Linux/WSL、無原生桌面、深淺色自定義樣式或 CI 自動化測試環境中，無縫退回由 CPPUI 控制項（`Form`, `Border`, `Button`, `TextBox`, `TextBlock`, `StackPanel`）組裝而成的自繪對話方塊，自動依據 `ThemeManager` 套用深色/淺色主題。

---

## 包含類別、結構與列舉

| 類別 / 結構 / 列舉 | 標頭檔 | 專屬技術文件 | 說明 |
| :--- | :--- | :--- | :--- |
| [`MessageBox`](../classes/dialogs_messagebox.md) | `MessageBox.hpp` | [**MessageBox 類別**](../classes/dialogs_messagebox.md) | 靜態模態訊息方塊外觀 API，提供 6 種重載、預設按鈕配置與雙後端模式切換。 |
| [`MessageBoxForm`](../classes/dialogs_messagebox.md#純-cppui-主題視窗-messageboxform) | `MessageBoxForm.hpp` | [**MessageBoxForm 類別**](../classes/dialogs_messagebox.md#純-cppui-主題視窗-messageboxform) | 純 CPPUI Form 佈景主題訊息視窗，具備向量狀態圖示、多行排版與深淺色主題自適應。 |
| [`CommonDialog`](../classes/dialogs_filedialog.md#1-commondialog-抽象基底類別) | `FileDialog.hpp` | [**CommonDialog 抽象基底**](../classes/dialogs_filedialog.md#1-commondialog-抽象基底類別) | 通用對話框抽象基底類別，規範 `ShowDialog(owner)` 與 `Reset()` 契約。 |
| [`FileDialog`](../classes/dialogs_filedialog.md#2-filedialog-通用檔案對話框基底) | `FileDialog.hpp` | [**FileDialog 檔案對話框基底**](../classes/dialogs_filedialog.md#2-filedialog-通用檔案對話框基底) | 檔案對話框通用基底，管理標題、起始目錄、檔名、篩選器、預設副檔名與檢查旗標。 |
| [`OpenFileDialog`](../classes/dialogs_openfiledialog.md) | `OpenFileDialog.hpp` | [**OpenFileDialog 開檔對話框**](../classes/dialogs_openfiledialog.md) | 開啟檔案對話方塊，支援單選/多選 (`Multiselect`)、唯讀選項與檔案存在性檢查。 |
| [`SaveFileDialog`](../classes/dialogs_savefiledialog.md) | `SaveFileDialog.hpp` | [**SaveFileDialog 存檔對話框**](../classes/dialogs_savefiledialog.md) | 儲存檔案對話方塊，支援覆寫確認 (`OverwritePrompt`) 與副檔名自動補齊。 |
| [`FileDialogForm`](../classes/dialogs_filedialogform.md) | `FileDialogForm.hpp` | [**FileDialogForm 瀏覽視窗**](../classes/dialogs_filedialogform.md) | 純 CPPUI Form 檔案總管視窗，支援目錄導航、即時篩選、檔案大小顯示與程式化測試。 |
| [`DialogUtilities`](../classes/dialogs_dialogcommon.md#1-dialogutilities-靜態純函數工具) | `DialogCommon.hpp` | [**DialogUtilities 工具類別**](../classes/dialogs_dialogcommon.md#1-dialogutilities-靜態純函數工具) | 純靜態演算法工具：篩選器字串解析、萬用字元比對、副檔名補齊與檔案大小易讀格式化。 |
| [`DialogManager`](../classes/dialogs_dialogcommon.md#2-dialogmanager-全域服務管理) | `DialogCommon.hpp` | [**DialogManager 管理器**](../classes/dialogs_dialogcommon.md#2-dialogmanager-全域服務管理) | 全域對話框後端派送管理員，提供 `SetDefaultBackendMode` 執行期模式切換。 |
| [`MessageBoxButtons`](../classes/dialogs_dialogcommon.md#3-列舉型別清單) | `DialogCommon.hpp` | [**MessageBoxButtons 列舉**](../classes/dialogs_dialogcommon.md#3-列舉型別清單) | 按鈕選項 (`OK`, `OKCancel`, `YesNo`, `YesNoCancel`, `RetryCancel`, `AbortRetryIgnore`)。 |
| [`MessageBoxIcon`](../classes/dialogs_dialogcommon.md#3-列舉型別清單) | `DialogCommon.hpp` | [**MessageBoxIcon 列舉**](../classes/dialogs_dialogcommon.md#3-列舉型別清單) | 提示圖示 (`None`, `Information`, `Warning`, `Error`, `Question`)。 |
| [`MessageBoxDefaultButton`](../classes/dialogs_dialogcommon.md#3-列舉型別清單) | `DialogCommon.hpp` | [**MessageBoxDefaultButton 列舉**](../classes/dialogs_dialogcommon.md#3-列舉型別清單) | 預設焦點按鈕 (`Button1`, `Button2`, `Button3`)。 |
| [`DialogBackendMode`](../classes/dialogs_dialogcommon.md#3-列舉型別清單) | `DialogCommon.hpp` | [**DialogBackendMode 列舉**](../classes/dialogs_dialogcommon.md#3-列舉型別清單) | 後端模式 (`Auto`, `Native`, `BuiltIn`)。 |
| [`FileFilterItem`](../classes/dialogs_dialogcommon.md#4-filefilteritem-結構) | `DialogCommon.hpp` | [**FileFilterItem 結構**](../classes/dialogs_dialogcommon.md#4-filefilteritem-結構) | 結構化檔案篩選項目 (`description`, `patterns`)。 |

---

## 相關實戰指南
* [**對話方塊與檔案選取器實戰指南**](../guides/dialogs_and_file_picker_guide.md)
* [**主題與設計代幣規格**](../classes/theme_theme.md)
* [**Application 與 Form 視窗架構**](../classes/app_form.md)
