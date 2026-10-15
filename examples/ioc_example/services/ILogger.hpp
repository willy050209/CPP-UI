#pragma once

#include "compat/Compat.hpp"

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void Log(compat::string_view message) = 0;
};
