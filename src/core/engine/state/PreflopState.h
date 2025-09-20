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

class PreflopState : public IHandState, public IActionProcessor, public IDebuggableState
{
  public:
    PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId);
    explicit PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId,
                          std::shared_ptr<pkt::core::ServiceContainer> services);

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
    void ensureServicesInitialized() const;

    const GameEvents& myEvents;
    const int mySmallBlind{0};
    unsigned myDealerPlayerId{static_cast<unsigned>(-1)};
    unsigned mySmallBlindPlayerId{static_cast<unsigned>(-1)};
    unsigned myBigBlindPlayerId{static_cast<unsigned>(-1)};
    mutable std::shared_ptr<pkt::core::ServiceContainer> myServices;
};

} // namespace pkt::core