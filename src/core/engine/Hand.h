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

#include "EngineFactory.h"
#include "BoardInterface.h"
#include "Player.h"
#include "HandInterface.h"
#include "BettingRoundInterface.h"
#include <core/interfaces/ILogger.h>

#include <vector>

class SqliteLogStore;
class GuiInterface;

class Hand : public HandInterface
{
public:
	Hand(ILogger * logger, std::shared_ptr<EngineFactory> f, GuiInterface*, std::shared_ptr<BoardInterface>, SqliteLogStore *, PlayerList, PlayerList, PlayerList, int, int, unsigned, int, int);
	~Hand();

	void start();

	PlayerList getSeatsList() const {
		return seatsList;
	}
	PlayerList getActivePlayerList() const {
		return activePlayerList;
	}
	PlayerList getRunningPlayerList() const {
		return runningPlayerList;
	}

	std::shared_ptr<BoardInterface> getBoard() const {
		return myBoard;
	}
	std::shared_ptr<BettingRoundInterface> getPreflop() const {
		return myBettingRound[GAME_STATE_PREFLOP];
	}
	std::shared_ptr<BettingRoundInterface> getFlop() const {
		return myBettingRound[GAME_STATE_FLOP];
	}
	std::shared_ptr<BettingRoundInterface> getTurn() const {
		return myBettingRound[GAME_STATE_TURN];
	}
	std::shared_ptr<BettingRoundInterface> getRiver() const {
		return myBettingRound[GAME_STATE_RIVER];
	}
	GuiInterface* getGuiInterface() const {
		return myGui;
	}
	std::shared_ptr<BettingRoundInterface> getCurrentBettingRound() const {
		return myBettingRound[currentRound];
	}

	SqliteLogStore *getLog() const {
		return myLog;
	}

	void setID(int theValue) {
		myID = theValue;
	}
	int getID() const {
		return myID;
	}

	void setStartQuantityPlayers(int theValue) {
		startQuantityPlayers = theValue;
	}
	int getStartQuantityPlayers() const {
		return startQuantityPlayers;
	}

	void setCurrentRound(GameState theValue) {
		currentRound = theValue;
	}
	GameState getCurrentRound() const {
		return currentRound;
	}
	GameState getRoundBeforePostRiver() const {
		return roundBeforePostRiver;
	}

	void setDealerPosition(int theValue) {
		dealerPosition = theValue;
	}
	int getDealerPosition() const {
		return dealerPosition;
	}

	void setSmallBlind(int theValue) {
		smallBlind = theValue;
	}
	int getSmallBlind() const {
		return smallBlind;
	}

	void setAllInCondition(bool theValue) {
		allInCondition = theValue;
	}
	bool getAllInCondition() const {
		return allInCondition;
	}

	void setStartCash(int theValue)	{
		startCash = theValue;
	}
	int getStartCash() const {
		return startCash;
	}

	void setPreviousPlayerID(int theValue) {
		previousPlayerID = theValue;
	}
	int getPreviousPlayerID() const {
		return previousPlayerID;
	}

	void setLastActionPlayerID ( unsigned theValue );

	unsigned getLastActionPlayerID() const {
		return lastActionPlayerID;
	}

	void setCardsShown(bool theValue) {
		cardsShown = theValue;
	}
	bool getCardsShown() const {
		return cardsShown;
	}

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

	PlayerListIterator getSeatIt(unsigned) const;
	PlayerListIterator getActivePlayerIt(unsigned) const;
	PlayerListIterator getRunningPlayerIt(unsigned) const;

private:

	std::shared_ptr<EngineFactory> myFactory;
	GuiInterface *myGui;
	std::shared_ptr<BoardInterface> myBoard;
	SqliteLogStore *myLog;
	ILogger * myLogger;

	PlayerList seatsList; // all player
	PlayerList activePlayerList; // all player who are not out
	PlayerList runningPlayerList; // all player who are not folded, not all in and not out

	std::vector<std::shared_ptr<BettingRoundInterface> > myBettingRound;

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



