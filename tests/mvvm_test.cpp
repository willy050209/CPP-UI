#include "compat/compat.hpp"
#include "ui/mvvm/ViewModelBase.hpp"
#include "ui/mvvm/ICommand.hpp"
#include "ui/mvvm/RelayCommand.hpp"
#include "ui/controls/Button.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include <cassert>
#include <string>

using namespace cppui;
using namespace cppui::mvvm;
using namespace cppui::controls;
using namespace cppui::layout;

// 測試 ViewModel
class CounterViewModel : public ViewModelBase {
public:
    int GetCount() const noexcept { return m_count; }

    bool SetCount(int newCount) {
        return SetProperty(m_count, newCount, "Count");
    }

    std::string GetTitle() const noexcept { return m_title; }

    bool SetTitle(compat::string_view newTitle) {
        std::string s(newTitle.data(), newTitle.size());
        return SetProperty(m_title, s, "Title");
    }

private:
    int m_count = 0;
    std::string m_title = "Initial Title";
};

void TestViewModelPropertyChanged() {
    compat::println("[Test] 測試 ViewModelBase 屬性變更通知...");
    CounterViewModel vm;

    std::string changedProperty;
    int changeCount = 0;
    vm.AddPropertyChangedHandler([&](compat::string_view propName) {
        changedProperty = std::string(propName.data(), propName.size());
        changeCount++;
    });

    // 第一次修改值：應觸發通知
    bool changed1 = vm.SetCount(10);
    assert(changed1 == true);
    assert(vm.GetCount() == 10);
    assert(changeCount == 1);
    assert(changedProperty == "Count");

    // 設定相同的值：不應觸發通知
    bool changed2 = vm.SetCount(10);
    assert(changed2 == false);
    assert(changeCount == 1); // 仍為 1

    // 修改另一個屬性
    bool changed3 = vm.SetTitle("Updated");
    assert(changed3 == true);
    assert(changeCount == 2);
    assert(changedProperty == "Title");

    compat::println("  ✔ ViewModelBase 屬性變更與重複值保護驗證通過！");
}

void TestRelayCommandExecution() {
    compat::println("[Test] 測試 RelayCommand 執行與可執行狀態...");
    bool executed = false;
    bool canRun = false;

    auto cmd = std::make_shared<RelayCommand>(
        [&]() { executed = true; },
        [&]() { return canRun; }
    );

    assert(cmd->CanExecute() == false);
    assert(executed == false);

    canRun = true;
    assert(cmd->CanExecute() == true);

    cmd->Execute();
    assert(executed == true);

    // 測試 CanExecuteChanged 通知
    bool canExecuteChangedNotified = false;
    cmd->AddCanExecuteChangedHandler([&]() {
        canExecuteChangedNotified = true;
    });

    cmd->NotifyCanExecuteChanged();
    assert(canExecuteChangedNotified == true);

    compat::println("  ✔ RelayCommand 執行與狀態通知驗證通過！");
}

void TestButtonCommandBindingAndCanExecute() {
    compat::println("[Test] 測試 Button 命令綁定與 IsEnabled 自動連動...");
    bool cmdRan = false;
    bool allowed = false;

    auto cmd = std::make_shared<RelayCommand>(
        [&]() { cmdRan = true; },
        [&]() { return allowed; }
    );

    auto btn = std::make_shared<Button>("Action");
    btn->SetCommand(cmd);

    // 由於 allowed 初始為 false，按鈕應自動被禁用
    assert(btn->IsEnabled() == false);

    // 模擬點擊被禁用的按鈕，命令不應執行
    btn->OnClick([&]() {});
    assert(cmdRan == false);

    // 變更條件並廣播狀態變更
    allowed = true;
    cmd->NotifyCanExecuteChanged();

    // 按鈕應自動變為啟用狀態
    assert(btn->IsEnabled() == true);

    // 觸發按鈕點擊
    // 透過外部直接呼叫或者內部狀態機
    // 這裡驗證 Command 本身可被執行
    if (btn->IsEnabled() && btn->GetCommand()->CanExecute()) {
        btn->GetCommand()->Execute();
    }
    assert(cmdRan == true);

    compat::println("  ✔ Button Command 綁定與自動啟用/禁用連動驗證通過！");
}

void TestDataContextVisualTreeInheritance() {
    compat::println("[Test] 測試 DataContext 沿視覺樹繼承...");
    auto rootPanel = std::make_shared<StackPanel>();
    auto border = std::make_shared<Border>();
    auto button = std::make_shared<Button>("Click");

    border->SetChild(button);
    rootPanel->AddChild(border);

    auto vm = std::make_shared<CounterViewModel>();
    vm->SetCount(99);

    // 在未設定 DataContext 前，所有節點均應為 nullptr
    assert(button->GetDataContext() == nullptr);
    assert(border->GetDataContext() == nullptr);
    assert(rootPanel->GetDataContext() == nullptr);

    // 僅在最頂層 rootPanel 設定 DataContext
    rootPanel->SetDataContext(vm);

    // 子節點 border 與孫節點 button 應自動沿視覺樹繼承取得 vm
    assert(rootPanel->GetDataContext() == vm);
    assert(border->GetDataContext() == vm);
    assert(button->GetDataContext() == vm);

    auto resolvedVm = button->GetDataContentType<CounterViewModel>();
    assert(resolvedVm != nullptr);
    assert(resolvedVm->GetCount() == 99);

    // 若子節點手動覆蓋自己的 DataContext，應取得自己的實例
    auto customVm = std::make_shared<CounterViewModel>();
    customVm->SetCount(777);
    button->SetDataContext(customVm);

    assert(button->GetDataContentType<CounterViewModel>()->GetCount() == 777);
    assert(border->GetDataContentType<CounterViewModel>()->GetCount() == 99);

    compat::println("  ✔ DataContext 視覺樹向下繼承與局部覆寫驗證通過！");
}

int main() {
    compat::println("==================================================");
    compat::println(" CPPUI MVVM Framework TDD Unit Test Suite");
    compat::println("==================================================");

    TestViewModelPropertyChanged();
    TestRelayCommandExecution();
    TestButtonCommandBindingAndCanExecute();
    TestDataContextVisualTreeInheritance();

    compat::println("==================================================");
    compat::println(" 全部 4 項 MVVM 單元測試通過！");
    compat::println("==================================================");
    return 0;
}
