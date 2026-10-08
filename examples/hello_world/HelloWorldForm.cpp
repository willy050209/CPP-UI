/*
 * File: HelloWorldForm.cpp
 * Purpose: Implements HelloWorldForm layout definition using fluent UI DSL.
 */

#include "HelloWorldForm.hpp"
#include "ui/dsl/Fluent.hpp"

HelloWorldForm::HelloWorldForm() {
    using namespace cppui::dsl;

    SetTitle("CPPUI - Hello World");
    SetFormSize(480, 260);

    SetContent(
        VStack({
            Label("Hello, CPPUI World!")
                .FontSize(24.0f)
                .Foreground(cppui::core::Colors::White),

            Label("極簡、高效、現代化的跨平台 C++ UI 框架。")
                .FontSize(14.0f)
                .Foreground(cppui::core::Color(160, 160, 160, 255)),

            Button("關閉視窗")
                .FontSize(14.0f)
                .Padding(cppui::core::Thickness(16.0f, 8.0f, 16.0f, 8.0f))
                .Background(cppui::core::Color(45, 110, 210, 255))
                .Foreground(cppui::core::Colors::White)
                .CornerRadius(6.0f)
                .OnClick([this]() {
                    Close();
                })
        })
        .Padding(cppui::core::Thickness(32.0f))
        .Spacing(16.0f)
    );
}
