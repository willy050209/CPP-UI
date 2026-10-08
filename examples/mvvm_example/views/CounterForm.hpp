#pragma once

#include "ui/app/Form.hpp"
#include "../viewmodels/CounterViewModel.hpp"
#include <memory>

namespace cppui {
namespace controls {
class TextBlock;
} // namespace controls
} // namespace cppui

class CounterForm : public cppui::app::Form {
public:
    explicit CounterForm(std::shared_ptr<CounterViewModel> viewModel);
    ~CounterForm() override = default;

private:
    std::shared_ptr<CounterViewModel> m_viewModel;
    std::shared_ptr<cppui::controls::TextBlock> m_countLabel;
    std::shared_ptr<cppui::controls::TextBlock> m_statusLabel;
};
