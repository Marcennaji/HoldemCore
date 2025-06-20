// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"

#include <core/services/GlobalServices.h>
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

EngineFactory::EngineFactory(const GameEvents& events) : myEvents(events)
{
}

EngineFactory::~EngineFactory() = default;

std::shared_ptr<IHand> EngineFactory::createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard> b,
                                                 pkt::core::player::PlayerList sl, pkt::core::player::PlayerList apl,
                                                 pkt::core::player::PlayerList rpl, int id, GameData gd, StartData sd)
{
    return std::shared_ptr<IHand>(new Hand(myEvents, f, b, sl, apl, rpl, id, gd, sd));
}

std::shared_ptr<IBoard> EngineFactory::createBoard(unsigned dp)
{
    return std::shared_ptr<IBoard>(new Board(dp));
}

std::vector<std::shared_ptr<IBettingRound>> EngineFactory::createBettingRound(IHand* hi, unsigned dP, int sB)
{
    std::vector<std::shared_ptr<IBettingRound>> myBettingRound;

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundPreflop(myEvents, hi, dP, sB)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hi, dP, sB, GameStateFlop)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hi, dP, sB, GameStateTurn)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hi, dP, sB, GameStateRiver)));

    myBettingRound.push_back(std::shared_ptr<IBettingRound>(new BettingRoundPostRiver(myEvents, hi, dP, sB)));

    return myBettingRound;
}
} // namespace pkt::core
