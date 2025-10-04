// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Logger.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Simple interface for components that only need logging
 * 
 * This replaces the heavy ServiceContainer dependency with a focused interface.
 * Used as a proof-of-concept for eliminating ensureServicesInitialized() pattern.
 */
class HasLogger
{
  public:
    virtual ~HasLogger() = default;
    virtual Logger& logger() = 0;
};

/**
 * @brief Simple wrapper around a Logger instance
 */
class LoggerService : public HasLogger
{
  public:
    explicit LoggerService(std::shared_ptr<Logger> logger) : m_logger(logger) {}
    
    Logger& logger() override { return *m_logger; }
    
  private:
    std::shared_ptr<Logger> m_logger;
};

} // namespace pkt::core