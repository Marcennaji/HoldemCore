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
                                                 pkt::core::player::PlayerList seats,
                                                 pkt::core::player::PlayerList runningPlayers, int id, GameData gd,
                                                 StartData sd)
{
    return std::shared_ptr<IHand>(new Hand(myEvents, f, b, seats, runningPlayers, id, gd, sd));
}

std::shared_ptr<IBoard> EngineFactory::createBoard(unsigned dealerPosition)
{
    return std::shared_ptr<IBoard>(new Board(dealerPosition));
}

std::vector<std::shared_ptr<IBettingRound>> EngineFactory::createAllBettingRounds(IHand* hand, unsigned dealerPosition,
                                                                                  int smallBlind)
{
    std::vector<std::shared_ptr<IBettingRound>> myBettingRound;

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRoundPreflop(myEvents, hand, dealerPosition, smallBlind)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hand, dealerPosition, smallBlind, GameStateFlop)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hand, dealerPosition, smallBlind, GameStateTurn)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hand, dealerPosition, smallBlind, GameStateRiver)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRoundPostRiver(myEvents, hand, dealerPosition, smallBlind)));

    return myBettingRound;
}
} // namespace pkt::core
