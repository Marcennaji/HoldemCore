// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "SessionFsm.h"
#include <core/engine/GameEvents.h>
#include <core/engine/GameFsm.h>

#include <core/engine/EngineFactory.h>

#include <core/player/deprecated/BotPlayer.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/player/strategy/ManiacBotStrategy.h>
#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <core/player/strategy/UltraTightBotStrategy.h>
#include <core/services/GlobalServices.h>
#include "core/engine/deprecated/Helpers.h"
#include "core/player/DefaultPlayerFactory.h"
#include "core/player/deprecated/HumanPlayer.h"
#include "core/player/strategy/StrategyAssigner.h"

#include <algorithm>
#include <random>
#include <sstream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

SessionFsm::SessionFsm(const GameEvents& events) : myEvents(events)
{
}

SessionFsm::~SessionFsm() = default;

pkt::core::player::PlayerFsmList SessionFsm::createPlayersList(DefaultPlayerFactory& playerFactory, int numberOfPlayers,
                                                               unsigned startMoney, const TableProfile& tableProfile)
{
    auto playersList = std::make_shared<std::list<std::shared_ptr<PlayerFsm>>>();

    for (int i = 0; i < numberOfPlayers; ++i)
        playersList->push_back(playerFactory.createPlayerFsm(i, tableProfile, startMoney));

    return playersList;
}

void SessionFsm::startGame(const GameData& gameData, const StartData& startData)
{
    myCurrentGame.reset();

    if (myEvents.onGameInitialized)
        myEvents.onGameInitialized(gameData.guiSpeed);

    auto engineFactory = std::make_shared<EngineFactory>(myEvents);
    auto strategyAssigner = std::make_unique<StrategyAssigner>(gameData.tableProfile, startData.numberOfPlayers - 1);
    auto playerFactory = std::make_unique<DefaultPlayerFactory>(myEvents, strategyAssigner.get());

    auto playersList =
        createPlayersList(*playerFactory, startData.numberOfPlayers, gameData.startMoney, gameData.tableProfile);

    // Board is fully prepared here
    auto board = engineFactory->createBoardFsm(startData.startDealerPlayerId);
    board->setSeatsListFsm(playersList);
    board->setActingPlayersListFsm(playersList);

    myCurrentGame = std::make_unique<GameFsm>(myEvents, engineFactory, board, playersList,
                                              startData.startDealerPlayerId, gameData, startData);

    myCurrentGame->startNewHand();
}

} // namespace pkt::core
