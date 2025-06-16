// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "EngineDefs.h"
#include "model/GameData.h"
#include "model/StartData.h"

#include <list>
#include <memory>
#include <string>

namespace pkt::core
{

struct GameEvents;
class IRankingStore;
class IPlayersStatisticsStore;
class IHandAuditStore;
class IHand;
class IBoard;
class EngineFactory;

class Game
{

  public:
    Game(GameEvents*, std::shared_ptr<EngineFactory> factory, const pkt::core::player::PlayerList& playerList,
         const GameData& gameData, const StartData& startData, int gameId, IRankingStore* rankingStore,
         IPlayersStatisticsStore* playersStatisticsStore, IHandAuditStore* handAuditStore);

    ~Game();

    void initHand();
    void startHand();

    std::shared_ptr<IHand> getCurrentHand();
    const std::shared_ptr<IHand> getCurrentHand() const;

    pkt::core::player::PlayerList getSeatsList() const { return seatsList; }
    pkt::core::player::PlayerList getActivePlayerList() const { return activePlayerList; }
    pkt::core::player::PlayerList getRunningPlayerList() const { return runningPlayerList; }

    void setStartQuantityPlayers(int theValue) { startQuantityPlayers = theValue; }
    int getStartQuantityPlayers() const { return startQuantityPlayers; }

    void setStartSmallBlind(int theValue) { startSmallBlind = theValue; }
    int getStartSmallBlind() const { return startSmallBlind; }

    void setStartCash(int theValue) { startCash = theValue; }
    int getStartCash() const { return startCash; }

    int getGameID() const { return myGameID; }

    void setCurrentSmallBlind(int theValue) { currentSmallBlind = theValue; }
    int getCurrentSmallBlind() const { return currentSmallBlind; }

    void setCurrentHandID(int theValue) { currentHandID = theValue; }
    int getCurrentHandID() const { return currentHandID; }

    unsigned getDealerPosition() const { return dealerPosition; }

    void replaceDealer(unsigned oldDealer, unsigned newDealer)
    {
        if (dealerPosition == oldDealer)
            dealerPosition = newDealer;
    }

    std::shared_ptr<pkt::core::player::Player> getPlayerByUniqueId(unsigned id);
    std::shared_ptr<pkt::core::player::Player> getCurrentPlayer();

    void raiseBlinds();

  private:
    std::shared_ptr<EngineFactory> myFactory;

    GameEvents* myEvents;
    IRankingStore* myRankingStore;
    IPlayersStatisticsStore* myPlayersStatisticsStore;
    IHandAuditStore* myHandAuditStore;
    std::shared_ptr<IHand> currentHand;
    std::shared_ptr<IBoard> currentBoard;

    pkt::core::player::PlayerList seatsList;
    pkt::core::player::PlayerList activePlayerList;  // used seats
    pkt::core::player::PlayerList runningPlayerList; // nonfolded and nonallin active players

    // start variables
    int startQuantityPlayers;
    int startCash;
    int startSmallBlind;
    int myGameID;

    // running variables
    int currentSmallBlind;
    int currentHandID;
    unsigned dealerPosition;
    int lastHandBlindsRaised;
    int lastTimeBlindsRaised;
    const GameData myGameData;
    std::list<int> blindsList;
};

} // namespace pkt::core
