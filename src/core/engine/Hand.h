// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "EngineFactory.h"
#include "Player.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/interfaces/IBettingRound.h"
#include "core/interfaces/IBoard.h"
#include "core/interfaces/IHand.h"

#include <vector>

namespace pkt::core
{

class Hand : public IHand
{
  public:
    Hand(const GameEvents&, std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard>, pkt::core::player::PlayerList,
         pkt::core::player::PlayerList, pkt::core::player::PlayerList, int handId, GameData gameData,
         StartData startData);
    ~Hand();

    void start();

    size_t dealBoardCards();
    void dealHoleCards(size_t lastArrayIndex);
    void initAndShuffleDeck();

    pkt::core::player::PlayerList getSeatsList() const { return mySeatsList; }
    pkt::core::player::PlayerList getActivePlayerList() const { return myActivePlayerList; }
    pkt::core::player::PlayerList getRunningPlayerList() const { return myRunningPlayerList; }

    std::shared_ptr<IBoard> getBoard() const { return myBoard; }
    std::shared_ptr<IBettingRound> getPreflop() const { return myBettingRound[GameStatePreflop]; }
    std::shared_ptr<IBettingRound> getFlop() const { return myBettingRound[GameStateFlop]; }
    std::shared_ptr<IBettingRound> getTurn() const { return myBettingRound[GameStateTurn]; }
    std::shared_ptr<IBettingRound> getRiver() const { return myBettingRound[GameStateRiver]; }
    std::shared_ptr<IBettingRound> getCurrentBettingRound() const { return myBettingRound[myCurrentRound]; }

    void setId(int theValue) { myID = theValue; }
    int getId() const { return myID; }

    void setStartQuantityPlayers(int theValue) { myStartQuantityPlayers = theValue; }
    int getStartQuantityPlayers() const { return myStartQuantityPlayers; }

    void setCurrentRound(GameState theValue) { myCurrentRound = theValue; }
    GameState getCurrentRound() const { return myCurrentRound; }
    GameState getRoundBeforePostRiver() const { return myRoundBeforePostRiver; }

    void setDealerPosition(int theValue) { myDealerPosition = theValue; }
    int getDealerPosition() const { return myDealerPosition; }

    void setSmallBlind(int theValue) { mySmallBlind = theValue; }
    int getSmallBlind() const { return mySmallBlind; }

    void setAllInCondition(bool theValue) { myAllInCondition = theValue; }
    bool getAllInCondition() const { return myAllInCondition; }

    void setStartCash(int theValue) { myStartCash = theValue; }
    int getStartCash() const { return myStartCash; }

    void setPreviousPlayerId(int theValue) { myPreviousPlayerId = theValue; }
    int getPreviousPlayerId() const { return myPreviousPlayerId; }

    void setLastActionPlayerId(unsigned theValue);

    unsigned getLastActionPlayerId() const { return myLastActionPlayerId; }

    void setCardsShown(bool theValue) { myCardsShown = theValue; }
    bool getCardsShown() const { return myCardsShown; }

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
    int getLastRaiserId();
    int getPreflopLastRaiserId();
    void setPreflopLastRaiserId(int id);
    int getFlopLastRaiserId();
    void setFlopLastRaiserId(int id);
    int getTurnLastRaiserId();
    void setTurnLastRaiserId(int id);

  protected:
    pkt::core::player::PlayerListIterator getSeatIt(unsigned) const;
    pkt::core::player::PlayerListIterator getActivePlayerIt(unsigned) const;
    pkt::core::player::PlayerListIterator getRunningPlayerIt(unsigned) const;

  private:
    std::shared_ptr<EngineFactory> myFactory;
    const GameEvents& myEvents;
    std::shared_ptr<IBoard> myBoard;

    pkt::core::player::PlayerList mySeatsList;         // all player
    pkt::core::player::PlayerList myActivePlayerList;  // all player who are not out
    pkt::core::player::PlayerList myRunningPlayerList; // all player who are not folded, not all in and not out

    std::vector<std::shared_ptr<IBettingRound>> myBettingRound;

    std::vector<int> myCardsArray;

    int myID;
    int myStartQuantityPlayers;
    unsigned myDealerPosition;
    unsigned mySmallBlindPosition;
    unsigned myBigBlindPosition;
    GameState myCurrentRound{GameStatePreflop};
    GameState myRoundBeforePostRiver{GameStatePreflop};
    int mySmallBlind;
    int myStartCash;

    int myPreviousPlayerId{-1};
    int myPreflopLastRaiserId;
    int myFlopLastRaiserId;
    int myTurnLastRaiserId;
    unsigned myLastActionPlayerId{0};

    bool myAllInCondition{false};
    bool myCardsShown{false};
};
} // namespace pkt::core
