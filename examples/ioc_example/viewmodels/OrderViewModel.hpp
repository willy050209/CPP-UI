#pragma once

#include "ui/mvvm/ViewModelBase.hpp"
#include "ui/mvvm/RelayCommand.hpp"
#include "../services/IOrderService.hpp"
#include <memory>
#include <string>

class OrderViewModel : public cppui::mvvm::ViewModelBase {
public:
    explicit OrderViewModel(std::shared_ptr<IOrderService> orderService);
    ~OrderViewModel() override = default;

    COMPAT_NODISCARD compat::string_view GetSummaryText() const noexcept;
    COMPAT_NODISCARD compat::string_view GetLastActionText() const noexcept;

    std::shared_ptr<cppui::mvvm::RelayCommand> PlaceOrderCommand;

private:
    void RefreshSummary();

    std::shared_ptr<IOrderService> m_orderService;
    std::string m_summaryText;
    std::string m_lastActionText;
};
