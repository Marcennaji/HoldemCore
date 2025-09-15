// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/HandEvaluator.h>

#include "core/cards/Card.h"
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
class IHandState;

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

    void setAction(IHandState& state, const PlayerAction& action);
    PlayerAction getLastAction() const;

    // Legacy interface for compatibility with existing engine code that uses int arrays
    void setCards(int* theValue);
    void getCards(int* theValue) const;

    // Modern Card-based interface - preferred for new code
    const HoleCards& getHoleCards() const { return myHoleCards; }
    void setHoleCards(const HoleCards& cards) { myHoleCards = cards; }
    void setHoleCards(const Card& card1, const Card& card2) { myHoleCards = HoleCards(card1, card2); }
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
    const CurrentHandActions& getCurrentHandActions() const;
    void resetCurrentHandActions() { myCurrentHandActions.reset(); }

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
    HoleCards myHoleCards;
    int myCash{0};
    int myCashAtHandStart{0};

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
