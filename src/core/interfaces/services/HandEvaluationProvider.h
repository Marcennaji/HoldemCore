// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/HandEvaluationEngine.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Focused interface for classes that need hand evaluation capabilities
 * 
 * Replaces the monolithic ServiceContainer dependency with a focused
 * interface that only provides hand evaluation access, following ISP.
 */
class HandEvaluationProvider
{
  public:
    virtual ~HandEvaluationProvider() = default;
    virtual HandEvaluationEngine& getHandEvaluationEngine() = 0;
};

/**
 * @brief Simple implementation that wraps a HandEvaluationEngine instance
 */
class SimpleHandEvaluationProvider : public HandEvaluationProvider
{
  public:
    explicit SimpleHandEvaluationProvider(std::shared_ptr<HandEvaluationEngine> engine) : m_engine(engine) {}
    
    HandEvaluationEngine& getHandEvaluationEngine() override { return *m_engine; }
    
  private:
    std::shared_ptr<HandEvaluationEngine> m_engine;
};

} // namespace pkt::core