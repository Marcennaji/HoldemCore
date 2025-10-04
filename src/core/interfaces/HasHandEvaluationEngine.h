// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/HandEvaluationEngine.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Focused interface for components that need hand evaluation capabilities
 * 
 * This replaces the heavy ServiceContainer dependency with a focused interface.
 * Follows Interface Segregation Principle (ISP) - clients should not be forced
 * to depend on interfaces they do not use.
 */
class HasHandEvaluationEngine
{
  public:
    virtual ~HasHandEvaluationEngine() = default;
    virtual HandEvaluationEngine& handEvaluationEngine() = 0;
};

/**
 * @brief Simple wrapper around a HandEvaluationEngine instance
 * 
 * Follows Single Responsibility Principle - only responsible for providing
 * hand evaluation engine access.
 */
class HandEvaluationEngineService : public HasHandEvaluationEngine
{
  public:
    explicit HandEvaluationEngineService(std::shared_ptr<HandEvaluationEngine> engine) : m_engine(engine) {}
    
    HandEvaluationEngine& handEvaluationEngine() override { return *m_engine; }
    
  private:
    std::shared_ptr<HandEvaluationEngine> m_engine;
};

} // namespace pkt::core