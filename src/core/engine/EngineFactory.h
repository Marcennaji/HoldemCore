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

#ifndef ENGINEFACTORY_H
#define ENGINEFACTORY_H

#include "EngineFactory.h"

#include "HandInterface.h"
#include "BoardInterface.h"
#include <core/interfaces/persistence/ILogger.h>


#include <core/player/Player.h>

#include <memory>
#include <vector>

class EngineFactory 
{
public:
	EngineFactory(ILogger *);
	~EngineFactory();

	virtual std::shared_ptr<HandInterface> createHand(std::shared_ptr<EngineFactory> f, GuiInterface *g, std::shared_ptr<BoardInterface> b, Log *l, PlayerList sl, PlayerList apl, PlayerList rpl, int id, int sP, int dP, int sB,int sC);
	virtual std::shared_ptr<BoardInterface> createBoard(unsigned dp);
	virtual std::vector<std::shared_ptr<BettingRoundInterface> > createBettingRound(HandInterface *hi, unsigned dP, int sB);

private:
	ILogger * myLogger;
};

#endif
