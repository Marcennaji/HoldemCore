// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"

#include <core/services/GlobalServices.h>
#include "core/engine/BoardFsm.h"
#include "core/engine/deprecated/BettingRoundPostRiver.h"
#include "core/engine/deprecated/BettingRoundPreflop.h"
#include "core/engine/deprecated/Hand.h"
#include "deprecated/Board.h"

#include "core/player/deprecated/HumanPlayer.h"
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
                                                 pkt::core::player::PlayerList actingPlayers, GameData gd, StartData sd)
{
    return std::shared_ptr<IHand>(new Hand(myEvents, f, b, seats, actingPlayers, gd, sd));
}
std::shared_ptr<HandFsm> EngineFactory::createHandFsm(std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard> b,
                                                      pkt::core::player::PlayerFsmList seats,
                                                      pkt::core::player::PlayerFsmList actingPlayers, GameData gd,
                                                      StartData sd)
{
    return std::shared_ptr<HandFsm>(new HandFsm(myEvents, f, b, seats, actingPlayers, gd, sd));
}

std::shared_ptr<IBoard> EngineFactory::createBoard(unsigned dealerPosition)
{
    return std::shared_ptr<IBoard>(new Board(dealerPosition));
}
std::shared_ptr<IBoard> EngineFactory::createBoardFsm(unsigned dealerPosition)
{
    return std::shared_ptr<IBoard>(new BoardFsm(dealerPosition, myEvents));
}
std::vector<std::shared_ptr<IBettingRound>> EngineFactory::createAllBettingRounds(IHand* hand, unsigned dealerPosition,
                                                                                  int smallBlind)
{
    std::vector<std::shared_ptr<IBettingRound>> myBettingRound;

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRoundPreflop(myEvents, hand, dealerPosition, smallBlind)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hand, dealerPosition, smallBlind, Flop)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hand, dealerPosition, smallBlind, Turn)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRound(myEvents, hand, dealerPosition, smallBlind, River)));

    myBettingRound.push_back(
        std::shared_ptr<IBettingRound>(new BettingRoundPostRiver(myEvents, hand, dealerPosition, smallBlind)));

    return myBettingRound;
}
} // namespace pkt::core
