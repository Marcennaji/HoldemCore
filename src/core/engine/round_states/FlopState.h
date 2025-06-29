#pragma once
#include "core/interfaces/IBettingRoundState.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{

class FlopState : public IBettingRoundState
{
  private:
    bool bettingComplete = false;
    int highestBet = 0;
    int lastRaiserId = -1;
    int currentPlayerToAct = -1;
    int playersActedCount = 0;

  public:
    // Core state lifecycle
    void enter(Hand& hand) override;
    void exit(Hand& hand) override;
    std::unique_ptr<IBettingRoundState> processAction(Hand& hand, PlayerAction action) override;

    // State identification
    GameState getGameState() const override { return GameStateFlop; }
    std::string getStateName() const override { return "Flop"; }

    // State queries
    bool isRoundComplete(const Hand& hand) const override;
    bool canProcessAction(const Hand& hand, PlayerAction action) const override;

    // Utility for logging/debugging
    void logStateInfo(const Hand& hand) const override;

  private:
    // Setup methods
    void initializeBettingRound(Hand& hand);
    void dealFlopCards(Hand& hand);
    void setFirstPlayerToAct(Hand& hand);

    // Action processing
    void processPlayerBetting(Hand& hand, PlayerAction action);
    void advanceToNextPlayer(Hand& hand);

    // Transition logic
    std::unique_ptr<IBettingRoundState> checkForTransition(Hand& hand);
    bool checkSinglePlayerRemaining(const Hand& hand) const;
    bool checkAllInCondition(const Hand& hand) const;

    // Validation
    bool validateAction(const Hand& hand, PlayerAction action) const;
    bool isBettingComplete(const Hand& hand) const;

    // Player management
    void updatePlayerLists(Hand& hand);
    int getNextPlayerToAct(const Hand& hand) const;
};

} // namespace pkt::core
