/*
 * File: IOrderService.hpp
 * Purpose: Declares the IOrderService interface for order management business operations.
 */

#pragma once

#include <cstdint>
#include <vector>
#include "compat/Compat.hpp"
#include "../models/Order.hpp"

/// <summary>
/// 訂單業務邏輯服務介面
/// </summary>
class IOrderService {
public:
    virtual ~IOrderService() = default;

    /// <summary>
    /// 取得所有訂單清單。
    /// </summary>
    /// <returns>訂單 vector 清單。</returns>
    COMPAT_NODISCARD virtual std::vector<Order> GetOrders() const = 0;

    /// <summary>
    /// 取得目前訂單總數。
    /// </summary>
    /// <returns>訂單總筆數。</returns>
    COMPAT_NODISCARD virtual int32_t GetOrderCount() const = 0;

    /// <summary>
    /// 取得目前所有訂單總金額。
    /// </summary>
    /// <returns>訂單總金額數值。</returns>
    COMPAT_NODISCARD virtual double GetTotalRevenue() const = 0;

    /// <summary>
    /// 建立新訂單。
    /// </summary>
    /// <param name="customer">客戶名稱字串視圖。</param>
    /// <param name="amount">訂單金額。</param>
    virtual void CreateOrder(compat::string_view customer, double amount) = 0;
};
