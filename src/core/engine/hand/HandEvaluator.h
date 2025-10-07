#pragma once
#include <memory>
#include "core/interfaces/HandEvaluationEngine.h"

namespace pkt::core
{

/// Poker hand evaluation utility class using pluggable evaluation engines
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