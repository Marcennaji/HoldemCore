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

#include <third_party/psim/psim.hpp>
#include <third_party/sqlite3/sqlite3.h>

#include <array>
#include <assert.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

enum PlayerType
{
    PLAYER_TYPE_COMPUTER,
    PLAYER_TYPE_HUMAN
};

class IHandAuditStore;
class IPlayersStatisticsStore;
class IHand;

class CurrentHandActions
{
  public:
    CurrentHandActions() { reset(); };
    ~CurrentHandActions() {};

    void reset(); // init to zero

    std::vector<PlayerAction>& getPreflopActions();
    std::vector<PlayerAction>& getFlopActions();
    std::vector<PlayerAction>& getTurnActions();
    std::vector<PlayerAction>& getRiverActions();

  protected:
    friend class Player;

    std::vector<PlayerAction> m_preflopActions;
    std::vector<PlayerAction> m_flopActions;
    std::vector<PlayerAction> m_turnActions;
    std::vector<PlayerAction> m_riverActions;
};

static const char* TightAgressivePlayerName[] = {"Tintin",  "Tonio", "Theo",  "Ted",  "Thor",
                                                 "Taslima", "Tina",  "Tania", "Tata", "Timmy"};

static const char* LooseAggressivePlayerName[] = {"Louis", "Louane", "Ludovic", "Lucas", "Laure",
                                                  "Leila", "Lino",   "Laurent", "Lucie", "Ludivine"};

static const char* ManiacPlayerName[] = {"Maurice",  "Milou",    "Michou", "Maelle",  "Mokhtar",
                                         "Mireille", "Marianne", "Momo",   "Maurane", "Maya"};

static const char* UltraTightPlayerName[] = {"Ursula",  "Uri",    "Ulrich", "Ulysses", "Urbain",
                                             "Umberto", "Urania", "Ugo",    "Uma",     "Urso"};

