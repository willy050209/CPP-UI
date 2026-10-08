#pragma once

#include "IOrderService.hpp"
#include "ILogger.hpp"
#include <memory>
#include <vector>

class OrderService : public IOrderService {
public:
    explicit OrderService(std::shared_ptr<ILogger> logger);
    ~OrderService() override = default;

    COMPAT_NODISCARD std::vector<Order> GetOrders() const override;
    COMPAT_NODISCARD int GetOrderCount() const override;
    COMPAT_NODISCARD double GetTotalRevenue() const override;

    void CreateOrder(compat::string_view customer, double amount) override;

private:
    std::shared_ptr<ILogger> m_logger;
    std::vector<Order> m_orders;
    int m_nextId = 1001;
};
