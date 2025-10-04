#pragma once
#include "core/engine/hand/ActionProcessor.h"
#include "core/engine/hand/DebuggableState.h"
#include "core/engine/hand/HandState.h"
#include "core/interfaces/HasLogger.h"

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

class PreflopState : public HandState, public HandActionProcessor, public HandDebuggableState
{
  public:
    PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId);
    explicit PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId,
                          std::shared_ptr<pkt::core::ServiceContainer> services);
    // ISP-compliant constructor using focused service interface
    PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId,
                 std::shared_ptr<HasLogger> logger);

    void enter(Hand&) override;
    void exit(Hand&) override;
    std::unique_ptr<HandState> computeNextState(Hand& hand) override;

    bool isActionAllowed(const Hand&, const PlayerAction) const override;

    std::shared_ptr<player::Player> getNextPlayerToAct(const Hand& hand) const override;
    std::shared_ptr<player::Player> getFirstPlayerToActInRound(const Hand& hand) const override;
    bool isRoundComplete(const Hand& hand) const override;

    void logStateInfo(Hand& hand) override;
    const GameState getGameState() const override { return GameState::Preflop; }
    void promptPlayerAction(Hand&, player::Player& player) override;

  protected:
    // ISP-compliant service access helper
    pkt::core::Logger& getLogger() const;

  private:
    void setBlinds(Hand& hand);
    void logHoleCards(Hand& hand);

    const GameEvents& m_events;
    const int m_smallBlind{0};
    unsigned m_dealerPlayerId{static_cast<unsigned>(-1)};
    unsigned m_smallBlindPlayerId{static_cast<unsigned>(-1)};
    unsigned m_bigBlindPlayerId{static_cast<unsigned>(-1)};
    mutable std::shared_ptr<pkt::core::ServiceContainer> m_services; // Legacy support
    std::shared_ptr<HasLogger> m_logger; // ISP-compliant focused dependency
};

} // namespace pkt::core