#pragma once

#include <string>

struct Order {
    int id = 0;
    std::string customerName;
    double amount = 0.0;
    std::string status;
};