static const char* HumanPlayerName[] = {" "};

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
    Player(IHandAuditStore*, IPlayersStatisticsStore*, int id, PlayerType type, std::string name, int sC, bool aS,
           bool sotS, int mB);

    ~Player();

    void doPreflopAction();
    void doFlopAction();
    void doTurnAction();
    void doRiverAction();

    int getID() const;
    void setID(unsigned newId);
    void setGuid(const std::string& theValue);
    std::string getGuid() const;
    PlayerType getType() const;
    void setName(const std::string& theValue);
    std::string getName() const;
    void setAvatar(const std::string& theValue);
    std::string getAvatar() const;
    void setCash(int theValue);
    int getCash() const;
    void setSet(int theValue);
    void setSetAbsolute(int theValue);
    void setSetNull();
    int getSet() const;
    int getLastRelativeSet() const;

    void setHand(IHand*);
    void setAction(PlayerAction theValue, bool blind = 0);
    PlayerAction getAction() const;

    void setButton(int theValue);
    int getButton() const;
    void setActiveStatus(bool theValue);
    bool getActiveStatus() const;

    void setStayOnTableStatus(bool theValue);
    bool getStayOnTableStatus() const;

    void setCards(int* theValue);
    void getCards(int* theValue) const;

    void setTurn(bool theValue);
    bool getTurn() const;

    void setCardsFlip(bool theValue, int state);
    bool getCardsFlip() const;

    void setCardsValueInt(int theValue);
    int getCardsValueInt() const;

    void setLogHoleCardsDone(bool theValue);

    bool getLogHoleCardsDone() const;

    void setBestHandPosition(int* theValue);
    void getBestHandPosition(int* theValue) const;

    void setRoundStartCash(int theValue);
    int getRoundStartCash() const;

    void setLastMoneyWon(int theValue);
    int getLastMoneyWon() const;

    std::string getCardsValueString() const;

    void action();

    const PlayerPosition getPosition() const;
    void setPosition();
    std::string getPositionLabel(PlayerPosition p) const;

    void setEstimatedRange(const std::string);

    bool getHavePosition(PlayerPosition myPos, PlayerList runningPlayers) const;

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

    void DisplayHandState(const PostFlopState* state) const;
    std::vector<std::string> getRangeAtomicValues(std::string range, const bool returnRange = false) const;
    std::string getStringBoard() const;
    std::map<int, float> evaluateOpponentsStrengths() const;
    void computeEstimatedPreflopRange(const int playerId) const;
    const SimResults getHandSimulation() const;

    float getPreflopCallingRange() const;
    float getPreflopRaisingRange() const;
    bool isAgressor(const GameState gameState) const;
    std::string getEstimatedRange() const;

    void updateUnplausibleRangesGivenPreflopActions();
    void updateUnplausibleRangesGivenFlopActions();
    void updateUnplausibleRangesGivenTurnActions();
    void updateUnplausibleRangesGivenRiverActions();

    std::string substractRange(const std::string startingRange, const std::string rangeToSubstract,
                               const std::string board = "");

    bool isInVeryLooseMode() const;

  protected:
    virtual bool preflopShouldCall() = 0;
    virtual bool flopShouldCall() = 0;
    virtual bool turnShouldCall() = 0;
    virtual bool riverShouldCall() = 0;

    virtual bool preflopShouldRaise() = 0;
    virtual bool flopShouldRaise() = 0;
    virtual bool turnShouldRaise() = 0;
    virtual bool riverShouldRaise() = 0;

    virtual bool flopShouldBet() = 0;
    virtual bool turnShouldBet() = 0;
    virtual bool riverShouldBet() = 0;

    bool shouldPotControl(const PostFlopState&, const SimResults&, const GameState) const;

    void loadStatistics();
    void resetPlayerStatistics();
    void simulateHand();
    bool isCardsInRange(std::string card1, std::string card2, std::string range) const;
    std::string getFakeCard(char c) const;
    void evaluateBetAmount();
    float getMaxOpponentsStrengths() const;
    bool isPreflopBigBet() const;
    float getOpponentWinningHandsPercentage(const int idPlayer, std::string board) const;
    void displayPlausibleRange(GameState g);
    std::string getFilledRange(std::vector<std::string>& ranges, std::vector<float>& rangesValues,
                               const float rangeMax) const;
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);
    int getRange(PlayerPosition p) const;
    std::string getStringRange(int nbPlayers, int range) const;
    std::string getHandToRange(const std::string card1, const std::string card2) const;
    int getBoardCardsHigherThan(std::string card) const;
    bool isDrawingProbOk() const;
    bool canBluff(const GameState) const;
    int getPreflopPotOdd() const;
    void computePreflopRaiseAmount();
    int getStandardRaisingRange(int nbPlayers) const;
    int getStandardCallingRange(int nbPlayers) const;
    std::string computeEstimatedPreflopRangeFromLastRaiser(const int opponentId,
                                                           PreflopStatistics& lastRaiserStats) const;
    std::string computeEstimatedPreflopRangeFromCaller(const int opponentId, PreflopStatistics& callerStats) const;

    // returns a % chance, for a winning draw
    const int getDrawingProbability() const;

    const int getImplicitOdd() const;

    char incrementCardValue(char c) const;

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

    // vector index is player position, value is range %
    std::vector<int> UTG_STARTING_RANGE;
    std::vector<int> UTG_PLUS_ONE_STARTING_RANGE;
    std::vector<int> UTG_PLUS_TWO_STARTING_RANGE;
    std::vector<int> MIDDLE_STARTING_RANGE;
    std::vector<int> MIDDLE_PLUS_ONE_STARTING_RANGE;
    std::vector<int> LATE_STARTING_RANGE;
    std::vector<int> CUTOFF_STARTING_RANGE;
    std::vector<int> BUTTON_STARTING_RANGE;
    std::vector<int> SB_STARTING_RANGE;
    std::vector<int> BB_STARTING_RANGE;

    IHandAuditStore* myHandAuditStore;
    IPlayersStatisticsStore* myPlayersStatisticsStore;
    IHand* currentHand;

    PostFlopState myFlopState;
    PostFlopState myTurnState;
    PostFlopState myRiverState;

    SimResults myFlopHandSimulation;
    SimResults myTurnHandSimulation;
    SimResults myRiverHandSimulation;

    CurrentHandActions myCurrentHandActions;

    // const
    int myID;
    std::string myGuid;
    PlayerType myType;
    std::string myName;
    std::string myAvatar;

    // vars
    PlayerPosition myPosition;
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> myStatistics;
    int myCardsValueInt;
    int myBestHandPosition[5];
    bool logHoleCardsDone;

    // current hand playing
    bool myShouldBet;
    bool myShouldRaise;
    bool myShouldCall;

    int myCards[2];
    std::string myCard1;
    std::string myCard2;
    int myCash;
    int mySet;
    int myRaiseAmount;
    int myBetAmount;
    int myLastRelativeSet;
    PlayerAction myAction;
    int myButton;             // 0 = none, 1 = dealer, 2 =small, 3 = big
    bool myActiveStatus;      // 0 = inactive, 1 = active
    bool myStayOnTableStatus; // 0 = left, 1 = stay
    bool myTurn;              // 0 = no, 1 = yes
    bool myCardsFlip;         // 0 = cards are not fliped, 1 = cards are already flipped,
    int myRoundStartCash;
    int lastMoneyWon;
    int myPreflopPotOdd;
    std::string myEstimatedRange; // estimated range for a particular hand (i.e, plausible range to my opponents eyes)

    bool m_isSessionActive;
};
