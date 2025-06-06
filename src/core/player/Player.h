/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#pragma once

#include <core/engine/CardsValue.h>
#include <core/engine/EngineDefs.h>
#include <core/engine/model/PlayerStatistics.h>
#include <core/interfaces/IHand.h>
#include "CurrentHandActions.h"
#include "RangeManager.h"

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
class IHandAuditStore;
class IPlayersStatisticsStore;

} // namespace pkt::core

namespace pkt::core::player
{

enum PlayerType
{
    PLAYER_TYPE_COMPUTER,
    PLAYER_TYPE_HUMAN
};

class IBotStrategy;
class CurrentHandContext;

static const char* TightAggressiveBotStrategyName[] = {"Tintin",  "Tonio", "Theo",  "Ted",  "Thor",
                                                       "Taslima", "Tina",  "Tania", "Tata", "Timmy"};

static const char* LooseAggressiveBotStrategyName[] = {"Louis", "Louane", "Ludovic", "Lucas", "Laure",
                                                       "Leila", "Lino",   "Laurent", "Lucie", "Ludivine"};

static const char* ManiacBotStrategyName[] = {"Maurice",  "Milou",    "Michou", "Maelle",  "Mokhtar",
                                              "Mireille", "Marianne", "Momo",   "Maurane", "Maya"};

static const char* UltraTightBotStrategyName[] = {"Ursula",  "Uri",    "Ulrich", "Ulysses", "Urbain",
                                                  "Umberto", "Urania", "Ugo",    "Uma",     "Urso"};

static const char* HumanPlayerName = "You";

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
    Player(GameEvents*, IHandAuditStore*, IPlayersStatisticsStore*, int id, PlayerType type, std::string name, int sC,
           bool aS, int mB);

    virtual ~Player() = default;

    int getID() const;
    void setID(unsigned newId);

    PlayerType getType() const;
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

    void setBestHandPosition(int* theValue);
    void getBestHandPosition(int* theValue) const;

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
    void computeEstimatedPreflopRange(const int playerId) const;
    const SimResults getHandSimulation() const;

    bool isAgressor(const GameState gameState) const;

    void updateUnplausibleRangesGivenPreflopActions();
    void updateUnplausibleRangesGivenFlopActions();
    void updateUnplausibleRangesGivenTurnActions();
    void updateUnplausibleRangesGivenRiverActions();

    bool isInVeryLooseMode(const int nbPlayers) const;

    std::unique_ptr<RangeManager>& getRangeManager() { return myRangeManager; }
    int getPreflopPotOdd() const;

  protected:
    const PreflopStatistics getPreviousRaiserStats(const int opponentId, const PreflopStatistics& opponentStats) const;
    void loadStatistics();
    void resetPlayerStatistics();
    float getMaxOpponentsStrengths() const;
    bool isPreflopBigBet() const;
    float getOpponentWinningHandsPercentage(const int idPlayer, std::string board) const;
    void logUnplausibleHands(GameState g);

    bool canBluff(const GameState) const;

    bool isUnplausibleHandGivenFlopCheck(const PostFlopState& r, const FlopStatistics& flop);
    bool isUnplausibleHandGivenFlopBet(const PostFlopState& r, int nbChecks, const FlopStatistics& flop);
    bool isUnplausibleHandGivenFlopCall(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                        const FlopStatistics& flop);
    bool isUnplausibleHandGivenFlopRaise(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                         const FlopStatistics& flop);
    bool isUnplausibleHandGivenFlopAllin(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                         const FlopStatistics& flop);

    bool isUnplausibleHandGivenTurnCheck(const PostFlopState& r, const TurnStatistics&);
    bool isUnplausibleHandGivenTurnBet(const PostFlopState& r, int nbChecks, const TurnStatistics&);
    bool isUnplausibleHandGivenTurnCall(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                        const TurnStatistics&);
    bool isUnplausibleHandGivenTurnRaise(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                         const TurnStatistics&);
    bool isUnplausibleHandGivenTurnAllin(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                         const TurnStatistics&);

    bool isUnplausibleHandGivenRiverCheck(const PostFlopState& r, const RiverStatistics&);
    bool isUnplausibleHandGivenRiverBet(const PostFlopState& r, int nbChecks, const RiverStatistics&);
    bool isUnplausibleHandGivenRiverCall(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                         const RiverStatistics&);
    bool isUnplausibleHandGivenRiverRaise(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                          const RiverStatistics&);
    bool isUnplausibleHandGivenRiverAllin(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                          const RiverStatistics&);

    // attributes

    IHandAuditStore* myHandAuditStore;
    IPlayersStatisticsStore* myPlayersStatisticsStore;
    IHand* currentHand;
    GameEvents* myEvents;

    CurrentHandActions myCurrentHandActions;

    // const
    int myID;
    PlayerType myType;
    std::string myName;
    std::string myAvatar;

    // vars
    PlayerPosition myPosition;
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> myStatistics;
    int myCardsValueInt;
    int myBestHandPosition[5];

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
    std::unique_ptr<RangeManager> myRangeManager;

    bool m_isSessionActive;
};
} // namespace pkt::core::player
