# ViewModelBase 類別 (cppui::mvvm::ViewModelBase)

MVVM 架構核心視圖模型基底類別，提供型別安全的屬性變更通知機制（對標 .NET `INotifyPropertyChanged` / `CommunityToolkit.Mvvm.ObservableObject`）。

---

## 元資料 (Metadata)

* **命名空間**：[`cppui::mvvm`](../namespaces/cppui_mvvm.md)
* **標頭檔**：`#include <ui/mvvm/ViewModelBase.hpp>`
* **模組**：`CPPUI MVVM Framework`
* **相容性**：`C++14`, `C++17`, `C++20`, `C++23`

---

## 語法 (Syntax)

```cpp
namespace cppui {
namespace mvvm {

class ViewModelBase {
public:
    virtual ~ViewModelBase() = default;

    using PropertyChangedCallback = std::function<void(compat::string_view)>;
    void AddPropertyChangedHandler(PropertyChangedCallback handler);

protected:
    void NotifyPropertyChanged(compat::string_view propertyName);

    template<typename T>
    bool SetProperty(T& storage, const T& value, compat::string_view propertyName);
};

} // namespace mvvm
} // namespace cppui
```

---

## 簡介 (Description)

`ViewModelBase` 是 MVVM 模式中 View 與 Model 之間的狀態樞紐：
1. **屬性變更通知**：支援註冊監聽器，當 ViewModel 內部狀態改變時即時通知 View。
2. **`SetProperty<T>` 輔助方法**：自動比對新舊值，當值相同時直接回傳 `false` 不觸發通知；當值不同時更新內部欄位、觸發 `NotifyPropertyChanged(propertyName)` 並回傳 `true`，防止無謂重繪。
3. **無縫整合 `Signal<T>`**：ViewModel 內的屬性可與 M6 的 `Signal<T>` 結合，達成完全宣告式的雙向資料流。

---

## 範例程式碼 (Example)

```cpp
#include "compat/compat.hpp"
#include "ui/mvvm/ViewModelBase.hpp"

class UserProfileViewModel : public cppui::mvvm::ViewModelBase {
public:
    int GetAge() const { return m_age; }
    void SetAge(int newAge) {
        SetProperty(m_age, newAge, "Age");
    }

private:
    int m_age = 18;
};
```

---

## 請參閱 (See Also)
* [`RelayCommand` 命令模式實作](mvvm_relaycommand.md)
* [`Form` 視窗類別](app_form.md)
* [`現代 MVVM 與 IoC 架構指南`](../guides/mvvm_and_ioc_architecture.md)
