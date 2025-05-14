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

#include "enginefactory.h"

#include "hand.h"
#include "board.h"
#include "BettingRoundpreflop.h"
#include "BettingRoundflop.h"
#include "BettingRoundturn.h"
#include "BettingRoundriver.h"
#include "BettingRoundpostriver.h"
#include "tools.h"
#include <core/interfaces/persistence/ILogger.h>

#include "TightAgressivePlayer.h"
#include "LooseAggressivePlayer.h"
#include "ManiacPlayer.h"
#include "HumanPlayer.h"


EngineFactory::EngineFactory(ILogger * logger)
	: myLogger(logger)
{
}


EngineFactory::~EngineFactory()
{
}


std::shared_ptr<HandInterface>
EngineFactory::createHand(std::shared_ptr<EngineFactory> f, GuiInterface *g, std::shared_ptr<BoardInterface> b, SqliteLogStore *l, PlayerList sl, PlayerList apl, PlayerList rpl, int id, int sP, int dP, int sB,int sC)
{
	return std::shared_ptr<HandInterface>(new Hand(myLogger, f, g, b, l, sl, apl, rpl, id, sP, dP, sB, sC));
}

std::shared_ptr<BoardInterface>
EngineFactory::createBoard(unsigned dp)
{
	return std::shared_ptr<BoardInterface>(new Board(dp));
}

std::vector<std::shared_ptr<BettingRoundInterface> >
EngineFactory::createBettingRound(HandInterface *hi, unsigned dP, int sB)
{
	std::vector<std::shared_ptr<BettingRoundInterface> > myBettingRound;

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundPreflop(myLogger, hi, dP, sB)));

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundFlop(myLogger,hi, dP, sB)));

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundTurn(myLogger,hi, dP, sB)));

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundRiver(myLogger,hi, dP, sB)));

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundPostRiver(myLogger,hi, dP, sB)));

	return myBettingRound;

}
