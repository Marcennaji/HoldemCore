#pragma once
#include "core/engine/Hand.h"
#include "core/interfaces/IBettingRoundStateFsm.h"

namespace pkt::core
{

class PreflopState : public IBettingRoundStateFsm
{
  private:
    bool bettingComplete = false;
    int highestBet = 0;
    int lastRaiserId = -1;
    int currentPlayerToAct = -1;
    int playersActedCount = 0;

  public:
    void enter(Hand& hand) override;
    void exit(Hand& hand) override;
    std::unique_ptr<IBettingRoundStateFsm> processAction(Hand& hand, PlayerAction action) override;

    GameState getGameState() const override { return GameStatePreflop; }
    std::string getStateName() const override { return "Preflop"; }

    bool isRoundComplete(const Hand& hand) const override;
    bool canProcessAction(const Hand& hand, PlayerAction action) const override;

    void logStateInfo(const Hand& hand) const override;

  private:
    // Setup methods
    void initializeBettingRound(Hand& hand);
    void postBlinds(Hand& hand);
    void setFirstPlayerToAct(Hand& hand);

    // Action processing
    void processPlayerBetting(Hand& hand, PlayerAction action);
    void advanceToNextPlayer(Hand& hand);

    // Transition logic
    std::unique_ptr<IBettingRoundStateFsm> checkForTransition(Hand& hand);
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