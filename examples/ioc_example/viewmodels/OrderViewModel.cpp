/*
 * File: OrderViewModel.cpp
 * Purpose: Implements OrderViewModel presentation logic and command handlers.
 */

#include "OrderViewModel.hpp"
#include <cstdint>

OrderViewModel::OrderViewModel(std::shared_ptr<IOrderService> orderService)
    : m_orderService(orderService) {

    PlaceOrderCommand = std::make_shared<cppui::mvvm::RelayCommand>([this]() {
        if (!m_orderService) return;
        static int32_t customerIdx = 1;
        std::string customer = "企業客戶 #" + std::to_string(customerIdx++);
        double amount = 1200.0 + (customerIdx * 350.0);

        m_orderService->CreateOrder(customer, amount);

        std::string action = "最新操作：成功建立 " + customer + " 訂單 ($" + std::to_string(static_cast<int32_t>(amount)) + ")";
        SetProperty(m_lastActionText, action, "LastActionText");

        RefreshSummary();
    });

    RefreshSummary();
    m_lastActionText = "系統就緒，點擊下方按鈕以 IoC 注入之服務建立新訂單。";
}

compat::string_view OrderViewModel::GetSummaryText() const noexcept {
    return m_summaryText;
}

compat::string_view OrderViewModel::GetLastActionText() const noexcept {
    return m_lastActionText;
}

void OrderViewModel::RefreshSummary() {
    if (!m_orderService) return;
    int32_t count = m_orderService->GetOrderCount();
    double revenue = m_orderService->GetTotalRevenue();

    std::string summary = "累計訂單總數: " + std::to_string(count) +
                          " 筆  |  總營收金額: $" + std::to_string(static_cast<int32_t>(revenue));
    SetProperty(m_summaryText, summary, "SummaryText");
}
