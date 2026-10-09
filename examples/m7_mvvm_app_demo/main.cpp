/*
 * File: main.cpp
 * Purpose: Milestone 7 demo illustrating Application host builder, IoC DI, and MVVM patterns.
 */

#include <cstdint>
#include <memory>
#include <string>
#include "compat/compat.hpp"
#include "ui/app/Application.hpp"
#include "ui/app/Form.hpp"
#include "ui/app/AppBuilder.hpp"
#include "ui/mvvm/ViewModelBase.hpp"
#include "ui/mvvm/RelayCommand.hpp"
#include "ui/dsl/Fluent.hpp"
#include "ui/reactive/Signal.hpp"

namespace m7_demo {

namespace text = cppui::text;
using cppui::mvvm::ViewModelBase;
using cppui::mvvm::RelayCommand;
using cppui::reactive::Signal;
using cppui::app::Form;
using cppui::app::FormStartPosition;
using cppui::app::FormClosingEventArgs;
using cppui::app::FormClosedEventArgs;
using cppui::core::Color;
using cppui::core::Thickness;
using cppui::core::Size;

// =============================================================================
// 1. 領域服務介面與實作 (Domain Service & Implementation for IoC)
// =============================================================================

class IWeatherService {
public:
    virtual ~IWeatherService() = default;
    virtual std::string GetWeatherReport(compat::string_view city) = 0;
};

class WeatherService : public IWeatherService {
public:
    std::string GetWeatherReport(compat::string_view city) override {
        m_queryCount++;
        std::string c(city.data(), city.size());
        if (c.empty()) c = "Taipei";
        return c + " - 晴時多雲 26°C (查詢次數: " + compat::to_string(m_queryCount) + ")";
    }

private:
    int32_t m_queryCount = 0;
};

// =============================================================================
// 2. 視圖模型 (ViewModel with ViewModelBase, RelayCommand, and Signals)
// =============================================================================

class DashboardViewModel : public ViewModelBase {
public:
    explicit DashboardViewModel(std::shared_ptr<IWeatherService> weatherService)
        : m_weatherService(weatherService) {

        // 綁定指令動作 (RelayCommand)
        IncrementCommand = std::make_shared<RelayCommand>([this]() {
            Counter = Counter.Get() + 1;
            StatusMessage = "計數器已更新為: " + compat::to_string(Counter.Get());
        });

        ResetCommand = std::make_shared<RelayCommand>([this]() {
            Counter = 0;
            StatusMessage = "計數器已歸零！";
        }, [this]() {
            return Counter.Get() > 0;
        });

        FetchWeatherCommand = std::make_shared<RelayCommand>([this]() {
            if (m_weatherService) {
                WeatherReport = m_weatherService->GetWeatherReport(CityName.Get());
                StatusMessage = "已成功擷取天氣資料！";
            }
        });

        // 監聽 Counter 改變以更新 ResetCommand 的 CanExecute 狀態
        Counter.Subscribe([this](int32_t) {
            ResetCommand->NotifyCanExecuteChanged();
        });
    }

    Signal<int32_t> Counter{ 0 };
    Signal<std::string> CityName{ "Taipei" };
    Signal<std::string> WeatherReport{ "點擊按鈕查詢天氣..." };
    Signal<std::string> StatusMessage{ "歡迎體驗 CPPUI M7 現代化 Application 與 MVVM 架構！" };

