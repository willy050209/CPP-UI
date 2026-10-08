/*
 * File: OrderService.hpp
 * Purpose: Declares OrderService implementing IOrderService for order lifecycle processing.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "IOrderService.hpp"
#include "ILogger.hpp"

/// <summary>
/// 訂單業務邏輯服務實作
/// </summary>
class OrderService : public IOrderService {
public:
    /// <summary>
    /// 建構 OrderService 實例。
    /// </summary>
    /// <param name="logger">記錄器服務智慧指標。</param>
    explicit OrderService(std::shared_ptr<ILogger> logger);
    ~OrderService() override = default;

    COMPAT_NODISCARD std::vector<Order> GetOrders() const override;
    COMPAT_NODISCARD int32_t GetOrderCount() const override;
    COMPAT_NODISCARD double GetTotalRevenue() const override;

    void CreateOrder(compat::string_view customer, double amount) override;

private:
    std::shared_ptr<ILogger> m_logger;
    std::vector<Order> m_orders;
    int32_t m_nextId = 1001;
};
