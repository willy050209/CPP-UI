/*
 * File: Order.hpp
 * Purpose: Declares the Order data entity model for the IoC sample application.
 */

#pragma once

#include <cstdint>
#include <string>

/// <summary>
/// 訂單資料實體模型
/// </summary>
struct Order {
    int32_t id = 0;
    std::string customerName;
    double amount = 0.0;
    std::string status;
};
