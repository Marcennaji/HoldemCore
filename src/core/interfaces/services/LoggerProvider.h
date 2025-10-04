// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Logger.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Focused interface for classes that need logging capabilities
 * 
 * Replaces the monolithic ServiceContainer dependency with a focused
 * interface that only provides logging access, following ISP.
 */
class LoggerProvider
{
  public:
    virtual ~LoggerProvider() = default;
    virtual Logger& getLogger() = 0;
};

/**
 * @brief Simple implementation that wraps a Logger instance
 */
class SimpleLoggerProvider : public LoggerProvider
{
  public:
    explicit SimpleLoggerProvider(std::shared_ptr<Logger> logger) : m_logger(logger) {}
    
    Logger& getLogger() override { return *m_logger; }
    
  private:
    std::shared_ptr<Logger> m_logger;
};

} // namespace pkt::core