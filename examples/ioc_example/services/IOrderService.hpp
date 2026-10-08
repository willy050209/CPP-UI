#pragma once

#include "compat/compat.hpp"
#include "../models/Order.hpp"
#include <vector>

class IOrderService {
public:
    virtual ~IOrderService() = default;

    COMPAT_NODISCARD virtual std::vector<Order> GetOrders() const = 0;
    COMPAT_NODISCARD virtual int GetOrderCount() const = 0;
    COMPAT_NODISCARD virtual double GetTotalRevenue() const = 0;

    virtual void CreateOrder(compat::string_view customer, double amount) = 0;
};
