/*
 * File: main.cpp
 * Purpose: Entry point for the cardioid and linear gradient drawing example.
 */

#include "ui/app/Application.hpp"
#include "compat/compat.hpp"
#include "CardioidForm.hpp"

int main() {
    using namespace cppui::app;
    using namespace drawing_cardioid_example;

    compat::println("==================================================");
    compat::println(" CPPUI Example: GDI+ Graphics (Cardioid & Gradient)");
    compat::println("==================================================");

    return Application::Run<CardioidForm>();
}
