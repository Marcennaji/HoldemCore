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

class PostRiverState : public IHandState, public IActionProcessor, public IDebuggableState
{
  public:
    explicit PostRiverState(const GameEvents& events);

    void enter(HandFsm&) override;
    void exit(HandFsm&) override;
    std::unique_ptr<IHandState> computeNextState(HandFsm& hand, const PlayerAction action) override;

    bool isActionAllowed(const HandFsm&, const PlayerAction) const override;
    bool isTerminal() const override { return true; }

    void logStateInfo(const HandFsm&) const override;
    GameState getGameState() const override { return GameState::PostRiver; }
    void promptPlayerAction(HandFsm&, player::PlayerFsm& player) override;

  private:
    const GameEvents& myEvents;
};

} // namespace pkt::core