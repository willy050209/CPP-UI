#include "ConsoleLogger.hpp"

void ConsoleLogger::Log(compat::string_view message) {
    compat::println("[ConsoleLogger] {}", message);
}
