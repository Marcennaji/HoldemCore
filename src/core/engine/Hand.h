// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "EngineFactory.h"
#include "Player.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/interfaces/IBettingRound.h"
#include "core/interfaces/IBettingRoundStateFsm.h"
#include "core/interfaces/IBoard.h"
#include "core/interfaces/IHand.h"

#include <memory>
#include <vector>

namespace pkt::core
{

class Hand : public IHand
{
  public:
    Hand(const GameEvents&, std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard>,
         pkt::core::player::PlayerList seats, pkt::core::player::PlayerList runningPlayers, int handId,
         GameData gameData, StartData startData);
    ~Hand();

    void start();

    size_t dealBoardCards();
    void dealHoleCards(size_t lastArrayIndex);
    void initAndShuffleDeck();

    pkt::core::player::PlayerList getSeatsList() const { return mySeatsList; }
    pkt::core::player::PlayerList getRunningPlayersList() const { return myRunningPlayersList; }

    std::shared_ptr<IBoard> getBoard() const { return myBoard; }
    std::shared_ptr<IBettingRound> getPreflop() const { return myBettingRounds[GameStatePreflop]; }
    std::shared_ptr<IBettingRound> getFlop() const { return myBettingRounds[GameStateFlop]; }
    std::shared_ptr<IBettingRound> getTurn() const { return myBettingRounds[GameStateTurn]; }
    std::shared_ptr<IBettingRound> getRiver() const { return myBettingRounds[GameStateRiver]; }
    std::shared_ptr<IBettingRound> getCurrentBettingRound() const { return myBettingRounds[myCurrentRound]; }

    void setStartQuantityPlayers(int theValue) { myStartQuantityPlayers = theValue; }
    int getStartQuantityPlayers() const { return myStartQuantityPlayers; }

    void setCurrentRoundState(GameState theValue) { myCurrentRound = theValue; }
    GameState getCurrentRoundState() const { return myCurrentRound; }

    GameState getCurrentRoundStateFsm() const;

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

    void resolveHandConditions();

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

    const GameEvents& getEvents() const { return myEvents; }

  protected:
    pkt::core::player::PlayerListIterator getSeatIt(unsigned) const;
    pkt::core::player::PlayerListIterator getActivePlayerIt(unsigned) const;
    pkt::core::player::PlayerListIterator getRunningPlayerIt(unsigned) const;
    void updateRunningPlayersList();

  private:
    std::unique_ptr<IBettingRoundStateFsm> myCurrentStateFsm;
    std::shared_ptr<EngineFactory> myFactory;
    const GameEvents& myEvents;
    std::shared_ptr<IBoard> myBoard;

    pkt::core::player::PlayerList mySeatsList;          // all players
    pkt::core::player::PlayerList myRunningPlayersList; // all players who have not folded and are not all in

    std::vector<std::shared_ptr<IBettingRound>> myBettingRounds;

    std::vector<int> myCardsArray;

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
