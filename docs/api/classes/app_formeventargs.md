# FormEventArgs 與相關列舉型別

- **命名空間**: `cppui::app`
- **標頭檔**: `include/ui/app/FormEventArgs.hpp`

---

## 概述

`FormEventArgs.hpp` 定義了 CPPUI 桌面表單視窗在生命週期變更時所傳遞的事件引數，以及視窗外觀與模態對話方塊的枚舉型別（對標 Windows Forms `FormClosingEventArgs`, `FormClosedEventArgs`, `DialogResult`）。

---

## 型別宣告

```cpp
namespace cppui {
namespace app {

enum class FormStartPosition {
    Manual,
    CenterScreen,
    WindowsDefault
};

enum class WindowState {
    Normal,
    Minimized,
    Maximized
};

enum class DialogResult {
    None,
    OK,
    Cancel,
    Abort,
    Retry,
    Ignore,
    Yes,
    No
};

class FormClosingEventArgs {
public:
    explicit FormClosingEventArgs(bool cancel = false) : Cancel(cancel) {}
    bool Cancel = false; // 設為 true 即可取消視窗關閉行為
};

class FormClosedEventArgs {
public:
    FormClosedEventArgs() = default;
};

} // namespace app
} // namespace cppui
```

---

## 關鍵欄位與說明

### `FormClosingEventArgs::Cancel`
- **型別**: `bool`（預設為 `false`）。
- **說明**: 當視窗使用者按下關閉按鈕或調用 `Form::Close()` 時，事件處理常式（如 `OnFormClosing` 或 `AddClosingHandler`）可將此旗標設為 `true`，以**攔截並取消關閉程序**（常用於未儲存變更提示或確認對話框）。

### `DialogResult`
- **說明**: 當以模態對話方塊模式開啟視窗（`form->ShowDialog()`）時，視窗內部透過 `SetDialogResult(DialogResult::OK)` 關閉視窗後回傳給調用方的結果。

---

## 使用範例

```cpp
auto form = std::make_shared<Form>();

// 攔截取消關閉範例
form->AddClosingHandler([](Form& sender, FormClosingEventArgs& e) {
    if (HasUnsavedChanges()) {
        e.Cancel = true; // 阻止視窗關閉
    }
});
```
