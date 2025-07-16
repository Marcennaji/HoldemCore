// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Session.h"
#include <core/engine/Game.h>
#include <core/engine/GameEvents.h>

#include <core/engine/EngineFactory.h>
#include <core/engine/Randomizer.h>
#include <core/player/BotPlayer.h>
#include <core/player/HumanPlayer.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/player/strategy/ManiacBotStrategy.h>
#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <core/player/strategy/UltraTightBotStrategy.h>
#include <core/services/GlobalServices.h>
#include "core/player/DefaultPlayerFactory.h"
#include "core/player/Helpers.h"
#include "core/player/strategy/StrategyAssigner.h"

#include <algorithm>
#include <random>
#include <sstream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Session::Session(const GameEvents& events) : myEvents(events)
{
}

Session::~Session() = default;

void Session::startGame(const GameData& gameData, const StartData& startData)
{
    myCurrentGame.reset();
    myCurrentGameNum++;

    if (myEvents.onGameInitialized)
    {
        myEvents.onGameInitialized(gameData.guiSpeed);
    }

    auto engineFactory = std::make_shared<EngineFactory>(myEvents);

    auto strategyAssigner = std::make_unique<StrategyAssigner>(gameData.tableProfile, startData.numberOfPlayers - 1);

    auto playerFactory = std::make_unique<DefaultPlayerFactory>(myEvents, strategyAssigner.get());

    auto playersList = std::make_shared<std::list<std::shared_ptr<Player>>>();

    playersList->push_back(playerFactory->createHumanPlayer(0, gameData.startMoney));

    for (int i = 1; i < startData.numberOfPlayers; ++i)
    {
        playersList->push_back(playerFactory->createBotPlayer(i, gameData.tableProfile, gameData.startMoney));
    }

    // Shuffle bots but keep human first
    shufflePlayers(*playersList, 0);

    // Reassign player IDs after shuffle
    int id = 0;
    for (auto& p : *playersList)
    {
        p->setId(id++);
    }

    myCurrentGame = std::make_unique<Game>(myEvents, engineFactory, playersList, gameData, startData, myCurrentGameNum);

    myCurrentGame->initHand();
    myCurrentGame->startHand();
}

std::shared_ptr<Game> Session::getCurrentGame()
{
    return myCurrentGame;
}
} // namespace pkt::core
