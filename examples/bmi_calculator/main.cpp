/*
 * File: main.cpp
 * Purpose: Entry point for the BMI Calculator sample application.
 */

#include "ui/app/Application.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "BmiForm.hpp"

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (compat::string_view(argv[i]) == "--capture" && i + 1 < argc) {
            bmi_calculator::BmiForm form(true);
            auto bmp = form.CaptureToBitmap();
            bmp->SaveAsBmp(argv[i + 1]);
            return 0;
        }
    }
    return cppui::app::Application::Run<bmi_calculator::BmiForm>();
}
