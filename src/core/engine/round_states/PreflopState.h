#pragma once
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IHandState.h"

namespace pkt::core::player
{
class Player;
}

namespace pkt::core
{

class GameEvents;

class PreflopState : public IHandState, public IActionProcessor, public IDebuggableState
{
  public:
    PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId);

    void enter(Hand&) override;
    void exit(Hand&) override;
    std::unique_ptr<IHandState> computeNextState(Hand& hand) override;

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

    const GameEvents& myEvents;
    const int mySmallBlind;
    unsigned myDealerPlayerId;
    unsigned mySmallBlindPlayerId;
    unsigned myBigBlindPlayerId;
};

} // namespace pkt::core