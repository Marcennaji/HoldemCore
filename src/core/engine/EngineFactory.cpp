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

#include "EngineFactory.h"

#include <core/interfaces/ILogger.h>
#include "BettingRoundPostRiver.h"
#include "BettingRoundPreflop.h"
#include "Board.h"
#include "Hand.h"
#include "Randomizer.h"

#include "HumanPlayer.h"
#include "strategy/LooseAggressiveBotStrategy.h"
#include "strategy/ManiacBotStrategy.h"
#include "strategy/TightAggressiveBotStrategy.h"

namespace pkt::core
{

EngineFactory::EngineFactory(GameEvents* events, ILogger* logger) : myLogger(logger), myEvents(events)
{
    assert(myLogger != nullptr);
    assert(myEvents != nullptr);
}

EngineFactory::~EngineFactory()
{
}

std::shared_ptr<IHand> EngineFactory::createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard> b,
                                                 IRankingStore* l, IPlayersStatisticsStore* ps, IHandAuditStore* hs,
                                                 pkt::core::player::PlayerList sl, pkt::core::player::PlayerList apl,
                                                 pkt::core::player::PlayerList rpl, int id, int sP, int dP, int sB,
                                                 int sC)
{
    return std::shared_ptr<IHand>(new Hand(myEvents, myLogger, f, b, l, ps, hs, sl, apl, rpl, id, sP, dP, sB, sC));
}

std::shared_ptr<IBoard> EngineFactory::createBoard(unsigned dp)
{
    return std::shared_ptr<IBoard>(new Board(dp));
}

std::vector<std::shared_ptr<IBettingRound>> EngineFactory::createBettingRound(IHand* hi, unsigned dP, int sB)
{
    std::vector<std::shared_ptr<IBettingRound>> myBettingRound;

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundPreflop(myEvents, myLogger, hi, dP, sB)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, myLogger, hi, dP, sB, GAME_STATE_FLOP)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, myLogger, hi, dP, sB, GAME_STATE_TURN)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, myLogger, hi, dP, sB, GAME_STATE_RIVER)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundPostRiver(myEvents, myLogger, hi, dP, sB)));

    return myBettingRound;
}
} // namespace pkt::core
