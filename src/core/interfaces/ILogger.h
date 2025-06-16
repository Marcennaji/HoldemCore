// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
#include <string>

namespace pkt::core
{

class ILogger
{
  public:
    virtual ~ILogger() = default;
    virtual void error(const std::string& msg) = 0;
    virtual void info(const std::string& msg) = 0;
    virtual void verbose(const std::string& msg, int level = 1) = 0;
};

} // namespace pkt::core
