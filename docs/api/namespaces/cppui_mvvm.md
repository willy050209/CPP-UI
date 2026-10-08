# cppui::mvvm 命名空間

- **標頭檔目錄**: `include/ui/mvvm/`

---

## 概述

`cppui::mvvm` 命名空間提供 CPPUI 框架的現代 MVVM（Model-View-ViewModel）架構基礎設施。對標微軟 WPF / .NET CommunityToolkit.Mvvm 規範，提供屬性變更通知機制、委派命令模式以及視覺樹 DataContext 自動向下傳遞繼承，協助開發者實現視圖（View）與業務邏輯（ViewModel/Model）的徹底解耦。

---

## 包含型別與介面

| 型別名稱 | 標頭檔 | 說明 |
| :--- | :--- | :--- |
| [`ViewModelBase`](../classes/mvvm_viewmodelbase.md) | `ViewModelBase.hpp` | 視圖模型基底類別，提供 `SetProperty`、`NotifyPropertyChanged` 屬性追蹤與**背景執行緒自動封送 (Thread Marshalling)** 至 UI 主執行緒之安全機制。 |
| [`ICommand`](../classes/mvvm_icommand.md) | `ICommand.hpp` | 命令模式純虛擬介面，定義 `Execute`、`CanExecute` 與狀態變更事件 `CanExecuteChanged`。 |
| [`RelayCommand`](../classes/mvvm_relaycommand.md) | `RelayCommand.hpp` | 輕量委派命令實作，允許以 Lambda 函式定義動作與執行條件，提供 `MakeCommand` 與 `Create` 工廠方法。 |

---

## 核心特性

### 1. 背景執行緒自動封送 (Thread Marshalling)
在非同步或多執行緒架構中，背景工作執行緒（Worker Thread）呼叫 `SetProperty` 或 `NotifyPropertyChanged` 時，`ViewModelBase` 會自動偵測當前執行緒狀態（透過 `Application::CheckAccess()`）。若發現處於背景執行緒，將自動透過全域 `Dispatcher` 將屬性變更通知封送至 UI 主執行緒執行，保障綁定控制項之執行緒安全性，避免跨執行緒並行存取引發的記憶體衝突或競態條件。

### 2. 簡化命令工廠 (MakeCommand)
提供命名空間全域工廠函數 `cppui::mvvm::MakeCommand(...)` 與類別工廠方法 `RelayCommand::Create(...)`，消除撰寫 `std::make_shared<cppui::mvvm::RelayCommand>(...)` 的繁瑣語法。

---

## 典型使用模式

```cpp
#include "ui/mvvm/ViewModelBase.hpp"
#include "ui/mvvm/RelayCommand.hpp"

class CounterViewModel : public cppui::mvvm::ViewModelBase {
public:
    CounterViewModel() {
        // 使用 MakeCommand 快速建立命令
        IncrementCmd = cppui::mvvm::MakeCommand([this]() {
            SetProperty(m_count, m_count + 1, "Count");
            IncrementCmd->NotifyCanExecuteChanged();
        }, [this]() {
            return m_count < 100;
        });
    }

    int GetCount() const noexcept { return m_count; }

    std::shared_ptr<cppui::mvvm::RelayCommand> IncrementCmd;

private:
    int m_count = 0;
};
```
