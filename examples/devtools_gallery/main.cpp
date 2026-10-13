/**
 * @file main.cpp
 * @brief 視覺化診斷工具與 Visual Tree Inspector (DevTools) 展示範例
 */

#include "compat/compat.hpp"
#include "ui/app/AppBuilder.hpp"
#include "ui/app/Form.hpp"
#include "ui/controls/Button.hpp"
#include "ui/controls/Label.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/diagnostics/VisualTreeInspector.hpp"
#include "ui/diagnostics/DirtyRectTracker.hpp"

using namespace cppui;
using namespace cppui::controls;
using namespace cppui::layout;

class DevToolsDemoForm : public app::Form {
public:
    DevToolsDemoForm() {
        SetTitle("CppUI DevTools Gallery [Press F12 to inspect]");
        SetWidth(700.0f);
        SetHeight(500.0f);

        auto rootPanel = std::make_shared<StackPanel>();
        rootPanel->SetOrientation(Orientation::Vertical);
        rootPanel->SetMargin(core::Thickness(20.0f));

        auto lblTitle = std::make_shared<Label>("Visual Tree Inspector & Diagnostics Demo");
        rootPanel->AddChild(lblTitle);

        auto lblHelp = std::make_shared<Label>("Press F12 to toggle DevTools overlay. Click buttons below to test Dirty Rect flashing.");
        lblHelp->SetMargin(core::Thickness(0.0f, 6.0f, 0.0f, 16.0f));
        rootPanel->AddChild(lblHelp);

        auto btnToggleInspector = std::make_shared<Button>("Toggle DevTools Inspector (F12)");
        btnToggleInspector->SetWidth(280.0f);
        btnToggleInspector->SetHeight(32.0f);
        btnToggleInspector->SetMargin(core::Thickness(0.0f, 0.0f, 0.0f, 10.0f));
        btnToggleInspector->OnClick([this]() {
            diagnostics::VisualTreeInspector::Get().Toggle();
            InvalidateVisual();
        });
        rootPanel->AddChild(btnToggleInspector);

        auto btnPicker = std::make_shared<Button>("Enable Element Picker (Eyedropper)");
        btnPicker->SetWidth(280.0f);
        btnPicker->SetHeight(32.0f);
        btnPicker->SetMargin(core::Thickness(0.0f, 0.0f, 0.0f, 10.0f));
        btnPicker->OnClick([this]() {
            diagnostics::VisualTreeInspector::Get().EnablePickerMode(true);
            InvalidateVisual();
        });
        rootPanel->AddChild(btnPicker);

        auto txtSample = std::make_shared<TextBox>();
        txtSample->SetText("Editable Text for Inspection");
        txtSample->SetWidth(280.0f);
        txtSample->SetHeight(30.0f);
        rootPanel->AddChild(txtSample);

        SetContent(rootPanel);
    }
};

int main() {
    diagnostics::DiagnosticsOptions diagOpts;
    diagOpts.enableDirtyRectOverlay = true;
    diagOpts.showFpsCounter = true;
    diagOpts.enableVisualTreeInspector = true;

    app::AppBuilder builder;
    builder.WithDiagnostics(diagOpts);
    auto app = builder.Build();
    return app.Run<DevToolsDemoForm>();
}
