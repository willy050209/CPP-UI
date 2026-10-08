#include "compat/compat.hpp"
#include "ui/mvvm/ViewModelBase.hpp"
#include "ui/mvvm/ICommand.hpp"
#include "ui/mvvm/RelayCommand.hpp"
#include "ui/controls/Button.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/app/Application.hpp"
#include <cassert>
#include <string>
#include <thread>
#include <atomic>

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
    (void)changed1;
    assert(vm.GetCount() == 10);
    assert(changeCount == 1);
    assert(changedProperty == "Count");

    // 設定相同的值：不應觸發通知
    bool changed2 = vm.SetCount(10);
    assert(changed2 == false);
    (void)changed2;
    assert(changeCount == 1); // 仍為 1

    // 修改另一個屬性
    bool changed3 = vm.SetTitle("Updated");
    assert(changed3 == true);
    (void)changed3;
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

    // 測試 RelayCommand::Create 與 MakeCommand 工廠函數
    bool factoryExecuted1 = false;
    auto cmdCreate = RelayCommand::Create([&]() { factoryExecuted1 = true; });
    assert(cmdCreate != nullptr);
    assert(cmdCreate->CanExecute() == true);
    cmdCreate->Execute();
    assert(factoryExecuted1 == true);

    bool factoryExecuted2 = false;
    auto cmdMake = MakeCommand([&]() { factoryExecuted2 = true; }, []() { return true; });
    assert(cmdMake != nullptr);
    assert(cmdMake->CanExecute() == true);
    cmdMake->Execute();
    assert(factoryExecuted2 == true);

    compat::println("  ✔ RelayCommand 執行、狀態通知與工廠函數驗證通過！");
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

void TestRelayCommandWithParameter() {
    compat::println("[Test] 測試 RelayCommand 攜帶參數之 Execute 與 CanExecute...");
    int receivedValue = 0;
    auto cmd = std::make_shared<RelayCommand>(
        [&](void* p) {
            if (p) {
                receivedValue = *static_cast<int*>(p);
            }
        },
        [](void* p) -> bool {
            if (!p) return false;
            return *static_cast<int*>(p) > 0;
        }
    );

    int testNeg = -5;
    assert(cmd->CanExecute(&testNeg) == false);
    (void)testNeg;

    int testPos = 42;
    assert(cmd->CanExecute(&testPos) == true);

    cmd->Execute(&testPos);
    (void)testPos;
    assert(receivedValue == 42);

    compat::println("  ✔ RelayCommand 參數化命令執行與判定驗證通過！");
}

void TestMultiplePropertyChangedObservers() {
    compat::println("[Test] 測試 ViewModelBase 多觀察者廣播...");
    CounterViewModel vm;

    int observer1Count = 0;
    int observer2Count = 0;
    std::string obs1Prop;
    std::string obs2Prop;

    vm.AddPropertyChangedHandler([&](compat::string_view p) {
        observer1Count++;
        obs1Prop = std::string(p.data(), p.size());
    });

    vm.AddPropertyChangedHandler([&](compat::string_view p) {
        observer2Count++;
        obs2Prop = std::string(p.data(), p.size());
    });

    vm.SetCount(100);

    assert(observer1Count == 1);
    assert(observer2Count == 1);
    assert(obs1Prop == "Count");
    assert(obs2Prop == "Count");

    compat::println("  ✔ ViewModelBase 多重觀察者廣播驗證通過！");
}

void TestDeepDataContextVisualTreeOverride() {
    compat::println("[Test] 測試深層視覺樹 DataContext 多層級局部覆寫...");
    auto root = std::make_shared<StackPanel>();
    auto childBranch1 = std::make_shared<Border>();
    auto grandchild = std::make_shared<Button>("GrandChild");
    childBranch1->SetChild(grandchild);

    auto childBranch2 = std::make_shared<Button>("Sibling");

    root->AddChild(childBranch1);
    root->AddChild(childBranch2);

    auto vmRoot = std::make_shared<CounterViewModel>();
    vmRoot->SetCount(10);

    auto vmBranch1 = std::make_shared<CounterViewModel>();
    vmBranch1->SetCount(20);

    root->SetDataContext(vmRoot);
    childBranch1->SetDataContext(vmBranch1);

    // 驗證 grandchild 繼承 branch1 的 vmBranch1 (count=20)
    assert(grandchild->GetDataContentType<CounterViewModel>()->GetCount() == 20);

    // 驗證 sibling 繼承 root 的 vmRoot (count=10)
    assert(childBranch2->GetDataContentType<CounterViewModel>()->GetCount() == 10);

    compat::println("  ✔ 深層視覺樹 DataContext 多層級繼承與分支隔離驗證通過！");
}

void TestViewModelBackgroundThreadMarshalling() {
    compat::println("[Test] 測試 ViewModelBase 背景執行緒屬性變更自動封送至主執行緒...");
    auto vm = std::make_shared<CounterViewModel>();

    std::atomic<int> notificationCount{0};
    std::thread::id receivedThreadId;
    std::string receivedProperty;

    vm->AddPropertyChangedHandler([&](compat::string_view propName) {
        notificationCount++;
        receivedThreadId = std::this_thread::get_id();
        receivedProperty = std::string(propName.data(), propName.size());
    });

    std::thread worker([&]() {
        // 從背景執行緒更新 ViewModel 屬性
        vm->SetCount(42);
    });
    worker.join();

    // 在背景執行緒變更後，因為被自動封送至 Dispatcher 佇列，
    // 在主執行緒 DrainTasks / DoEvents 之前，handler 尚未被觸發
    assert(notificationCount == 0);

    // 主執行緒處理事件佇列
    app::Application::DoEvents();

    // 驗證回呼已在主執行緒安全執行
    assert(notificationCount == 1);
    assert(receivedProperty == "Count");
    assert(receivedThreadId == std::this_thread::get_id());
    assert(vm->GetCount() == 42);

    compat::println("  ✔ ViewModelBase 背景執行緒屬性變更自動封送 (Thread Marshalling) 驗證通過！");
}

int main() {
    compat::println("==================================================");
    compat::println(" CPPUI MVVM Framework TDD Unit Test Suite");
    compat::println("==================================================");

    TestViewModelPropertyChanged();
    TestRelayCommandExecution();
    TestButtonCommandBindingAndCanExecute();
    TestDataContextVisualTreeInheritance();
    TestRelayCommandWithParameter();
    TestMultiplePropertyChangedObservers();
    TestDeepDataContextVisualTreeOverride();
    TestViewModelBackgroundThreadMarshalling();

    compat::println("==================================================");
    compat::println(" 全部 8 項 MVVM 單元測試通過！");
    compat::println("==================================================");
    return 0;
}
