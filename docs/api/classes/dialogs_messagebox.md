# MessageBox 模態訊息方塊

- **命名空間**: `cppui::dialogs`
- **標頭檔**: `include/ui/dialogs/MessageBox.hpp`, `include/ui/dialogs/MessageBoxForm.hpp`
- **實作檔**: `src/dialogs/MessageBox.cpp`, `src/dialogs/MessageBoxForm.cpp`

---

## 1. 類別概述

`MessageBox` 提供標準桌面應用的靜態模態對話框介面，支援單一訊息、自訂標題、按鈕組合、狀態圖示與主控表單綁定。

底層支援雙後端模式：在 Windows 上透過 `Win32DialogService` 呼叫原生 `MessageBoxW`，在跨平台或指定 BuiltIn 模式下透過 `MessageBoxForm` 呈現主題適配視窗。

---

## 2. API 宣告與多載方法

```cpp
namespace cppui {
namespace dialogs {

class MessageBox {
public:
    static app::DialogResult Show(compat::string_view text);

    static app::DialogResult Show(compat::string_view text, compat::string_view caption);

    static app::DialogResult Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons);

    static app::DialogResult Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons, MessageBoxIcon icon);

    static app::DialogResult Show(compat::string_view text, compat::string_view caption,
                                  MessageBoxButtons buttons, MessageBoxIcon icon,
                                  MessageBoxDefaultButton defaultButton);

    static app::DialogResult Show(std::shared_ptr<app::Form> owner,
                                  compat::string_view text,
                                  compat::string_view caption = "",
                                  MessageBoxButtons buttons = MessageBoxButtons::OK,
                                  MessageBoxIcon icon = MessageBoxIcon::None,
                                  MessageBoxDefaultButton defaultButton = MessageBoxDefaultButton::Button1);

    static app::DialogResult ShowWithMode(DialogBackendMode mode,
                                          std::shared_ptr<app::Form> owner,
                                          compat::string_view text,
                                          compat::string_view caption = "",
                                          MessageBoxButtons buttons = MessageBoxButtons::OK,
                                          MessageBoxIcon icon = MessageBoxIcon::None,
                                          MessageBoxDefaultButton defaultButton = MessageBoxDefaultButton::Button1);

    static void SetBackendMode(DialogBackendMode mode) noexcept;
    COMPAT_NODISCARD static DialogBackendMode GetBackendMode() noexcept;
};

} // namespace dialogs
} // namespace cppui
```

---

## 3. 回傳值 (DialogResult) 對應表

無論底層是 Win32 原生或是 BuiltIn Form，回傳結果均映射至統一的 `cppui::app::DialogResult`：

| 操作動作 | DialogResult 回傳值 |
| :--- | :--- |
| 使用者點擊 [確定] | `DialogResult::OK` |
| 使用者點擊 [取消] 或關閉視窗 | `DialogResult::Cancel` |
| 使用者點擊 [是] | `DialogResult::Yes` |
| 使用者點擊 [否] | `DialogResult::No` |
| 使用者點擊 [重試] | `DialogResult::Retry` |
| 使用者點擊 [中止] | `DialogResult::Abort` |
| 使用者點擊 [略過] | `DialogResult::Ignore` |

---

## 4. 純 CPPUI 主題視窗: MessageBoxForm

在 `DialogBackendMode::BuiltIn` 模式下，或在 Linux / Headless CI 環境中，MessageBox 會自動實體化 `MessageBoxForm`：

```cpp
class MessageBoxForm : public app::Form {
public:
    MessageBoxForm(compat::string_view text,
                   compat::string_view caption,
                   MessageBoxButtons buttons = MessageBoxButtons::OK,
                   MessageBoxIcon icon = MessageBoxIcon::None,
                   MessageBoxDefaultButton defaultButton = MessageBoxDefaultButton::Button1);

    COMPAT_NODISCARD compat::string_view GetMessageText() const noexcept;
    COMPAT_NODISCARD MessageBoxButtons GetButtons() const noexcept;
    COMPAT_NODISCARD MessageBoxIcon GetIcon() const noexcept;

    // 程式化模擬點擊 (供單元測試與 Headless CI 驗證)
    void SimulateButtonClick(app::DialogResult result);

protected:
    void OnLoad() override;
    void ApplyTheme(const theme::Theme& theme) override;
};
```

### 特色
- 自動監聽並同步 `ThemeManager` 深淺色主題。
- 支援 `SimulateButtonClick` 介面，無需彈出實體視窗即可在單元測試中完成無阻塞非同步驗證。

---

## 5. 程式碼使用範例

### 5.1 簡易訊息提示
```cpp
#include "ui/dialogs/MessageBox.hpp"

using namespace cppui::dialogs;

// 顯示預設 OK 提示方塊
MessageBox::Show("應用程式設定已儲存！", "提示");
```

### 5.2 帶有詢問與按鈕分支
```cpp
auto result = MessageBox::Show(
    "是否確定要刪除選取的項目？此動作無法復原。",
    "確認刪除",
    MessageBoxButtons::YesNo,
    MessageBoxIcon::Question,
    MessageBoxDefaultButton::Button2 // 預設聚焦於「否」
);

if (result == cppui::app::DialogResult::Yes) {
    DeleteSelectedItem();
}
```

### 5.3 強制指定純 CPPUI 主題模式
```cpp
// 強制使用純 CPPUI Form 佈景主題外觀 (符合應用程式暗色風格)
auto result = MessageBox::ShowWithMode(
    DialogBackendMode::BuiltIn,
    thisFormSharedPtr,
    "自訂暗色風格提示訊息",
    "佈景主題通知",
    MessageBoxButtons::OK,
    MessageBoxIcon::Information
);
```
