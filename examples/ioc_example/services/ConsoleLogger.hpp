#pragma once

#include "ILogger.hpp"

class ConsoleLogger : public ILogger {
public:
    ConsoleLogger() = default;
    ~ConsoleLogger() override = default;

    void Log(compat::string_view message) override;
};
