// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
#include <string>

namespace pkt::core
{

enum class LogLevel
{
    Quiet = 0,
    Info,
    DecisionMaking,
    Verbose,
    Debug
};

/**
 * @brief Abstract interface for logging functionality across the application.
 *
 * Provides a unified logging interface with multiple log levels (quiet, info,
 * verbose, debug) allowing different logging implementations to be plugged
 * into the poker engine system.
 */
class Logger
{
  public:
    virtual ~Logger() = default;
    virtual void setLogLevel(LogLevel level) = 0;
    virtual void error(const std::string& msg) = 0;
    virtual void info(const std::string& msg) = 0;
    virtual void decisionMaking(const std::string& msg) = 0;
    virtual void verbose(const std::string& msg) = 0;
    virtual void debug(const std::string& msg) = 0;

  protected:
    LogLevel m_logLevel{LogLevel::Info};
};

} // namespace pkt::core
