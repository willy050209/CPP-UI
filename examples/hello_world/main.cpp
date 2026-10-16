/*
 * File: main.cpp
 * Purpose: Entry point for the HelloWorld sample demonstrating Application::Run<Form>().
 */

#include "ui/app/Application.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "HelloWorldForm.hpp"

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (compat::string_view(argv[i]) == "--capture" && i + 1 < argc) {
            HelloWorldForm form;
            auto bmp = form.CaptureToBitmap();
            bmp->Save(argv[i + 1]);
            return 0;
        }
    }
    return cppui::app::Application::Run<HelloWorldForm>();
}
