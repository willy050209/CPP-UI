/*
 * File: main.cpp
 * Purpose: Entry point for the MVVM counter sample demonstrating Application::Run<CounterForm>(viewModel).
 */

#include "ui/app/Application.hpp"
#include "viewmodels/CounterViewModel.hpp"
#include "views/CounterForm.hpp"
#include <memory>

int main() {
    auto viewModel = std::make_shared<CounterViewModel>();
    return cppui::app::Application::Run<CounterForm>(viewModel);
}
