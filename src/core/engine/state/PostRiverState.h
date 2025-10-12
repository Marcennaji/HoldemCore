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
 * @brief Represents the Post-River state in a Texas Hold'em hand.
 * 
 * The Post-River state occurs after all betting rounds are complete and handles
 * the showdown phase where players reveal their cards and the winner is determined.
 * This state manages pot distribution and hand resolution.
 */
class PostRiverState : public HandState, public HandActionProcessor, public HandDebuggableState
{
  public:
    PostRiverState(const GameEvents& events, Logger& logger);

    void enter(Hand&) override;
    void exit(Hand&) override;
    std::unique_ptr<HandState> computeNextState(Hand& hand) override;

    bool isActionAllowed(const Hand&, const PlayerAction) const override;
    bool isTerminal() const override { return true; }

    std::shared_ptr<player::Player> getNextPlayerToAct(const Hand& hand) const override;
    std::shared_ptr<player::Player> getFirstPlayerToActInRound(const Hand& hand) const override;
    bool isRoundComplete(const Hand& hand) const override;

    const GameState getGameState() const override { return GameState::PostRiver; }
    void promptPlayerAction(Hand&, player::Player& player) override;

    void logStateInfo(Hand& hand) override;

  private:
    const GameEvents& m_events;
    Logger& m_logger; 
};

} // namespace pkt::core