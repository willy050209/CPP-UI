/*
 * File: main.cpp
 * Purpose: Entry point for the asynchronous events demo demonstrating Application::Post and Dispatcher.
 */

#include "ui/app/Application.hpp"
#include "compat/compat.hpp"
#include "AsyncEventForm.hpp"

int main() {
    using namespace cppui::app;
    using namespace async_events_example;

    compat::println("==================================================");
    compat::println(" CPPUI Example: Asynchronous Events & Dispatcher  ");
    compat::println("==================================================");

    return Application::Run<AsyncEventForm>();
}
