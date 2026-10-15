# RelayCommand 類別 (cppui::mvvm::RelayCommand)

命令模式委派實作類別，實作 `ICommand` 介面，允許直接傳入 Lambda 運算式定義行為邏輯與可執行性查詢（對標 WPF/Avalonia `RelayCommand`）。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::mvvm`](../namespaces/cppui_mvvm.md)
* **標頭檔**：`#include <ui/mvvm/RelayCommand.hpp>`
* **介面標頭檔**：`#include <ui/mvvm/ICommand.hpp>`
* **模組**：`CPPUI MVVM Framework`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace mvvm {

class ICommand {
public:
    virtual ~ICommand() = default;
    COMPAT_NODISCARD virtual bool CanExecute(void* parameter = nullptr) const = 0;
    virtual void Execute(void* parameter = nullptr) = 0;

    using CanExecuteChangedHandler = std::function<void()>;
    virtual void AddCanExecuteChangedHandler(CanExecuteChangedHandler handler) = 0;
    virtual void NotifyCanExecuteChanged() = 0;
};

class RelayCommand : public ICommand {
public:
    using ExecuteFunc = std::function<void(void*)>;
    using CanExecuteFunc = std::function<bool(void*)>;

    explicit RelayCommand(ExecuteFunc execute, CanExecuteFunc canExecute = nullptr);
    explicit RelayCommand(std::function<void()> execute, std::function<bool()> canExecute = nullptr);
    ~RelayCommand() override = default;

    // 快速工廠方法
    template<typename... Args>
    static std::shared_ptr<RelayCommand> Create(Args&&... args);

    COMPAT_NODISCARD bool CanExecute(void* parameter = nullptr) const override;
    void Execute(void* parameter = nullptr) override;
    void AddCanExecuteChangedHandler(CanExecuteChangedHandler handler) override;
    void NotifyCanExecuteChanged() override;
};

// 命名空間便捷工廠輔助函式
inline std::shared_ptr<RelayCommand> MakeCommand(RelayCommand::ExecuteFunc execute, RelayCommand::CanExecuteFunc canExecute = nullptr);
inline std::shared_ptr<RelayCommand> MakeCommand(std::function<void()> execute, std::function<bool()> canExecute = nullptr);

} // namespace mvvm
} // namespace cppui
```

---

## 簡介 (Description)

`RelayCommand` 將按鈕點擊等互動行為與業務邏輯解耦：
1. **Lambda 定義邏輯**：建構時可傳入無參數或帶參數的 `execute` 與 `canExecute` Lambda。
2. **快速工廠方法**：提供 `MakeCommand(...)` 與 `RelayCommand::Create(...)`，無需繁瑣之 `std::make_shared<RelayCommand>(...)`。
3. **與控制項自動連動**：
   - 當按鈕以 `btn->SetCommand(cmd)` 或 DSL `.Command(cmd)` 綁定後，按鈕會自動訂閱 `CanExecuteChanged`。
   - 若 `CanExecute()` 回傳 `false`，按鈕自動切換為 `IsEnabled(false)` 禁用外觀，無法點擊。
   - 當條件改變時調用 `cmd->NotifyCanExecuteChanged()`，按鈕自動復原為可用狀態。
4. **點擊自動派發**：按鈕在點擊事件觸發時自動執行 `cmd->Execute()`。

---

## 範例程式碼 (Example)

```cpp
#include "compat/Compat.hpp"
#include "ui/mvvm/RelayCommand.hpp"
#include "ui/controls/Button.hpp"

void SetupCommand() {
    using namespace cppui::mvvm;
    using namespace cppui::controls;

    bool hasPermission = false;
    
    // 使用便捷工廠函數 MakeCommand 建立命令
    auto saveCommand = MakeCommand(
        []() { compat::println("儲存成功！"); },
        [&]() { return hasPermission; }
    );

    auto btn = std::make_shared<Button>("儲存檔案");
    btn->SetCommand(saveCommand);

    // 初始為禁用狀態
    assert(btn->IsEnabled() == false);

    // 授權並通知狀態變更
    hasPermission = true;
    saveCommand->NotifyCanExecuteChanged();

    // 按鈕自動轉為啟用狀態
    assert(btn->IsEnabled() == true);
}
```

---

## 請參閱 (See Also)
* [`ViewModelBase` 視圖模型基底](mvvm_viewmodelbase.md)
* [`Button` 控制項](controls_button.md)
* [`現代 MVVM 與 IoC 架構指南`](../guides/mvvm_and_ioc_architecture.md)