    std::shared_ptr<RelayCommand> IncrementCommand;
    std::shared_ptr<RelayCommand> ResetCommand;
    std::shared_ptr<RelayCommand> FetchWeatherCommand;

private:
    std::shared_ptr<IWeatherService> m_weatherService;
};

// =============================================================================
// 3. 視窗表單 (Form View with Fluent DSL, Command Binding, and Lifecycle)
// =============================================================================

class MainDashboardForm : public Form {
public:
    explicit MainDashboardForm(std::shared_ptr<DashboardViewModel> vm)
        : m_vm(vm) {
        using namespace cppui;
        using namespace cppui::dsl;

        SetTitle("CPPUI M7 - Modern Application & MVVM Architecture Showcase");
        SetFormSize(1024, 700);
        SetStartPosition(FormStartPosition::CenterScreen);
        SetDataContext(vm);

        // 使用 Fluent RAD DSL 宣告視窗介面 (< 80 行極簡宣告)
        SetContent(
            Border(
                VStack({
                    // Header 橫幅
                    Border(
                        HStack({
                            Label("CPPUI M7")
                                .FontSize(22.0f)
                                .FontWeight(text::FontWeight::Bold)
                                .Foreground(Color(255, 255, 255, 255)),
                            Label("Application.Run + Form Lifecycle + MVVM & IoC Container")
                                .FontSize(13.0f)
                                .Foreground(Color(140, 175, 225, 255))
                                .Margin(14.0f, 6.0f, 0.0f, 0.0f)
                        })
                    )
                    .Background(Color(26, 30, 42, 255))
                    .BorderBrush(Color(45, 54, 75, 255))
                    .BorderThickness(0.0f, 0.0f, 0.0f, 1.0f)
                    .Padding(24.0f, 16.0f),

                    // 主工作區 (Grid: 左側計數器與表單、右側天氣服務與狀態日誌)
                    Grid({
                        // [Col 0] 左側卡片: MVVM 計數器與指令綁定
                        Border(
                            VStack({
                                Label("MVVM 響應式狀態 (ViewModel)")
                                    .FontSize(16.0f)
                                    .FontWeight(text::FontWeight::SemiBold)
                                    .Foreground(Color(230, 240, 255, 255)),

                                Label("計數器數值: 0")
                                    .FontSize(20.0f)
                                    .FontWeight(text::FontWeight::Bold)
                                    .Foreground(Color(80, 220, 140, 255))
                                    .Margin(0.0f, 8.0f)
                                    .BindText(vm->Counter, [](int32_t val) {
                                        return "計數器數值: " + compat::to_string(val);
                                    }),

                                HStack({
                                    Button("+1 增加 (Increment)")
                                        .Width(170.0f)
                                        .Height(36.0f)
                                        .Background(Color(40, 115, 210, 255))
                                        .Foreground(Color(255, 255, 255, 255))
                                        .CornerRadius(4.0f)
                                        .Command(vm->IncrementCommand),

                                    Button("歸零 (Reset)")
                                        .Width(130.0f)
                                        .Height(36.0f)
                                        .Background(Color(180, 50, 60, 255))
                                        .Foreground(Color(255, 255, 255, 255))
                                        .CornerRadius(4.0f)
                                        .Command(vm->ResetCommand)
                                }).Spacing(12.0f)
                            }).Spacing(10.0f)
                        )
                        .Row(0).Column(0)
                        .Background(Color(22, 25, 34, 255))
                        .BorderBrush(Color(42, 48, 65, 255))
                        .BorderThickness(1.0f)
                        .CornerRadius(8.0f)
                        .Padding(20.0f)
                        .Margin(16.0f),

                        // [Col 1] 右側卡片: IoC 依賴注入天氣服務
                        Border(
                            VStack({
                                Label("依賴反轉服務 (IoC Service)")
                                    .FontSize(16.0f)
                                    .FontWeight(text::FontWeight::SemiBold)
                                    .Foreground(Color(230, 240, 255, 255)),

                                Label("天氣資訊查詢結果:")
                                    .FontSize(13.0f)
                                    .Foreground(Color(160, 175, 195, 255)),

                                Label("點擊按鈕查詢天氣...")
                                    .FontSize(15.0f)
                                    .FontWeight(text::FontWeight::Medium)
                                    .Foreground(Color(240, 190, 80, 255))
                                    .Margin(0.0f, 6.0f)
                                    .BindText(vm->WeatherReport, [](const std::string& rep) {
                                        return rep;
                                    }),

                                Button("查詢天氣 (Fetch Weather)")
                                    .Width(200.0f)
                                    .Height(36.0f)
                                    .Background(Color(50, 130, 90, 255))
                                    .Foreground(Color(255, 255, 255, 255))
                                    .CornerRadius(4.0f)
                                    .Command(vm->FetchWeatherCommand)
                            }).Spacing(10.0f)
                        )
                        .Row(0).Column(1)
                        .Background(Color(22, 25, 34, 255))
                        .BorderBrush(Color(42, 48, 65, 255))
                        .BorderThickness(1.0f)
                        .CornerRadius(8.0f)
                        .Padding(20.0f)
                        .Margin(16.0f)
                    })
                    .Height(480.0f),

                    // Footer 狀態列 (自動雙向綁定 ViewModel)
                    Border(
                        Label("狀態: 系統準備就緒")
                            .FontSize(12.0f)
                            .FontFamily("Consolas")
                            .Foreground(Color(140, 165, 205, 255))
                            .BindText(vm->StatusMessage, [](const std::string& msg) {
                                return "狀態: " + msg;
                            })
                    )
                    .Background(Color(18, 20, 28, 255))
                    .BorderBrush(Color(36, 42, 58, 255))
                    .BorderThickness(0.0f, 1.0f, 0.0f, 0.0f)
                    .Padding(20.0f, 10.0f)
                })
            )
            .Background(Color(15, 17, 24, 255))
            .Get()
        );

        // 設定主 Grid 的欄列定義 (左右各佔 1 Star)
        if (auto* vstack = dynamic_cast<layout::StackPanel*>(GetContent().get())) {
            if (vstack->GetChildren().size() >= 2) {
                if (auto mainGrid = std::dynamic_pointer_cast<layout::Grid>(vstack->GetChildren()[1])) {
                    mainGrid->AddColumn(layout::GridLength::Star(1.0f));
                    mainGrid->AddColumn(layout::GridLength::Star(1.0f));
                    mainGrid->AddRow(layout::GridLength::Star(1.0f));
                }
            }
        }
    }

