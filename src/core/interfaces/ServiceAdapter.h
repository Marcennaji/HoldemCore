// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/HasLogger.h"
#include "core/services/ServiceContainer.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Helper to create focused service dependencies from ServiceContainer
 * 
 * This adapter makes it easy to migrate from ServiceContainer to focused dependencies
 * without changing all the creation code at once.
 */
class ServiceAdapter
{
  public:
    explicit ServiceAdapter(std::shared_ptr<ServiceContainer> services) : m_services(services) {}
    
    std::shared_ptr<HasLogger> createLoggerService() const {
        return std::make_shared<LoggerFromServiceContainer>(m_services);
    }
    
  private:
    class LoggerFromServiceContainer : public HasLogger {
      public:
        explicit LoggerFromServiceContainer(std::shared_ptr<ServiceContainer> services) : m_services(services) {}
        Logger& logger() override { return m_services->logger(); }
      private:
        std::shared_ptr<ServiceContainer> m_services;
    };
    
    std::shared_ptr<ServiceContainer> m_services;
};

} // namespace pkt::core