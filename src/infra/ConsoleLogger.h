// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Logger.h"

#include <iostream>
#include <string>

namespace pkt::infra
{

/**
 * @brief Logger implementation that outputs messages to the console.
 * 
 * This logger provides console-based logging functionality for debugging
 * and monitoring the poker engine. It supports different log levels and
 * outputs formatted messages to standard output streams.
 */
class ConsoleLogger : public pkt::core::Logger
{
  public:
    explicit ConsoleLogger();
    void setLogLevel(pkt::core::LogLevel level) override { m_logLevel = level; }
    void error(const std::string& msg) override;
    void info(const std::string& msg) override;
    void verbose(const std::string& msg) override;
    void debug(const std::string& msg) override;
};
} // namespace pkt::infra
