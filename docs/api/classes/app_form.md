# Form 類別 (cppui::app::Form)

現代化桌面視窗基底類別，繼承自 `ContentControl`。封裝底層平台原生視窗、事件循環、雙階段約束排版與生命週期事件（對標 .NET `System.Windows.Forms.Form`）。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::app`](../namespaces/cppui_app.md)
* **標頭檔**：`#include <ui/app/Form.hpp>`
* **基底類別**：[`cppui::controls::ContentControl`](controls_contentcontrol.md)
* **模組**：`CPPUI Application Framework`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace app {

enum class DialogResult {
    None, OK, Cancel, Abort, Retry, Ignore, Yes, No
};

enum class FormStartPosition {
    Manual, CenterScreen, WindowsDefaultLocation
};

enum class WindowState {
    Normal, Minimized, Maximized
};

class FormClosingEventArgs {
public:
    bool Cancel = false;
    bool CloseReasonIsUser = true;
};

class FormClosedEventArgs {
public:
    bool CloseReasonIsUser = true;
};

class Form : public controls::ContentControl {
public:
    Form();
    ~Form() override;

    // 視窗屬性
    COMPAT_NODISCARD compat::string_view GetTitle() const noexcept;
    void SetTitle(compat::string_view title);

    COMPAT_NODISCARD core::Size GetFormSize() const noexcept;
    void SetFormSize(uint32_t width, uint32_t height);

    COMPAT_NODISCARD FormStartPosition GetStartPosition() const noexcept;
    void SetStartPosition(FormStartPosition startPos) noexcept;

    COMPAT_NODISCARD WindowState GetWindowState() const noexcept;
    void SetWindowState(WindowState state);

    COMPAT_NODISCARD DialogResult GetDialogResult() const noexcept;
    void SetDialogResult(DialogResult result) noexcept;

    // 視窗行為
    virtual void Show();
    virtual void Hide();
    virtual void Close();
    virtual DialogResult ShowDialog(std::shared_ptr<Form> owner = nullptr);

    // 生命週期虛擬方法 (可由衍生類別覆寫)
    virtual void OnLoad();
    virtual void OnShown();
    virtual void OnActivated();
    virtual void OnDeactivate();
    virtual void OnFormClosing(FormClosingEventArgs& e);
    virtual void OnFormClosed(FormClosedEventArgs& e);

    // 外部監聽器註冊
    using FormClosingHandler = std::function<void(Form&, FormClosingEventArgs&)>;
    using FormClosedHandler = std::function<void(Form&, FormClosedEventArgs&)>;
    void AddClosingHandler(FormClosingHandler handler);
    void AddClosedHandler(FormClosedHandler handler);

    // 原生視窗存取
    COMPAT_NODISCARD std::shared_ptr<pal::IPlatformWindow> GetPlatformWindow() const noexcept;
};

} // namespace app
} // namespace cppui
```

---

## 簡介 (Description)

`Form` 代表一個頂層作業系統桌面視窗：
1. **單一內容容器**：繼承自 `ContentControl`，可使用 `SetContent(...)` 容納任意 `UIElement` 樹狀結構（配合 Fluent DSL 如 `VStack`, `Grid`, `Border` 等）。
2. **完整生命週期狀態機**：
   - 首次顯示時依序觸發：`OnLoad()` $\rightarrow$ `OnShown()` $\rightarrow$ `OnActivated()`。
   - 關閉視窗時依序觸發：`OnFormClosing(FormClosingEventArgs& e)` $\rightarrow$ 若 `e.Cancel == false` 則關閉視窗並觸發 `OnFormClosed(FormClosedEventArgs& e)`。
3. **取消關閉機制**：在 `OnFormClosing` 或 `AddClosingHandler` 中設定 `e.Cancel = true` 即可取消關閉動作（例如跳出「資料尚未存檔，確定要退出嗎？」對話框）。
4. **模態對話框 (`ShowDialog`)**：阻塞執行模態訊息循環，直到使用者設定 `DialogResult` 或關閉對話框為止，並回傳該結果。

---

## 方法說明 (Methods)

### `Show()` / `Hide()` / `Close()`
* `Show()`：初始化原生視窗、執行首次排版與繪製，觸發 `OnLoad` 與 `OnShown` 並呈現在螢幕上。
* `Hide()`：隱藏視窗並觸發 `OnDeactivate`。
* `Close()`：請求關閉視窗。觸發 `OnFormClosing`，若未被取消則釋放底層視窗資源、從 `Application::GetOpenForms()` 移除並觸發 `OnFormClosed`。

### `ShowDialog(std::shared_ptr<Form> owner)`
以模態（Modal）方式呈現表單，阻塞當前呼叫點並運行私有事件泵，直到設定 `SetDialogResult` 或表單關閉，回傳 `DialogResult`。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/Form.hpp"
#include "ui/dsl/Fluent.hpp"

using namespace cppui;
using namespace cppui::app;
using namespace cppui::dsl;

class EditorForm : public Form {
public:
    EditorForm() {
        SetTitle("文字編輯器");
        SetFormSize(800, 600);

        SetContent(
            VStack({
                Label("請輸入內容:"),
                TextBox().Placeholder("在此輸入..."),
                Button("關閉視窗").OnClick([this]() {
                    Close();
                })
            })
        );
    }

    void OnFormClosing(FormClosingEventArgs& e) override {
        Form::OnFormClosing(e);
        compat::println("視窗即將關閉，檢查是否存檔...");
    }
};

int main() {
    return Application::Run(new EditorForm());
}
```

---

## 請參閱 (See Also)
* [`Application` 靜態中樞類別](app_application.md)
* [`ViewModelBase` 視圖模型基底](mvvm_viewmodelbase.md)
* [`現代 MVVM 與 IoC 架構指南`](../guides/mvvm_and_ioc_architecture.md)
