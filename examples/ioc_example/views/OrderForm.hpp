#pragma once

#include "ui/app/Form.hpp"
#include "../viewmodels/OrderViewModel.hpp"
#include <memory>

namespace cppui {
namespace controls {
class TextBlock;
} // namespace controls
} // namespace cppui

class OrderForm : public cppui::app::Form {
public:
    explicit OrderForm(std::shared_ptr<OrderViewModel> viewModel);
    ~OrderForm() override = default;

private:
    std::shared_ptr<OrderViewModel> m_viewModel;
    std::shared_ptr<cppui::controls::TextBlock> m_summaryLabel;
    std::shared_ptr<cppui::controls::TextBlock> m_actionLabel;
};
