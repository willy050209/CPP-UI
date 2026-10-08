/*
 * File: main.cpp
 * Purpose: Entry point for the HelloWorld sample demonstrating Application::Run<Form>().
 */

#include "ui/app/Application.hpp"
#include "HelloWorldForm.hpp"

int main() {
    return cppui::app::Application::Run<HelloWorldForm>();
}
