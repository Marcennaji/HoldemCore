// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

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
