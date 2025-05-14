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

#include "PlayerPosition.h"
#include "BoardInterface.h"
#include "BettingRoundinterface.h"
#include "infra/persistence/SqliteLogStore.h"

#include <ui/interfaces/guiinterface.h>


class HandInterface
{
public:

	virtual ~HandInterface();

	virtual void start() = 0;

	virtual PlayerList getSeatsList() const =0;
	virtual PlayerList getActivePlayerList() const =0;
	virtual PlayerList getRunningPlayerList() const =0;

	virtual std::shared_ptr<BoardInterface> getBoard() const =0;
	virtual std::shared_ptr<BettingRoundInterface> getPreflop() const =0;
	virtual std::shared_ptr<BettingRoundInterface> getFlop() const =0;
	virtual std::shared_ptr<BettingRoundInterface> getTurn() const =0;
	virtual std::shared_ptr<BettingRoundInterface> getRiver() const =0;
	virtual GuiInterface* getGuiInterface() const =0;
	virtual std::shared_ptr<BettingRoundInterface> getCurrentBettingRound() const =0;
	virtual SqliteLogStore *getLog() const =0;

	virtual void setID(int theValue) =0;
	virtual int getID() const =0;

	virtual void setStartQuantityPlayers(int theValue) =0;
	virtual int getStartQuantityPlayers() const =0;

	virtual void setCurrentRound(GameState theValue) =0;
	virtual GameState getCurrentRound() const =0;
	virtual GameState getRoundBeforePostRiver() const =0;

	virtual void setDealerPosition(int theValue) =0;
	virtual int getDealerPosition() const =0;

	virtual void setSmallBlind(int theValue) =0;
	virtual int getSmallBlind() const =0;

	virtual void setAllInCondition(bool theValue) =0;
	virtual bool getAllInCondition() const =0;

	virtual void setStartCash(int theValue) =0;
	virtual int getStartCash() const =0;

	virtual void setPreviousPlayerID(int theValue) =0;
	virtual int getPreviousPlayerID() const =0;

	virtual void setLastActionPlayerID( unsigned theValue ) =0;
	virtual unsigned getLastActionPlayerID() const =0;

	virtual void setCardsShown(bool theValue) =0;
	virtual bool getCardsShown() const =0;

	virtual void switchRounds() =0;

	virtual int getPreflopCallsNumber()=0;
	virtual int getPreflopRaisesNumber()=0;
	virtual int getFlopBetsOrRaisesNumber()=0;
	virtual int getTurnBetsOrRaisesNumber()=0;
	virtual int getRiverBetsOrRaisesNumber()=0;

	virtual std::vector<PlayerPosition> getRaisersPositions()=0;
	virtual std::vector<PlayerPosition> getCallersPositions()=0;
	virtual int getLastRaiserID()=0;
	virtual int getPreflopLastRaiserID()=0;
	virtual void setPreflopLastRaiserID(int id)=0;
	virtual int getFlopLastRaiserID()=0;
	virtual void setFlopLastRaiserID(int id)=0;
	virtual int getTurnLastRaiserID()=0;
	virtual void setTurnLastRaiserID(int id)=0;

protected:
	virtual PlayerListIterator getSeatIt(unsigned) const =0;
	virtual PlayerListIterator getActivePlayerIt(unsigned) const =0;
	virtual PlayerListIterator getRunningPlayerIt(unsigned) const =0;

	friend class Game;
	friend class BettingRound;
	friend class BettingRoundPreflop;
	friend class BettingRoundFlop;
	friend class BettingRoundTurn;
	friend class BettingRoundRiver;
	friend class BettingRoundPostRiver;
};

