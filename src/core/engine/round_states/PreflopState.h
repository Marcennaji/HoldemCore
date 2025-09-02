#pragma once
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/interfaces/hand/IRoundCompletionChecker.h"

namespace pkt::core::player
{
class PlayerFsm;
}

namespace pkt::core
{

class GameEvents;

class PreflopState : public IHandState, public IActionProcessor, public IRoundCompletionChecker, public IDebuggableState
{
  public:
    PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId);

    void enter(HandFsm&) override;
    void exit(HandFsm&) override;
    std::unique_ptr<IHandState> computeNextState(HandFsm& IHand, const PlayerAction action) override;

    bool isRoundComplete(const HandFsm&) const override;
    bool isActionAllowed(const HandFsm&, const PlayerAction) const override;

    void logStateInfo(const HandFsm&) const override;
    GameState getGameState() const override { return GameState::Preflop; }
    void promptPlayerAction(HandFsm&, player::PlayerFsm& player);

  private:
    void assignButtons(HandFsm& hand);
    void setBlinds(HandFsm& hand);

    const GameEvents& myEvents;
    const int mySmallBlind;
    unsigned myDealerPlayerId;
    unsigned mySmallBlindPlayerId;
    unsigned myBigBlindPlayerId;
};

} // namespace pkt::core