/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#pragma once

#include <memory>
#include <string>

#include <core/engine/EngineDefs.h>
#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/interfaces/ILogger.h>
#include "core/engine/GameEvents.h"
namespace pkt::core
{
class Game;
class IRankingStore;
class IPlayersStatisticsStore;
class IHandAuditStore;
class QtToolsInterface;

class Session
{
  public:
    Session(GameEvents* events, ILogger*, IRankingStore*, IPlayersStatisticsStore*, IHandAuditStore*);

    ~Session();

    void startGame(const GameData& gameData, const StartData& startData);

    std::shared_ptr<Game> getCurrentGame();

    IRankingStore* getRankingStore() { return myRankingStore; }
    IPlayersStatisticsStore* getPlayersStatisticsStore() { return myPlayersStatisticsStore; }
    IHandAuditStore* getHandAuditStore() { return myHandAuditStore; }

  private:
    int currentGameNum;

    std::shared_ptr<Game> currentGame;
    GameEvents* myEvents;
    ILogger* myLogger;
    IRankingStore* myRankingStore;
    IPlayersStatisticsStore* myPlayersStatisticsStore;
    IHandAuditStore* myHandAuditStore;
};

} // namespace pkt::core
