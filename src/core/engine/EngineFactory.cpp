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

#include <core/interfaces/ILogger.h>
#include "BettingRoundflop.h"
#include "BettingRoundpostriver.h"
#include "BettingRoundpreflop.h"
#include "BettingRoundriver.h"
#include "BettingRoundturn.h"
#include "Randomizer.h"
#include "board.h"
#include "hand.h"

#include "HumanPlayer.h"
#include "LooseAggressivePlayer.h"
#include "ManiacPlayer.h"
#include "TightAgressivePlayer.h"

EngineFactory::EngineFactory(ILogger* logger) : myLogger(logger)
{
}

EngineFactory::~EngineFactory()
{
}

std::shared_ptr<IHand> EngineFactory::createHand(std::shared_ptr<EngineFactory> f, GuiInterface* g,
                                                 std::shared_ptr<IBoard> b, IRankingStore* l,
                                                 IPlayersStatisticsStore* ps, IHandAuditStore* hs, PlayerList sl,
                                                 PlayerList apl, PlayerList rpl, int id, int sP, int dP, int sB, int sC)
{
    return std::shared_ptr<IHand>(new Hand(myLogger, f, g, b, l, ps, hs, sl, apl, rpl, id, sP, dP, sB, sC));
}

std::shared_ptr<IBoard> EngineFactory::createBoard(unsigned dp)
{
    return std::shared_ptr<IBoard>(new Board(dp));
}

std::vector<std::shared_ptr<IBettingRound>> EngineFactory::createBettingRound(IHand* hi, unsigned dP, int sB)
{
    std::vector<std::shared_ptr<IBettingRound>> myBettingRound;

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundPreflop(myLogger, hi, dP, sB)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundFlop(myLogger, hi, dP, sB)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundTurn(myLogger, hi, dP, sB)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundRiver(myLogger, hi, dP, sB)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundPostRiver(myLogger, hi, dP, sB)));

    return myBettingRound;
}
