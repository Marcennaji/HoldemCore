#pragma once
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/interfaces/hand/IRoundCompletionChecker.h"

namespace pkt::core::player
{
class Player;
}

namespace pkt::core
{

class GameEvents;

class PreflopState : public IHandState, public IActionProcessor, public IRoundCompletionChecker, public IDebuggableState
{
  public:
    PreflopState(GameEvents& events);

    void enter(IHand& IHand) override;
    void exit(IHand& IHand) override;
    std::unique_ptr<IHandState> processAction(IHand& IHand, const PlayerAction action) override;

    bool isRoundComplete(const IHand& IHand) const override;
    bool canProcessAction(const IHand&, const PlayerAction) const override;

    void logStateInfo(const IHand& IHand) const override;
    std::string getStateName() const override { return "Preflop"; }
    void handlePlayerAction(IHand& hand, player::Player& player);

  private:
    GameEvents& myEvents;
};

} // namespace pkt::core