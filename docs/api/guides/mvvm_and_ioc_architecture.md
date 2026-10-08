# 現代 MVVM 與依賴反轉 (IoC) 架構指南 (Modern MVVM & IoC Architecture Guide)

本指南介紹 CPPUI 如何透過 `Application`、`Form`、`ServiceCollection` 與 `ViewModelBase` / `RelayCommand` 打造高生產力、高可測試性與零耦合的現代桌面應用程式。

---

## 1. 架構核心理念

傳統 C++ 桌面應用往往將業務邏輯、UI 繪製與 Windows 訊息循環混雜在 `main()` 或自訂 Window 類別中，導致代碼難以維護且無法單元測試。

CPPUI 採用與現代 .NET (Generic Host + WinForms/WPF + CommunityToolkit.Mvvm) 相同的分層架構：

```
+-----------------------------------------------------------+
|                      Model (領域模型與服務)                |
|             (IWeatherService, Database, Network)          |
+-----------------------------------------------------------+
                              ^
                              | 依賴注入 (IoC)
+-----------------------------------------------------------+
|                   ViewModel (狀態與命令邏輯)               |
|            (ViewModelBase, RelayCommand, Signal<T>)       |
+-----------------------------------------------------------+
                              ^
                              | 資料與指令綁定 (Data & Command Binding)
+-----------------------------------------------------------+
|                       View (表單與視覺樹)                  |
|                 (Form, Fluent DSL, Controls)              |
+-----------------------------------------------------------+
                              |
                              v 託管執行
+-----------------------------------------------------------+
|               Application (全域靜態中樞與訊息泵)            |
|                  Application::Run(new MainForm())         |
+-----------------------------------------------------------+
```

---

## 2. 極速啟動：`Application::Run` 與 `Form` 生命週期

### 2.1 單行啟動視窗應用程式
現在您無須再手動宣告 `WindowDesc`、配置 `PlatformWindowFactory`、綁定 `SoftwareRenderTarget` 或撰寫 `while(!win->IsClosed()) WaitEvents(16)`！

只需繼承 `Form` 並調用 `Application::Run`：
```cpp
#include "ui/app/Application.hpp"
#include "ui/app/Form.hpp"
#include "ui/dsl/Fluent.hpp"

using namespace cppui;
using namespace cppui::app;
using namespace cppui::dsl;

class MyForm : public Form {
public:
    MyForm() {
        SetTitle("極簡應用程式");
        SetFormSize(800, 600);

        SetContent(
            VStack({
                Label("Hello World!"),
                Button("關閉").OnClick([this]() { Close(); })
            })
        );
    }
};

int main() {
    return Application::Run(new MyForm());
}
```
`Application::Run(new MyForm())` 會立即接管指標所有權並封裝為 `std::shared_ptr<Form>`，安全杜絕記憶體洩漏。

### 2.2 表單生命週期與取消關閉
`Form` 提供了完整的視窗事件：
1. `OnLoad()`：視窗首次顯示前調用，用於加載初始數據。
2. `OnShown()`：視窗已呈現在螢幕上時調用。
3. `OnFormClosing(FormClosingEventArgs& e)`：視窗即將關閉時調用。若使用者尚未存檔，可設定 `e.Cancel = true` 取消關閉！
4. `OnFormClosed(FormClosedEventArgs& e)`：視窗已關閉並釋放底層原生資源。

```cpp
void MyForm::OnFormClosing(FormClosingEventArgs& e) {
    Form::OnFormClosing(e);
    if (m_hasUnsavedData) {
        // 取消關閉操作
        e.Cancel = true;
        compat::println("資料尚未存檔，視窗拒絕關閉！");
    }
}
```

---

## 3. 依賴反轉 (IoC) 與現代 Generic Host

借助 `Application::CreateBuilder()`，您可以享受如現代 ASP.NET Core / .NET 6+ 一樣直覺的依賴注入體驗：

```cpp
int main() {
    auto builder = Application::CreateBuilder();

    // 1. 註冊服務
    builder.Services().AddSingleton<IDataService, SqlDataService>();

    // 2. 註冊 ViewModel (透過工廠注入依賴)
    builder.Services().AddTransient<MainViewModel>([](ioc::IServiceProvider& sp) {
        auto svc = sp.GetRequiredService<IDataService>();
        return std::make_shared<MainViewModel>(svc);
    });

    // 3. 註冊 Form 主視窗
    builder.Services().AddTransient<MainForm>([](ioc::IServiceProvider& sp) {
        auto vm = sp.GetRequiredService<MainViewModel>();
        return std::make_shared<MainForm>(vm);
    });

    // 4. 構建並啟動
    auto app = builder.Build();
    return app.Run<MainForm>();
}
```

---

## 4. 現代 MVVM 模式：ViewModel、RelayCommand 與 DataContext

### 4.1 定義 ViewModel
ViewModel 繼承自 `ViewModelBase`，維護狀態與指令：
```cpp
class CounterViewModel : public cppui::mvvm::ViewModelBase {
public:
    CounterViewModel() {
        IncrementCommand = std::make_shared<RelayCommand>([this]() {
            Count = Count.Get() + 1;
        });

        ResetCommand = std::make_shared<RelayCommand>([this]() {
            Count = 0;
        }, [this]() {
            return Count.Get() > 0; // 當 Count > 0 時才允許重設
        });

        Count.Subscribe([this](int) {
            ResetCommand->NotifyCanExecuteChanged();
        });
    }

    Signal<int> Count{0};
    std::shared_ptr<RelayCommand> IncrementCommand;
    std::shared_ptr<RelayCommand> ResetCommand;
};
```

### 4.2 視圖綁定指令與狀態
在 Form 中使用 Fluent DSL，直接鏈式綁定指令與狀態：
```cpp
class CounterForm : public Form {
public:
    explicit CounterForm(std::shared_ptr<CounterViewModel> vm) {
        SetDataContext(vm);

        SetContent(
            VStack({
                Label("計數器")
                    .BindText(vm->Count, [](int c) { return "數值: " + std::to_string(c); }),

                Button("增加")
                    .Command(vm->IncrementCommand),

                Button("歸零")
                    .Command(vm->ResetCommand) // 當 CanExecute 為 false 時按鈕自動禁用！
            })
        );
    }
};
```

### 4.3 DataContext 沿視覺樹繼承
當您在 `Form` 或上層容器呼叫 `SetDataContext(vm)` 時，任何子元件在未特別設定自身 DataContext 的情況下，呼叫 `child->GetDataContext()` 皆會自動沿視覺樹向上繼承取得父層的 ViewModel，達成零樣板的上下文共用。

---

## 5. 請參閱 (See Also)
* [`Application` 靜態中樞類別](../classes/app_application.md)
* [`Form` 視窗基底類別](../classes/app_form.md)
* [`ServiceCollection` 依賴反轉類別](../classes/ioc_servicecollection.md)
* [`ViewModelBase` 與 `RelayCommand`](../classes/mvvm_viewmodelbase.md)
