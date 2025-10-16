// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <exception>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>

namespace pkt::core::utils
{

/**
 * @brief Throws a standard exception with source location information.
 *
 * This utility automatically captures and includes file name, line number,
 * and function name in the exception message, making debugging much easier.
 *
 * Usage:
 *   throwWithLocation<std::runtime_error>("Something went wrong");
 *   throwWithLocation<std::invalid_argument>("Invalid parameter value");
 *   throwWithLocation("Default uses runtime_error");
 *
 * @tparam ExceptionType The type of exception to throw (must derive from std::exception)
 * @param message The error message to include
 * @param location Source location (automatically captured by default)
 * @throws ExceptionType with formatted message including source location
 */
template <typename ExceptionType = std::runtime_error>
[[noreturn]] void throwWithLocation(const std::string& message,
                                    const std::source_location& location = std::source_location::current())
{
    std::ostringstream oss;
    oss << location.file_name() << ":" << location.line() << " in " << location.function_name() << " - " << message;
    throw ExceptionType(oss.str());
}

/**
 * @brief Convenience wrapper for throwing std::runtime_error with location.
 *
 * @param message The error message
 * @param location Source location (automatically captured)
 */
[[noreturn]] inline void throwRuntimeError(const std::string& message,
                                           const std::source_location& location = std::source_location::current())
{
    throwWithLocation<std::runtime_error>(message, location);
}

/**
 * @brief Convenience wrapper for throwing std::invalid_argument with location.
 *
 * @param message The error message
 * @param location Source location (automatically captured)
 */
[[noreturn]] inline void throwInvalidArgument(const std::string& message,
                                              const std::source_location& location = std::source_location::current())
{
    throwWithLocation<std::invalid_argument>(message, location);
}

/**
 * @brief Convenience wrapper for throwing std::logic_error with location.
 *
 * @param message The error message
 * @param location Source location (automatically captured)
 */
[[noreturn]] inline void throwLogicError(const std::string& message,
                                         const std::source_location& location = std::source_location::current())
{
    throwWithLocation<std::logic_error>(message, location);
}

} // namespace pkt::core::utils
