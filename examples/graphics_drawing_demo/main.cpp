/*
 * File: main.cpp
 * Purpose: Entry point for the 2D Graphics Drawing and ImageFilters Grayscale demo.
 */

#include "ui/app/Application.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "DrawingDemoForm.hpp"

int main(int argc, char* argv[]) {
    bool startGrayscale = false;
    compat::string_view capturePath = "";

    for (int i = 1; i < argc; ++i) {
        if (compat::string_view(argv[i]) == "--grayscale") {
            startGrayscale = true;
        } else if (compat::string_view(argv[i]) == "--capture" && i + 1 < argc) {
            capturePath = argv[i + 1];
            ++i;
        }
    }

    if (!capturePath.empty()) {
        graphics_drawing_demo::DrawingDemoForm form(startGrayscale);
        auto bmp = form.CaptureToBitmap();
        bmp->SaveAsBmp(capturePath);
        return 0;
    }

    return cppui::app::Application::Run<graphics_drawing_demo::DrawingDemoForm>(startGrayscale);
}
