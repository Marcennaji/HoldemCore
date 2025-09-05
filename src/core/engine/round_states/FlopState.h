#pragma once
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IHandState.h"

namespace pkt::core::player
{
class PlayerFsm;
}
namespace pkt::core::player
{
class PlayerFsm;
}
namespace pkt::core
{
class GameEvents;

class FlopState : public IHandState, public IActionProcessor, public IDebuggableState
{
  public:
    explicit FlopState(const GameEvents& events);

    void enter(HandFsm&) override;
    void exit(HandFsm&) override;
    std::unique_ptr<IHandState> computeNextState(HandFsm& hand, const PlayerAction action) override;

    bool isActionAllowed(const HandFsm&, const PlayerAction) const override;

    void logStateInfo(const HandFsm&) const override;
    GameState getGameState() const override { return GameState::Flop; }
    void promptPlayerAction(HandFsm&, player::PlayerFsm& player) override;

  private:
    const GameEvents& myEvents;
};

} // namespace pkt::core