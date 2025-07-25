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
    PreflopState(const GameEvents& events);

    void enter(HandFsm&) override;
    void exit(HandFsm&) override;
    std::unique_ptr<IHandState> processAction(HandFsm& IHand, const PlayerAction action) override;

    bool isRoundComplete(const HandFsm&) const override;
    bool canProcessAction(const HandFsm&, const PlayerAction) const override;

    void logStateInfo(const HandFsm&) const override;
    std::string getStateName() const override { return "Preflop"; }
    void handlePlayerAction(HandFsm&, player::Player& player);

  private:
    const GameEvents& myEvents;
};

} // namespace pkt::core