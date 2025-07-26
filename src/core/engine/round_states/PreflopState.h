#pragma once
#include "core/engine/round_states/BettingStateBase.h"
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IRoundCompletionChecker.h"

namespace pkt::core::player
{
class Player;
}

namespace pkt::core
{

class GameEvents;

class PreflopState : public BettingStateBase,
                     public IActionProcessor,
                     public IRoundCompletionChecker,
                     public IDebuggableState
{
  public:
    PreflopState(const GameEvents& events, const int smallBlind);

    void enter(HandFsm&) override;
    void exit(HandFsm&) override;
    std::unique_ptr<IHandState> processAction(HandFsm& IHand, const PlayerAction action) override;

    bool isRoundComplete(const HandFsm&) const override;
    bool canProcessAction(const HandFsm&, const PlayerAction) const override;

    void logStateInfo(const HandFsm&) const override;
    std::string getStateName() const override { return "Preflop"; }
    void promptPlayerAction(HandFsm&, player::Player& player);

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