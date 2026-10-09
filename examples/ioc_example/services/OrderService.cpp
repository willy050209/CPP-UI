/*
 * File: OrderService.cpp
 * Purpose: Implements OrderService order registration, aggregation, and logging.
 */

#include "OrderService.hpp"
#include <cstdint>
#include <numeric>

OrderService::OrderService(std::shared_ptr<ILogger> logger)
    : m_logger(logger) {
    if (m_logger) {
        m_logger->Log("OrderService 實例建立 (由 IoC 容器瞬態工廠建構)");
    }
}

std::vector<Order> OrderService::GetOrders() const {
    return m_orders;
}

int32_t OrderService::GetOrderCount() const {
    return static_cast<int32_t>(m_orders.size());
}

double OrderService::GetTotalRevenue() const {
    double total = 0.0;
    for (const auto& ord : m_orders) {
        total += ord.amount;
    }
    return total;
}

void OrderService::CreateOrder(compat::string_view customer, double amount) {
    Order ord;
    ord.id = m_nextId++;
    ord.customerName = std::string(customer.data(), customer.size());
    ord.amount = amount;
    ord.status = "已確認付款";

    m_orders.push_back(ord);

    if (m_logger) {
        m_logger->Log("新訂單建立成功：#" + compat::to_string(ord.id) + " - 客戶: " + ord.customerName);
    }
}
