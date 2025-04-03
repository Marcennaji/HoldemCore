/*****************************************************************************
 * PokerTraining - THNL training software, based on the PokerTH GUI          *
 * Copyright (C) 2013 Marc Ennaji                                            *
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

#ifndef BOARDINTERFACE_H
#define BOARDINTERFACE_H

#include <engine/engine.h>

class HandInterface;

class BoardInterface
{

public:

	virtual ~BoardInterface();
//
	virtual void setPlayerLists(PlayerList, PlayerList, PlayerList) =0;
//
	virtual void setCards(int* theValue) =0;
	virtual void getCards(int* theValue) =0;
//
	virtual int getPot() const=0;
	virtual void setPot(int theValue) =0;
	virtual int getSets() const=0;
	virtual void setSets(int theValue) =0;

	virtual void setAllInCondition(bool theValue) =0;
	virtual void setLastActionPlayerID(unsigned theValue) =0;
//
	virtual void collectSets() =0;
	virtual void collectPot() =0;

	virtual void distributePot() =0;
	virtual void determinePlayerNeedToShowCards() =0;

	virtual std::list<unsigned> getWinners() const =0;
	virtual void setWinners(const std::list<unsigned> &winners) =0;

	virtual std::list<unsigned> getPlayerNeedToShowCards() const =0;
	virtual void setPlayerNeedToShowCards(const std::list<unsigned> &playerNeedToShowCards) =0;

};

#endif
