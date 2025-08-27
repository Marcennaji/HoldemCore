// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/ILogger.h"

#include <iostream>
#include <string>

namespace pkt::infra
{

class ConsoleLogger : public pkt::core::ILogger
{
  public:
    explicit ConsoleLogger();
    void setLogLevel(pkt::core::LogLevel level) override { myLogLevel = level; }
    void error(const std::string& msg) override;
    void info(const std::string& msg) override;
    void verbose(const std::string& msg) override;
    void debug(const std::string& msg) override;
};
} // namespace pkt::infra
