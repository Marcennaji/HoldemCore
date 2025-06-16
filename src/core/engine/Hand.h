// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/interfaces/ILogger.h>
#include "EngineFactory.h"
#include "Player.h"
#include "core/interfaces/IBettingRound.h"
#include "core/interfaces/IBoard.h"
#include "core/interfaces/IHand.h"

#include <vector>

namespace pkt::core
{
class IRankingStore;
class IPlayersStatisticsStore;
class IHandAuditStore;
class Hand : public IHand
{
  public:
    Hand(GameEvents*, ILogger*, std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard>, IRankingStore*,
         IPlayersStatisticsStore*, IHandAuditStore*, pkt::core::player::PlayerList, pkt::core::player::PlayerList,
         pkt::core::player::PlayerList, int, int, unsigned, int, int);
    ~Hand();

    void start();

    pkt::core::player::PlayerList getSeatsList() const { return seatsList; }
    pkt::core::player::PlayerList getActivePlayerList() const { return activePlayerList; }
    pkt::core::player::PlayerList getRunningPlayerList() const { return runningPlayerList; }

    std::shared_ptr<IBoard> getBoard() const { return myBoard; }
    std::shared_ptr<IBettingRound> getPreflop() const { return myBettingRound[GAME_STATE_PREFLOP]; }
    std::shared_ptr<IBettingRound> getFlop() const { return myBettingRound[GAME_STATE_FLOP]; }
    std::shared_ptr<IBettingRound> getTurn() const { return myBettingRound[GAME_STATE_TURN]; }
    std::shared_ptr<IBettingRound> getRiver() const { return myBettingRound[GAME_STATE_RIVER]; }
    std::shared_ptr<IBettingRound> getCurrentBettingRound() const { return myBettingRound[currentRound]; }

    IRankingStore* getRankingStore() const { return myRankingStore; }
    IPlayersStatisticsStore* getPlayersStatisticsStore() const { return myPlayersStatisticsStore; }
    IHandAuditStore* getHandAuditStore() const { return myHandAuditStore; }

    void setID(int theValue) { myID = theValue; }
    int getID() const { return myID; }

    void setStartQuantityPlayers(int theValue) { startQuantityPlayers = theValue; }
    int getStartQuantityPlayers() const { return startQuantityPlayers; }

    void setCurrentRound(GameState theValue) { currentRound = theValue; }
    GameState getCurrentRound() const { return currentRound; }
    GameState getRoundBeforePostRiver() const { return roundBeforePostRiver; }

    void setDealerPosition(int theValue) { dealerPosition = theValue; }
    int getDealerPosition() const { return dealerPosition; }

    void setSmallBlind(int theValue) { smallBlind = theValue; }
    int getSmallBlind() const { return smallBlind; }

    void setAllInCondition(bool theValue) { allInCondition = theValue; }
    bool getAllInCondition() const { return allInCondition; }

    void setStartCash(int theValue) { startCash = theValue; }
    int getStartCash() const { return startCash; }

    void setPreviousPlayerID(int theValue) { previousPlayerID = theValue; }
    int getPreviousPlayerID() const { return previousPlayerID; }

    void setLastActionPlayerID(unsigned theValue);

    unsigned getLastActionPlayerID() const { return lastActionPlayerID; }

    void setCardsShown(bool theValue) { cardsShown = theValue; }
    bool getCardsShown() const { return cardsShown; }

    void assignButtons();
    void setBlinds();

    void switchRounds();

    int getPreflopCallsNumber();
    int getPreflopRaisesNumber();
    int getFlopBetsOrRaisesNumber();
    int getTurnBetsOrRaisesNumber();
    int getRiverBetsOrRaisesNumber();

    std::vector<PlayerPosition> getRaisersPositions();
    std::vector<PlayerPosition> getCallersPositions();
    int getLastRaiserID();
    int getPreflopLastRaiserID();
    void setPreflopLastRaiserID(int id);
    int getFlopLastRaiserID();
    void setFlopLastRaiserID(int id);
    int getTurnLastRaiserID();
    void setTurnLastRaiserID(int id);

  protected:
    pkt::core::player::PlayerListIterator getSeatIt(unsigned) const;
    pkt::core::player::PlayerListIterator getActivePlayerIt(unsigned) const;
    pkt::core::player::PlayerListIterator getRunningPlayerIt(unsigned) const;

  private:
    std::shared_ptr<EngineFactory> myFactory;
    GameEvents* myEvents;
    std::shared_ptr<IBoard> myBoard;
    IRankingStore* myRankingStore;
    IPlayersStatisticsStore* myPlayersStatisticsStore;
    IHandAuditStore* myHandAuditStore;
    ILogger* myLogger;

    pkt::core::player::PlayerList seatsList;         // all player
    pkt::core::player::PlayerList activePlayerList;  // all player who are not out
    pkt::core::player::PlayerList runningPlayerList; // all player who are not folded, not all in and not out

    std::vector<std::shared_ptr<IBettingRound>> myBettingRound;

    int myID;
    int startQuantityPlayers;
    unsigned dealerPosition;
    unsigned smallBlindPosition;
    unsigned bigBlindPosition;
    GameState currentRound;
    GameState roundBeforePostRiver;
    int smallBlind;
    int startCash;

    int previousPlayerID;
    int preflopLastRaiserID;
    int flopLastRaiserID;
    int turnLastRaiserID;
    unsigned lastActionPlayerID;

    bool allInCondition;
    bool cardsShown;
};
} // namespace pkt::core
