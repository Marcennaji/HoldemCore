#pragma once
#include <memory>
#include "core/interfaces/IHandEvaluationEngine.h"

namespace pkt::core
{

/// Poker hand evaluation utility class using pluggable evaluation engines
class HandEvaluator
{
  public:
    /// Evaluates the strength of a 7-card poker hand.
    /// \param hand A string representing the 7-card poker hand (5 board + 2 hole).
    /// \return A hand strength score where higher means better.
    static unsigned int evaluateHand(const char* hand);
};

} // namespace pkt::core