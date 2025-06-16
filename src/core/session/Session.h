// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

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
