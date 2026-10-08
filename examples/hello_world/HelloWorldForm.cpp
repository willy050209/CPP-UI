#include "HelloWorldForm.hpp"
#include "ui/dsl/Fluent.hpp"

using namespace cppui;
using namespace cppui::dsl;

HelloWorldForm::HelloWorldForm() {
    SetTitle("CPPUI - Hello World");
    SetFormSize(480, 260);

    SetContent(
        VStack({
            Label("Hello, CPPUI World!")
                .FontSize(24.0f)
                .Foreground(core::Colors::White),

            Label("極簡、高效、現代化的跨平台 C++ UI 框架。")
                .FontSize(14.0f)
                .Foreground(core::Color(160, 160, 160, 255)),

            Button("關閉視窗")
                .FontSize(14.0f)
                .Padding(core::Thickness(16.0f, 8.0f, 16.0f, 8.0f))
                .Background(core::Color(45, 110, 210, 255))
                .Foreground(core::Colors::White)
                .CornerRadius(6.0f)
                .OnClick([this]() {
                    Close();
                })
        })
        .Padding(core::Thickness(32.0f))
        .Spacing(16.0f)
    );
}
