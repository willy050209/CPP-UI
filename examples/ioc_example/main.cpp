#include "ui/app/Application.hpp"
#include "ui/app/AppBuilder.hpp"
#include "services/ConsoleLogger.hpp"
#include "services/OrderService.hpp"
#include "viewmodels/OrderViewModel.hpp"
#include "views/OrderForm.hpp"

int main() {
    auto builder = cppui::app::Application::CreateBuilder();

    // 1. 註冊 Singleton 日誌服務
    builder.Services().AddSingleton<ILogger, ConsoleLogger>();

    // 2. 註冊 Transient 業務服務（由 IoC 工廠自動注入 ILogger）
    builder.Services().AddTransient<IOrderService>([](cppui::ioc::IServiceProvider& sp) {
        return std::make_shared<OrderService>(sp.GetRequiredService<ILogger>());
    });

    // 3. 註冊 ViewModel 與 Form
    builder.Services().AddTransient<OrderViewModel>([](cppui::ioc::IServiceProvider& sp) {
        return std::make_shared<OrderViewModel>(sp.GetRequiredService<IOrderService>());
    });

    builder.Services().AddTransient<OrderForm>([](cppui::ioc::IServiceProvider& sp) {
        return std::make_shared<OrderForm>(sp.GetRequiredService<OrderViewModel>());
    });

    // 4. 構建 Host 並啟動應用程式
    auto app = builder.Build();
    return app.Run<OrderForm>();
}
