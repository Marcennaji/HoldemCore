// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Logger.h"

namespace pkt::infra
{

/**
 * @brief Null object pattern implementation of Logger that discards all messages.
 * 
 * This logger implementation provides a no-op logger that silently discards
 * all log messages. Useful for testing or when logging is not desired
 * without having to check for null pointers throughout the code.
 */
class NullLogger : public pkt::core::Logger
{
  public:
    virtual void setLogLevel(pkt::core::LogLevel level) override {}
    virtual void error(const std::string& msg) override {}
    virtual void info(const std::string& msg) override {}
    virtual void verbose(const std::string& msg) override {}
    virtual void debug(const std::string& msg) override {}
};

} // namespace pkt::infra
