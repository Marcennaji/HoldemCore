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

#include "Session.h"
#include <core/engine/Game.h>
#include <core/engine/GameEvents.h>

#include <core/engine/EngineFactory.h>
#include <core/engine/Randomizer.h>
#include <core/interfaces/ILogger.h>
#include <core/player/BotPlayer.h>
#include <core/player/HumanPlayer.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/player/strategy/ManiacBotStrategy.h>
#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <core/player/strategy/UltraTightBotStrategy.h>
#include "core/player/DefaultPlayerFactory.h"
#include "core/player/Helpers.h"
#include "core/player/strategy/StrategyAssigner.h"

#include <algorithm>
#include <random>
#include <sstream>

namespace pkt::core
{

using namespace std;

Session::Session(GameEvents* events, ILogger* logger, IRankingStore* rs, IPlayersStatisticsStore* ps,
                 IHandAuditStore* ha)
    : myLogger(logger), currentGameNum(0), myRankingStore(rs), myPlayersStatisticsStore(ps), myHandAuditStore(ha),
      myEvents(events)
{
}

Session::~Session()
{
}

void Session::startGame(const GameData& gameData, const StartData& startData)
{
    currentGame.reset();
    currentGameNum++;

    if (myEvents && myEvents->onHideHoleCards)
        myEvents->onHideHoleCards();
    if (myEvents && myEvents->onInitializeGui)
        myEvents->onInitializeGui(gameData.guiSpeed);

    auto engineFactory = std::make_shared<EngineFactory>(myEvents, myLogger);

    auto strategyAssigner = std::make_unique<StrategyAssigner>(gameData.tableProfile, startData.numberOfPlayers - 1);

    auto playerFactory = std::make_unique<DefaultPlayerFactory>(myEvents, myHandAuditStore, myPlayersStatisticsStore,
                                                                strategyAssigner.get());

    auto playerList = std::make_shared<std::list<std::shared_ptr<Player>>>();

    playerList->push_back(playerFactory->createHumanPlayer(0, HumanPlayerName, gameData.startMoney));

    for (int i = 1; i < startData.numberOfPlayers; ++i)
    {
        playerList->push_back(playerFactory->createBotPlayer(i, gameData.tableProfile));
    }

    // Shuffle bots but keep human first
    shufflePlayers(*playerList, 0);

    // Reassign player IDs after shuffle
    int id = 0;
    for (auto& p : *playerList)
    {
        p->setID(id++);
        p->setIsSessionActive(true);
    }

    currentGame = std::make_unique<Game>(myEvents, engineFactory, playerList, gameData, startData, currentGameNum,
                                         myRankingStore, myPlayersStatisticsStore, myHandAuditStore);

    currentGame->initHand();
    currentGame->startHand();
}

std::shared_ptr<Game> Session::getCurrentGame()
{
    return currentGame;
}
} // namespace pkt::core
