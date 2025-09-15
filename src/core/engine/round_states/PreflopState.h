#pragma once
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IHandState.h"

namespace pkt::core::player
{
class PlayerFsm;
}

namespace pkt::core
{

class GameEvents;

class PreflopState : public IHandState, public IActionProcessor, public IDebuggableState
{
  public:
    PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId);

    void enter(HandFsm&) override;
    void exit(HandFsm&) override;
    std::unique_ptr<IHandState> computeNextState(HandFsm& hand) override;

    bool isActionAllowed(const HandFsm&, const PlayerAction) const override;

    std::shared_ptr<player::PlayerFsm> getNextPlayerToAct(const HandFsm& hand) const override;
    std::shared_ptr<player::PlayerFsm> getFirstPlayerToActInRound(const HandFsm& hand) const override;
    bool isRoundComplete(const HandFsm& hand) const override;

    void logStateInfo(HandFsm& hand) override;
    const GameState getGameState() const override { return GameState::Preflop; }
    void promptPlayerAction(HandFsm&, player::PlayerFsm& player) override;

  private:
    void setBlinds(HandFsm& hand);
    void logHoleCards(HandFsm& hand);

    const GameEvents& myEvents;
    const int mySmallBlind;
    unsigned myDealerPlayerId;
    unsigned mySmallBlindPlayerId;
    unsigned myBigBlindPlayerId;
};

} // namespace pkt::core