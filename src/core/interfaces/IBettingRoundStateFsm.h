#pragma once
#include <memory>
#include <string>
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

class Hand; // Forward declaration

class IBettingRoundStateFsm
{
  public:
    virtual ~IBettingRoundStateFsm() = default;

    // Core state lifecycle
    virtual void enter(Hand& hand) = 0;
    virtual void exit(Hand& hand) = 0;

    // Action processing - returns next state if transition needed
    virtual std::unique_ptr<IBettingRoundStateFsm> processAction(Hand& hand, PlayerAction action) = 0;

    // State identification
    virtual GameState getGameState() const = 0;
    virtual std::string getStateName() const = 0;

    // State queries
    virtual bool isRoundComplete(const Hand& hand) const = 0;
    virtual bool canProcessAction(const Hand& hand, PlayerAction action) const = 0;

    // Utility for logging/debugging
    virtual void logStateInfo(const Hand& hand) const {}
};

} // namespace pkt::core