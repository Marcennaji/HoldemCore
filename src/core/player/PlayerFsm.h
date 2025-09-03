// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/HandEvaluator.h>

#include "core/engine/model/PlayerPosition.h"
#include "core/player/PlayerStatisticsUpdater.h"
#include "core/player/strategy/PlayerStrategy.h"

#include "CurrentHandActions.h"
#include "range/RangeEstimator.h"
#include "strategy/CurrentHandContext.h"

#include "core/interfaces/IHandEvaluationEngine.h"

#include <array>
#include <assert.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace pkt::core
{
class HandFsm;

} // namespace pkt::core

namespace pkt::core::player
{

class PlayerFsm
{
  public:
    PlayerFsm(const GameEvents&, int id, std::string name, int cash);

    virtual ~PlayerFsm() = default;

    int getId() const;
    std::string getName() const;

    void setStrategy(std::unique_ptr<PlayerStrategy> strategy) { myStrategy = std::move(strategy); }

    PlayerAction decideAction(const CurrentHandContext& ctx)
    {
        assert(myStrategy && "PlayerStrategy must be set before deciding action");
        return myStrategy->decideAction(ctx);
    }

    void setCash(int theValue);
    int getCash() const;
    void addBetAmount(int theValue);
    void resetBetAmount();
    int getTotalBetAmount() const;
    void setTotalBetAmount(int theValue);
    int getLastBetAmount() const;

    void setAction(ActionType theValue, bool blind = 0);
    ActionType getAction() const;

    void setCards(int* theValue);
    void getCards(int* theValue) const;

    void setCardsFlip(bool theValue);
    bool getCardsFlip() const;

    void setHandRanking(int theValue);
    int getHandRanking() const;

    void setCashAtHandStart(int theValue);
    int getCashAtHandStart() const;

    void setLastMoneyWon(int theValue);
    int getLastMoneyWon() const;

    std::string getCardsValueString() const;

    const PlayerPosition getPosition() const;
    void setPosition(const HandFsm& hand);

    bool checkIfINeedToShowCards() const;

    CurrentHandContext& getCurrentHandContext() { return *myCurrentHandContext; };

    virtual float calculatePreflopCallingRange(const CurrentHandContext& ctx) const;

    const PostFlopAnalysisFlags getPostFlopAnalysisFlags() const;
    CurrentHandActions& getCurrentHandActions();

    void setPreflopPotOdd(const int potOdd);

    bool isAgressor(const GameState gameState) const;

    void updateCurrentHandContext(const GameState gameState, HandFsm&);

    bool isInVeryLooseMode(const int nbPlayers) const;

    std::unique_ptr<RangeEstimator>& getRangeEstimator() { return myRangeEstimator; }
    int getPreflopPotOdd() const;

    void resetForNewHand(const HandFsm& hand);

    const std::unique_ptr<PlayerStatisticsUpdater>& getStatisticsUpdater() const { return myStatisticsUpdater; }

  protected:
    float getMaxOpponentsStrengths() const;
    bool isPreflopBigBet() const;
    float getOpponentWinningHandsPercentage(const int idPlayer, std::string board) const;

    const GameEvents& myEvents;
    CurrentHandActions myCurrentHandActions;
    std::unique_ptr<CurrentHandContext> myCurrentHandContext;
    const int myID;
    const std::string myName;
    PlayerPosition myPosition;

    int myHandRanking{0};
    int myCards[2];
    std::string myCard1;
    std::string myCard2;
    int myCash{0};
    int myCashAtHandStart{0};

    /**
     * Purpose: Represents the total amount of chips the player has committed to the pot during the hand.
     * Usage:
     * Tracks the cumulative amount the player has bet across all rounds.
     * Used in pot distribution calculations to determine the player's share of the pot.
     * Example:
     * If a player bets 200 in the preflop and 300 on the flop, myTotalBetAmount would be 500.
     */
    int myTotalBetAmount{0};

    /**
     * Purpose: Represents the amount of chips the player has committed to the pot during the most recent betting
     * action. Usage: Tracks the player's contribution during the current betting action (e.g., a call, raise, or
     * all-in). Helps calculate the player's odds and contributions for the current pot level. Example: If a player
     * raises by 300 chips, myLastBetAmount would be set to 300
     */
    int myLastBetAmount{0};

    ActionType myAction{ActionType::None};
    bool myCardsFlip{false}; // 0 = cards are not fliped, 1 = cards are already flipped,
    int lastMoneyWon{0};
    int myPreflopPotOdd{0};
    std::unique_ptr<RangeEstimator> myRangeEstimator;

  private:
    std::map<int, float> evaluateOpponentsStrengths() const;
    const HandSimulationStats computeHandSimulation() const;
    std::unique_ptr<PlayerStrategy> myStrategy;
    std::unique_ptr<PlayerStatisticsUpdater> myStatisticsUpdater;
};
} // namespace pkt::core::player
