#pragma once
#include "core/interfaces/IBettingRoundStateFsm.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
class GameEvents;

class AllInState : public IBettingRoundStateFsm
{
  private:
    GameState entryState = GameStatePreflop; // Which state we entered from
    bool showdownReady = false;

  public:
    AllInState(GameEvents& events);
    // Core state lifecycle
    void enter(IHand& Ihand) override;
    void exit(IHand& Ihand) override;
    std::unique_ptr<IBettingRoundStateFsm> processAction(IHand& Ihand, PlayerAction action) override;

    // State identification
    GameState getGameState() const override { return entryState; } // Returns the state we're Ihandling all-in for
    std::string getStateName() const override { return "AllIn"; }

    // State queries
    bool isRoundComplete(const IHand& Ihand) const override;
    bool canProcessAction(const IHand& Ihand, PlayerAction action) const override;

    // Utility for logging/debugging
    void logStateInfo(const IHand& Ihand) const override;

  private:
    GameEvents& myEvents;

    // All-in specific methods
    void handleAllInSituation(IHand& Ihand);
    void dealRemainingCards(IHand& Ihand);
    void collectPots(IHand& Ihand);
    void flipCards(IHand& Ihand);

    // Transition logic
    std::unique_ptr<IBettingRoundStateFsm> checkForTransition(IHand& Ihand);
    void determineEntryState(IHand& Ihand);
};

} // namespace pkt::core
