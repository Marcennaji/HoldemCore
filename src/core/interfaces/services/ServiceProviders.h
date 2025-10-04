// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "LoggerProvider.h"
#include "RandomizerProvider.h"
#include "HandEvaluationProvider.h"
#include "core/services/ServiceContainer.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Adapter that creates focused service providers from ServiceContainer
 * 
 * This adapter allows gradual migration from ServiceContainer to focused interfaces.
 * It implements all provider interfaces and delegates to the underlying ServiceContainer.
 */
class ServiceContainerAdapter : public LoggerProvider, public RandomizerProvider, public HandEvaluationProvider
{
  public:
    explicit ServiceContainerAdapter(std::shared_ptr<ServiceContainer> services) : m_services(services) {}
    
    Logger& getLogger() override { return m_services->logger(); }
    Randomizer& getRandomizer() override { return m_services->randomizer(); }
    HandEvaluationEngine& getHandEvaluationEngine() override { return m_services->handEvaluationEngine(); }
    
  private:
    std::shared_ptr<ServiceContainer> m_services;
};

} // namespace pkt::core