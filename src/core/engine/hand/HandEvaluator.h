// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once
#include <memory>
#include "core/interfaces/HandEvaluationEngine.h"

namespace pkt::core
{

/**
 * @brief Evaluates poker hand strength using pluggable evaluation engines.
 * 
 * Provides a unified interface for evaluating 7-card poker hands (5 community
 * cards + 2 hole cards) using different evaluation algorithms through the
 * HandEvaluationEngine interface.
 */

class HandEvaluator
{
  public:

    /// Evaluates the strength of a 7-card poker hand using the provided evaluation engine.
    /// \param hand A string representing the 7-card poker hand (5 board + 2 hole).
    /// \param engine The hand evaluation engine to use.
    /// \return A hand strength score where higher means better.
    static unsigned int evaluateHand(const char* hand, std::shared_ptr<pkt::core::HandEvaluationEngine> engine);
};

} // namespace pkt::core