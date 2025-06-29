#pragma once
#include "core/interfaces/IBettingRoundState.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{

class AllInState : public IBettingRoundState
{
  private:
    GameState entryState = GameStatePreflop; // Which state we entered from
    bool showdownReady = false;

  public:
    // Core state lifecycle
    void enter(Hand& hand) override;
    void exit(Hand& hand) override;
    std::unique_ptr<IBettingRoundState> processAction(Hand& hand, PlayerAction action) override;

    // State identification
    GameState getGameState() const override { return entryState; } // Returns the state we're handling all-in for
    std::string getStateName() const override { return "AllIn"; }

    // State queries
    bool isRoundComplete(const Hand& hand) const override;
    bool canProcessAction(const Hand& hand, PlayerAction action) const override;

    // Utility for logging/debugging
    void logStateInfo(const Hand& hand) const override;

  private:
    // All-in specific methods
    void handleAllInSituation(Hand& hand);
    void dealRemainingCards(Hand& hand);
    void collectPots(Hand& hand);
    void flipCards(Hand& hand);

    // Transition logic
    std::unique_ptr<IBettingRoundState> checkForTransition(Hand& hand);
    void determineEntryState(Hand& hand);
};

} // namespace pkt::core
