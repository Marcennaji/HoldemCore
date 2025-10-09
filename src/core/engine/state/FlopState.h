#pragma once
#include "core/engine/hand/ActionProcessor.h"
#include "core/engine/hand/DebuggableState.h"
#include "core/engine/hand/HandState.h"
#include "core/interfaces/Logger.h"

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
    explicit FlopState(const GameEvents& events, Logger& logger);

    void enter(Hand&) override;
    void exit(Hand&) override;
    std::unique_ptr<HandState> computeNextState(Hand& hand) override;

    bool isActionAllowed(const Hand&, const PlayerAction) const override;

    std::shared_ptr<player::Player> getNextPlayerToAct(const Hand& hand) const override;
    std::shared_ptr<player::Player> getFirstPlayerToActInRound(const Hand& hand) const override;
    bool isRoundComplete(const Hand& hand) const override;

    const GameState getGameState() const override { return GameState::Flop; }
    void promptPlayerAction(Hand&, player::Player& player) override;
    
    void logStateInfo(Hand& hand) override;

  private:
    const GameEvents& m_events;
    Logger* m_logger; 
};

} // namespace pkt::core