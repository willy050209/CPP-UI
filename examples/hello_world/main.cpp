#include "ui/app/Application.hpp"
#include "HelloWorldForm.hpp"

int main() {
    return cppui::app::Application::Run(new HelloWorldForm());
}
