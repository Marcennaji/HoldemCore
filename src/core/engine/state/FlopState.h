#pragma once
#include "core/engine/hand/ActionProcessor.h"
#include "core/engine/hand/DebuggableState.h"
#include "core/engine/hand/HandState.h"
#include "core/interfaces/HasLogger.h"

namespace pkt::core::player
{
class Player;
}
namespace pkt::core::player
{
class Player;
}
namespace pkt::core
{
class GameEvents;

class FlopState : public HandState, public HandActionProcessor, public HandDebuggableState
{
  public:
    explicit FlopState(const GameEvents& events);
    
    // ISP-compliant constructor - only depends on what it needs (DIP principle)
    explicit FlopState(const GameEvents& events, std::shared_ptr<HasLogger> logger);

    void enter(Hand&) override;
    void exit(Hand&) override;
    std::unique_ptr<HandState> computeNextState(Hand& hand) override;

    bool isActionAllowed(const Hand&, const PlayerAction) const override;

    std::shared_ptr<player::Player> getNextPlayerToAct(const Hand& hand) const override;
    std::shared_ptr<player::Player> getFirstPlayerToActInRound(const Hand& hand) const override;
    bool isRoundComplete(const Hand& hand) const override;

    const GameState getGameState() const override { return GameState::Flop; }
    void promptPlayerAction(Hand&, player::Player& player) override;
    
    // Override to use focused logging dependency (LSP compliance)
    void logStateInfo(Hand& hand) override;

  private:
    Logger& getLogger(); // Helper method following SRP
    
    const GameEvents& m_events;
    std::shared_ptr<HasLogger> m_loggerService; // ISP-compliant dependency
};

} // namespace pkt::core