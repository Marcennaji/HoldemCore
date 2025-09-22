// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/ILogger.h"

namespace pkt::core
{

class NullLogger : public ILogger
{
  public:
    virtual void setLogLevel(LogLevel level) override {}
    virtual void error(const std::string& msg) override {}
    virtual void info(const std::string& msg) override {}
    virtual void verbose(const std::string& msg) override {}
    virtual void debug(const std::string& msg) override {}
};

} // namespace pkt::core
