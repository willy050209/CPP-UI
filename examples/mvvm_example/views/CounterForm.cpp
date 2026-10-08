#include "CounterForm.hpp"
#include "ui/dsl/Fluent.hpp"

using namespace cppui;
using namespace cppui::dsl;

CounterForm::CounterForm(std::shared_ptr<CounterViewModel> viewModel)
    : m_viewModel(viewModel) {

    SetTitle("CPPUI - MVVM Architecture Example");
    SetFormSize(560, 360);
    SetDataContext(m_viewModel);

    m_countLabel = Label(m_viewModel ? m_viewModel->GetCountText() : "計數數值: 0")
        .FontSize(28.0f)
        .Foreground(core::Color(100, 200, 255, 255));

    m_statusLabel = Label(m_viewModel ? m_viewModel->GetStatusMessage() : "")
        .FontSize(14.0f)
        .Foreground(core::Color(180, 180, 180, 255));

    // 訂閱 ViewModel 屬性通知
    if (m_viewModel) {
        m_viewModel->AddPropertyChangedHandler([this](compat::string_view prop) {
            if (prop == "CountText" && m_countLabel && m_viewModel) {
                m_countLabel->SetText(m_viewModel->GetCountText());
            } else if (prop == "StatusMessage" && m_statusLabel && m_viewModel) {
                m_statusLabel->SetText(m_viewModel->GetStatusMessage());
            }
            InvalidateForm();
        });
    }

    SetContent(
        VStack({
            Label("MVVM 雙向狀態驅動展示")
                .FontSize(20.0f)
                .Foreground(core::Colors::White),

            Border(
                VStack({
                    m_countLabel,
                    m_statusLabel
                })
                .Spacing(8.0f)
            )
            .Padding(core::Thickness(16.0f))
            .Background(core::Color(36, 40, 52, 255))
            .BorderBrush(core::Color(60, 68, 88, 255))
            .BorderThickness(1.0f)
            .CornerRadius(8.0f),

            HStack({
                Button("＋ 增加")
                    .FontSize(15.0f)
                    .Padding(core::Thickness(20.0f, 10.0f, 20.0f, 10.0f))
                    .Background(core::Color(40, 167, 69, 255))
                    .Foreground(core::Colors::White)
                    .CornerRadius(6.0f)
                    .Command(m_viewModel ? m_viewModel->IncrementCommand : nullptr),

                Button("－ 減少")
                    .FontSize(15.0f)
                    .Padding(core::Thickness(20.0f, 10.0f, 20.0f, 10.0f))
                    .Background(core::Color(220, 53, 69, 255))
                    .Foreground(core::Colors::White)
                    .CornerRadius(6.0f)
                    .Command(m_viewModel ? m_viewModel->DecrementCommand : nullptr),

                Button("重設歸零")
                    .FontSize(15.0f)
                    .Padding(core::Thickness(20.0f, 10.0f, 20.0f, 10.0f))
                    .Background(core::Color(108, 117, 125, 255))
                    .Foreground(core::Colors::White)
                    .CornerRadius(6.0f)
                    .Command(m_viewModel ? m_viewModel->ResetCommand : nullptr)
            })
            .Spacing(12.0f)
        })
        .Padding(core::Thickness(28.0f))
        .Spacing(20.0f)
    );
}
