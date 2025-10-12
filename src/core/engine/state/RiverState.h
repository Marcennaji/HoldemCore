// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once
#include "core/engine/hand/ActionProcessor.h"
#include "core/engine/hand/DebuggableState.h"
#include "core/engine/hand/HandState.h"
#include "core/ports/Logger.h"

#include <memory>

namespace pkt::core::player
{
class Player;
}

namespace pkt::core
{
class GameEvents;

/**
 * @brief Represents the River state in a Texas Hold'em hand.
 * 
 * The River state occurs after the turn when the fifth and final community card is dealt.
 * This is the final betting round before showdown where players make their final decisions.
 * Players can check, bet, call, raise, or fold during this betting round.
 */
class RiverState : public HandState, public HandActionProcessor, public HandDebuggableState
{
  public:

    explicit RiverState(const GameEvents& events, Logger& logger);

    void enter(Hand&) override;
    void exit(Hand&) override;
    std::unique_ptr<HandState> computeNextState(Hand& hand) override;

    bool isActionAllowed(const Hand&, const PlayerAction) const override;

    std::shared_ptr<player::Player> getNextPlayerToAct(const Hand& hand) const override;
    std::shared_ptr<player::Player> getFirstPlayerToActInRound(const Hand& hand) const override;
    bool isRoundComplete(const Hand& hand) const override;

    const GameState getGameState() const override { return GameState::River; }
    void promptPlayerAction(Hand&, player::Player& player) override;

    void logStateInfo(Hand& hand) override;

  private:

    const GameEvents& m_events;
    Logger& m_logger; 
};

} // namespace pkt::core