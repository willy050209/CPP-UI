/*
 * File: Exceptions.hpp
 * Purpose: Defines core domain exception types for CPPUI runtime errors,
 *          invalid operations, and unsupported capabilities.
 */

#pragma once

#include <stdexcept>
#include <string>
#include "compat/Compat.hpp"

namespace cppui {
namespace core {

/// <summary>
/// Base class for all domain-level exceptions in CPPUI.
/// Inherits from std::runtime_error.
/// </summary>
class Exception : public std::runtime_error {
public:
    /// <summary>
    /// Constructs a new Exception with a string_view error message.
    /// </summary>
    /// <param name="message">The descriptive error message view.</param>
    explicit Exception(compat::string_view message)
        : std::runtime_error(std::string(message)) {}
};

/// <summary>
/// Exception thrown when an operation is invalid for the object's current state.
/// </summary>
class InvalidOperationException : public Exception {
public:
    /// <summary>
    /// Constructs a new InvalidOperationException with a string_view message.
    /// </summary>
    /// <param name="message">The descriptive error message view.</param>
    explicit InvalidOperationException(compat::string_view message)
        : Exception(message) {}
};

/// <summary>
/// Exception thrown when an invoked method or feature is not supported.
/// </summary>
class NotSupportedException : public Exception {
public:
    /// <summary>
    /// Constructs a new NotSupportedException with a string_view message.
    /// </summary>
    /// <param name="message">The descriptive error message view.</param>
    explicit NotSupportedException(compat::string_view message)
        : Exception(message) {}
};

} // namespace core
} // namespace cppui
