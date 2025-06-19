// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/CardsValue.h>
#include <core/engine/EngineDefs.h>
#include <core/engine/model/PlayerStatistics.h>
#include <core/interfaces/IHand.h>

#include "CurrentHandActions.h"
#include "range/RangeEstimator.h"
#include "strategy/CurrentHandContext.h"

#include <third_party/psim/psim.hpp>

#include <array>
#include <assert.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace pkt::core
{
class IHand;

} // namespace pkt::core

namespace pkt::core::player
{

class IBotStrategy;

// values are odd %, according to the outs number. Array index is the number of outs
static int outsOddsOneCard[] = {
    0,  2,  4,  6,  8,  11, /* 0 to 5 outs */
    13, 15, 17, 19, 21,     /* 6 to 10 outs */
    24, 26, 28, 30, 32,     /* 11 to 15 outs */
    34, 36, 39, 41, 43      /* 16 to 20 outs */
};

static int outsOddsTwoCard[] = {
    0,  4,  8,  13, 17, 20, /* 0 to 5 outs */
    24, 28, 32, 35, 38,     /* 6 to 10 outs */
    42, 45, 48, 51, 54,     /* 11 to 15 outs */
    57, 60, 62, 65, 68      /* 16 to 20 outs */
};

class Player
{
  public:
    Player(const GameEvents&, int id, std::string name, int sC, bool aS, int mB);

    virtual ~Player() = default;

    int getID() const;
    void setID(unsigned newId);

    void setName(const std::string& theValue);
    std::string getName() const;

    void setCash(int theValue);
    int getCash() const;
    void setSet(int theValue);
    void setSetNull();
    int getSet() const;
    int getLastRelativeSet() const;

    void setHand(IHand*);
    IHand* getHand() const { return currentHand; }
    void setAction(PlayerAction theValue, bool blind = 0);
    PlayerAction getAction() const;

    virtual std::string getStrategyName() const = 0;

    void setButton(int theValue);
    int getButton() const;
    void setActiveStatus(bool theValue);
    bool getActiveStatus() const;

    void setCards(int* theValue);
    void getCards(int* theValue) const;

    void setTurn(bool theValue);
    bool getTurn() const;

    void setCardsFlip(bool theValue);
    bool getCardsFlip() const;

    void setCardsValueInt(int theValue);
    int getCardsValueInt() const;

    void setRoundStartCash(int theValue);
    int getRoundStartCash() const;

    void setLastMoneyWon(int theValue);
    int getLastMoneyWon() const;

    std::string getCardsValueString() const;

    const PlayerPosition getPosition() const;
    void setPosition();
    std::string getPositionLabel(PlayerPosition p) const;

    static bool getHavePosition(PlayerPosition myPos, PlayerList runningPlayers);

    void setIsSessionActive(bool active);
    bool isSessionActive() const;

    bool checkIfINeedToShowCards() const;

    float getM() const;

    const PlayerStatistics& getStatistics(const int nbPlayers) const;
    CurrentHandContext& getCurrentHandContext() { return *myCurrentHandContext; };

    virtual float calculatePreflopCallingRange(CurrentHandContext& context, bool deterministic = false) const;

    const PostFlopState getPostFlopState() const;
    CurrentHandActions& getCurrentHandActions();

    void updatePreflopStatistics();
    void updateFlopStatistics();
    void updateTurnStatistics();
    void updateRiverStatistics();

    std::shared_ptr<Player> getPlayerByUniqueId(unsigned id) const;
    int getPotOdd() const;
    void setPreflopPotOdd(const int potOdd);

    std::string getStringBoard() const;
    std::map<int, float> evaluateOpponentsStrengths() const;
    const SimResults getHandSimulation() const;

    bool isAgressor(const GameState gameState) const;

    void updateCurrentHandContext(const GameState gameState);

    bool isInVeryLooseMode(const int nbPlayers) const;

    std::unique_ptr<RangeEstimator>& getRangeEstimator() { return myRangeEstimator; }
    int getPreflopPotOdd() const;

  protected:
    void loadStatistics();
    void resetPlayerStatistics();
    float getMaxOpponentsStrengths() const;
    bool isPreflopBigBet() const;
    float getOpponentWinningHandsPercentage(const int idPlayer, std::string board) const;
    void logUnplausibleHands(GameState g);

    bool canBluff(const GameState) const;

    // attributes

    IHand* currentHand;
    const GameEvents& myEvents;

    CurrentHandActions myCurrentHandActions;
    std::unique_ptr<CurrentHandContext> myCurrentHandContext;

    // const
    int myID;
    std::string myName;
    std::string myAvatar;

    // vars
    PlayerPosition myPosition;
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> myStatistics;
    int myCardsValueInt;

    int myCards[2];
    std::string myCard1;
    std::string myCard2;
    int myCash;
    int mySet;
    int myLastRelativeSet;
    PlayerAction myAction;
    int myButton;        // 0 = none, 1 = dealer, 2 =small, 3 = big
    bool myActiveStatus; // 0 = inactive, 1 = active
    bool myTurn;         // 0 = no, 1 = yes
    bool myCardsFlip;    // 0 = cards are not fliped, 1 = cards are already flipped,
    int myRoundStartCash;
    int lastMoneyWon;
    int myPreflopPotOdd;
    std::unique_ptr<RangeEstimator> myRangeEstimator;

    bool m_isSessionActive;
};
} // namespace pkt::core::player
