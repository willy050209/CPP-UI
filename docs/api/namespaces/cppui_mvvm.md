# cppui::mvvm 命名空間

- **標頭檔目錄**: `include/ui/mvvm/`

---

## 概述

`cppui::mvvm` 命名空間提供 CPPUI 框架的現代 MVVM（Model-View-ViewModel）架構基礎設施。對標微軟 WPF / .NET CommunityToolkit.Mvvm 規範，提供屬性變更通知機制、委派命令模式以及視覺樹 DataContext 自動向下傳遞繼承，協助開發者實現視圖（View）與業務邏輯（ViewModel/Model）的徹底解耦。

---

## 包含型別與介面

| 型別名稱 | 標頭檔 | 說明 |
| :--- | :--- | :--- |
| [`ViewModelBase`](file:///D:/P/CPP/CPPUI/docs/api/classes/mvvm_viewmodelbase.md) | `ViewModelBase.hpp` | 視圖模型基底類別，提供 `SetProperty` 與 `NotifyPropertyChanged` 屬性變更追蹤。 |
| [`ICommand`](file:///D:/P/CPP/CPPUI/docs/api/classes/mvvm_icommand.md) | `ICommand.hpp` | 命令模式純虛擬介面，定義 `Execute`、`CanExecute` 與狀態變更事件 `CanExecuteChanged`。 |
| [`RelayCommand`](file:///D:/P/CPP/CPPUI/docs/api/classes/mvvm_relaycommand.md) | `RelayCommand.hpp` | 輕量委派命令實作，允許以 Lambda 函式定義動作與執行條件，並廣播狀態通知。 |

---

## 典型使用模式

```cpp
#include "ui/mvvm/ViewModelBase.hpp"
#include "ui/mvvm/RelayCommand.hpp"

class CounterViewModel : public cppui::mvvm::ViewModelBase {
public:
    CounterViewModel() {
        IncrementCmd = std::make_shared<cppui::mvvm::RelayCommand>([this]() {
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
