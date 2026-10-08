#include "OrderForm.hpp"
#include "ui/dsl/Fluent.hpp"

using namespace cppui;
using namespace cppui::dsl;

OrderForm::OrderForm(std::shared_ptr<OrderViewModel> viewModel)
    : m_viewModel(viewModel) {

    SetTitle("CPPUI - Enterprise IoC & Dependency Injection Example");
    SetFormSize(620, 380);
    SetDataContext(m_viewModel);

    m_summaryLabel = Label(m_viewModel ? m_viewModel->GetSummaryText() : "")
        .FontSize(18.0f)
        .Foreground(core::Color(100, 220, 255, 255));

    m_actionLabel = Label(m_viewModel ? m_viewModel->GetLastActionText() : "")
        .FontSize(14.0f)
        .Foreground(core::Color(200, 200, 200, 255));

    // 監聽 ViewModel 變更以自動重繪
    if (m_viewModel) {
        m_viewModel->AddPropertyChangedHandler([this](compat::string_view prop) {
            if (prop == "SummaryText" && m_summaryLabel && m_viewModel) {
                m_summaryLabel->SetText(m_viewModel->GetSummaryText());
            } else if (prop == "LastActionText" && m_actionLabel && m_viewModel) {
                m_actionLabel->SetText(m_viewModel->GetLastActionText());
            }
            InvalidateForm();
        });
    }

    SetContent(
        VStack({
            Label("依賴注入與服務容器架構展示 (IoC Container)")
                .FontSize(20.0f)
                .Foreground(core::Colors::White),

            Border(
                VStack({
                    m_summaryLabel,
                    m_actionLabel
                })
                .Spacing(10.0f)
            )
            .Padding(core::Thickness(18.0f))
            .Background(core::Color(30, 34, 46, 255))
            .BorderBrush(core::Color(64, 76, 102, 255))
            .BorderThickness(1.0f)
            .CornerRadius(8.0f),

            HStack({
                Button("＋ 透過 IoC 服務建立新訂單")
                    .FontSize(15.0f)
                    .Padding(core::Thickness(20.0f, 10.0f, 20.0f, 10.0f))
                    .Background(core::Color(13, 110, 253, 255))
                    .Foreground(core::Colors::White)
                    .CornerRadius(6.0f)
                    .Command(m_viewModel ? m_viewModel->PlaceOrderCommand : nullptr),

                Button("關閉系統")
                    .FontSize(15.0f)
                    .Padding(core::Thickness(20.0f, 10.0f, 20.0f, 10.0f))
                    .Background(core::Color(108, 117, 125, 255))
                    .Foreground(core::Colors::White)
                    .CornerRadius(6.0f)
                    .OnClick([this]() {
                        Close();
                    })
            })
            .Spacing(12.0f)
        })
        .Padding(core::Thickness(28.0f))
        .Spacing(20.0f)
    );
}
