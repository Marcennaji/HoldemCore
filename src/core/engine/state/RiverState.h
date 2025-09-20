#pragma once
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IHandState.h"

#include <memory>

namespace pkt::core
{
class ServiceContainer;
} // namespace pkt::core

namespace pkt::core::player
{
class Player;
}

namespace pkt::core
{
class GameEvents;

class RiverState : public IHandState, public IActionProcessor, public IDebuggableState
{
  public:
    explicit RiverState(const GameEvents& events);
    explicit RiverState(const GameEvents& events, std::shared_ptr<pkt::core::ServiceContainer> services);

    void enter(Hand&) override;
    void exit(Hand&) override;
    std::unique_ptr<IHandState> computeNextState(Hand& hand) override;

    bool isActionAllowed(const Hand&, const PlayerAction) const override;

    std::shared_ptr<player::Player> getNextPlayerToAct(const Hand& hand) const override;
    std::shared_ptr<player::Player> getFirstPlayerToActInRound(const Hand& hand) const override;
    bool isRoundComplete(const Hand& hand) const override;

    const GameState getGameState() const override { return GameState::River; }
    void promptPlayerAction(Hand&, player::Player& player) override;

  private:
    void ensureServicesInitialized() const;

    const GameEvents& myEvents;
    mutable std::shared_ptr<pkt::core::ServiceContainer> myServices;
};

} // namespace pkt::core