    void OnLoad() override {
        Form::OnLoad();
        compat::println("[Lifecycle] MainDashboardForm::OnLoad() 觸發！視窗即將呈現。");
    }

    void OnShown() override {
        Form::OnShown();
        compat::println("[Lifecycle] MainDashboardForm::OnShown() 觸發！視窗已呈現於螢幕。");
    }

    void OnFormClosing(FormClosingEventArgs& e) override {
        Form::OnFormClosing(e);
        compat::println("[Lifecycle] MainDashboardForm::OnFormClosing() 觸發！");
    }

    void OnFormClosed(FormClosedEventArgs& e) override {
        Form::OnFormClosed(e);
        compat::println("[Lifecycle] MainDashboardForm::OnFormClosed() 觸發！視窗已關閉。");
    }

private:
    std::shared_ptr<DashboardViewModel> m_vm;
};

} // namespace m7_demo

// =============================================================================
// 4. 程式進入點 (極速 Host 依賴注入與 Application::Run 啟動)
// =============================================================================

int main() {
    using namespace cppui;
    using namespace cppui::app;
    using namespace cppui::ioc;
    using namespace m7_demo;

    compat::println("==================================================");
    compat::println(" CPPUI Milestone 7: Application & MVVM Demo");
    compat::println("==================================================");

    // 建立現代化應用程式建構器 (Generic Host Builder)
    auto builder = Application::CreateBuilder();

    // 1. 註冊服務 (IoC Dependency Injection)
    builder.Services().AddSingleton<IWeatherService, WeatherService>();

    // 2. 註冊 ViewModel (Transient 工廠注入)
    builder.Services().AddTransient<DashboardViewModel>([](ioc::IServiceProvider& sp) {
        auto weatherSvc = sp.GetRequiredService<IWeatherService>();
        return std::make_shared<DashboardViewModel>(weatherSvc);
    });

    // 3. 註冊 Form 主表單
    builder.Services().AddTransient<MainDashboardForm>([](ioc::IServiceProvider& sp) {
        auto vm = sp.GetRequiredService<DashboardViewModel>();
        return std::make_shared<MainDashboardForm>(vm);
    });

    // 4. 構建 Host 並啟動應用程式 (單行啟動主表單！)
    auto app = builder.Build();
    return app.Run<MainDashboardForm>();
}
