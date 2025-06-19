// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
#include <core/interfaces/ILogger.h>

namespace pkt::core
{

class NullLogger : public core::ILogger
{
  public:
    virtual void error(const std::string& msg) override {}
    virtual void info(const std::string& msg) override {}
    virtual void verbose(const std::string& msg, int level = 1) override {}
};

} // namespace pkt::core
