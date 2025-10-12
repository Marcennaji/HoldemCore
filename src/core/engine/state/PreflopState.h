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
 * @brief Represents the Preflop state in a Texas Hold'em hand.
 * 
 * The Preflop state is the initial betting round that occurs after players receive their hole cards
 * but before any community cards are dealt. Players can call, raise, or fold based on their hole cards.
 * This state handles blinds posting and manages the first round of betting actions.
 */
class PreflopState : public HandState, public HandActionProcessor, public HandDebuggableState
{
  public:
    PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId,
                 Logger& logger);

    void enter(Hand&) override;
    void exit(Hand&) override;
    std::unique_ptr<HandState> computeNextState(Hand& hand) override;

    bool isActionAllowed(const Hand&, const PlayerAction) const override;

    std::shared_ptr<player::Player> getNextPlayerToAct(const Hand& hand) const override;
    std::shared_ptr<player::Player> getFirstPlayerToActInRound(const Hand& hand) const override;
    bool isRoundComplete(const Hand& hand) const override;

    void logStateInfo(Hand& hand) override;
    const GameState getGameState() const override { return GameState::Preflop; }
    void promptPlayerAction(Hand&, player::Player& player) override;

  private:
    void setBlinds(Hand& hand);
    void logHoleCards(Hand& hand);

    const GameEvents& m_events;
    const int m_smallBlind{0};
    unsigned m_dealerPlayerId{static_cast<unsigned>(-1)};
    unsigned m_smallBlindPlayerId{static_cast<unsigned>(-1)};
    unsigned m_bigBlindPlayerId{static_cast<unsigned>(-1)};
    Logger* m_logger;
};

} // namespace pkt::core