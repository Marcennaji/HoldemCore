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

class TurnState : public IHandState, public IActionProcessor, public IDebuggableState
{
  public:
    explicit TurnState(const GameEvents& events);

    void enter(HandFsm&) override;
    void exit(HandFsm&) override;
    std::unique_ptr<IHandState> computeNextState(HandFsm& hand) override;

    bool isActionAllowed(const HandFsm&, const PlayerAction) const override;

    std::shared_ptr<player::PlayerFsm> getNextPlayerToAct(const HandFsm& hand) const override;
    std::shared_ptr<player::PlayerFsm> getFirstPlayerToActInRound(const HandFsm& hand) const override;
    bool isRoundComplete(const HandFsm& hand) const override;

    void logStateInfo(const HandFsm&) const override;
    const GameState getGameState() const override { return GameState::Turn; }
    void promptPlayerAction(HandFsm&, player::PlayerFsm& player) override;

  private:
    const GameEvents& myEvents;
};

} // namespace pkt::core