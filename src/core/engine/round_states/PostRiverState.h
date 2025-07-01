#pragma once
#include "core/interfaces/IBettingRoundStateFsm.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{

class PostRiverState : public IBettingRoundStateFsm
{
  private:
    bool showdownComplete = false;
    int highestCardsValue = 0;

  public:
    // Core state lifecycle
    void enter(Hand& hand) override;
    void exit(Hand& hand) override;
    std::unique_ptr<IBettingRoundStateFsm> processAction(Hand& hand, PlayerAction action) override;

    // State identification
    GameState getGameState() const override { return GameStatePostRiver; }
    std::string getStateName() const override { return "PostRiver"; }

    // State queries
    bool isRoundComplete(const Hand& hand) const override;
    bool canProcessAction(const Hand& hand, PlayerAction action) const override;

    // Utility for logging/debugging
    void logStateInfo(const Hand& hand) const override;

  private:
    // Showdown methods
    void runShowdown(Hand& hand);
    void determineWinners(Hand& hand);
    void showCards(Hand& hand);
    void distributePots(Hand& hand);
    void updateStatistics(Hand& hand);
    void handleHandPause(Hand& hand);

    // Helper methods
    void calculateHighestCardsValue(Hand& hand);
    bool shouldPauseHand(Hand& hand) const;
    int countNonFoldedPlayers(Hand& hand) const;
};

} // namespace pkt::core
