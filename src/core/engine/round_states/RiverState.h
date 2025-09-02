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

class RiverState : public IHandState, public IActionProcessor, public IRoundCompletionChecker, public IDebuggableState
{
  public:
    explicit RiverState(const GameEvents& events);

    void enter(HandFsm&) override;
    void exit(HandFsm&) override;
    std::unique_ptr<IHandState> computeNextState(HandFsm& hand, const PlayerAction action) override;

    bool isRoundComplete(const HandFsm&) const override;
    bool isActionAllowed(const HandFsm&, const PlayerAction) const override;

    void logStateInfo(const HandFsm&) const override;
    GameState getGameState() const override { return GameState::River; }
    void promptPlayerAction(HandFsm&, player::PlayerFsm& player);

  private:
    const GameEvents& myEvents;
};

} // namespace pkt::core