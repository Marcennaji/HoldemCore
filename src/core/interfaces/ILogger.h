// PokerTraining — Texas Hold'em simulator
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
    Verbose,
    Debug
};

class ILogger
{
  public:
    virtual ~ILogger() = default;
    virtual void setLogLevel(LogLevel level) = 0;
    virtual void error(const std::string& msg) = 0;
    virtual void info(const std::string& msg) = 0;
    virtual void verbose(const std::string& msg) = 0;

  protected:
    LogLevel myLogLevel{LogLevel::Info};
};

} // namespace pkt::core
