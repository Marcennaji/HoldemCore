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
#ifndef STDSESSION_H
#define STDSESSION_H

#include <memory>
#include <string>

#include <core/engine/EngineDefs.h>
#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/interfaces/ILogger.h>

class IGui;
class Game;
class IRankingStore;
class IPlayersStatisticsStore;
class IHandAuditStore;
class QtToolsInterface;

class Session
{
  public:
    Session(ILogger*, IGui*, IRankingStore*, IPlayersStatisticsStore*, IHandAuditStore*);

    ~Session();

    bool init();

    void startGame(const GameData& gameData, const StartData& startData);

    std::shared_ptr<Game> getCurrentGame();

    IGui* getGui();

    IRankingStore* getRankingStore() { return myRankingStore; }
    IPlayersStatisticsStore* getPlayersStatisticsStore() { return myPlayersStatisticsStore; }
    IHandAuditStore* getHandAuditStore() { return myHandAuditStore; }

    unsigned getGameIdOfPlayer(unsigned playerId) const;
    unsigned getClientCurrentGameId() const;
    unsigned getClientUniquePlayerId() const;

  private:
    int currentGameNum;

    std::shared_ptr<Game> currentGame;
    IGui* myGui;
    ILogger* myLogger;
    IRankingStore* myRankingStore;
    IPlayersStatisticsStore* myPlayersStatisticsStore;
    IHandAuditStore* myHandAuditStore;
};

#endif
