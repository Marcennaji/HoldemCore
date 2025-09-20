#pragma once
#include <memory>
#include "core/interfaces/HandEvaluationEngine.h"

namespace pkt::core
{
class ServiceContainer;
} // namespace pkt::core

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

    /// Evaluates the strength of a 7-card poker hand using the provided service container.
    /// \param hand A string representing the 7-card poker hand (5 board + 2 hole).
    /// \param services The service container providing the hand evaluation engine.
    /// \return A hand strength score where higher means better.
    static unsigned int evaluateHand(const char* hand, std::shared_ptr<pkt::core::ServiceContainer> services);
};

} // namespace pkt::core