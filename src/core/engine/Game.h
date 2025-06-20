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
class IHand;
class IBoard;
class EngineFactory;

class Game
{

  public:
    Game(const GameEvents&, std::shared_ptr<EngineFactory> factory, const pkt::core::player::PlayerList& playerList,
         const GameData& gameData, const StartData& startData, int gameId);

    ~Game();

    void initHand();
    void startHand();

    std::shared_ptr<IHand> getCurrentHand();
    const std::shared_ptr<IHand> getCurrentHand() const;

    pkt::core::player::PlayerList getSeatsList() const { return mySeatsList; }
    pkt::core::player::PlayerList getActivePlayerList() const { return myActivePlayerList; }
    pkt::core::player::PlayerList getRunningPlayerList() const { return myRunningPlayerList; }

    void setStartQuantityPlayers(int theValue) { myStartQuantityPlayers = theValue; }
    int getStartQuantityPlayers() const { return myStartQuantityPlayers; }

    void setStartSmallBlind(int theValue) { myStartSmallBlind = theValue; }
    int getStartSmallBlind() const { return myStartSmallBlind; }

    void setStartCash(int theValue) { myStartCash = theValue; }
    int getStartCash() const { return myStartCash; }

    int getGameId() const { return myGameId; }

    void setCurrentSmallBlind(int theValue) { myCurrentSmallBlind = theValue; }
    int getCurrentSmallBlind() const { return myCurrentSmallBlind; }

    void setCurrentHandID(int theValue) { myCurrentHandId = theValue; }
    int getCurrentHandID() const { return myCurrentHandId; }

    unsigned getDealerPosition() const { return myDealerPosition; }

    void replaceDealer(unsigned oldDealer, unsigned newDealer)
    {
        if (myDealerPosition == oldDealer)
        {
            myDealerPosition = newDealer;
        }
    }

    std::shared_ptr<pkt::core::player::Player> getPlayerByUniqueId(unsigned id);
    std::shared_ptr<pkt::core::player::Player> getCurrentPlayer();

    void raiseBlinds();

  private:
    std::shared_ptr<EngineFactory> myFactory;

    const GameEvents& myEvents;
    std::shared_ptr<IHand> myCurrentHand;
    std::shared_ptr<IBoard> myCurrentBoard;

    pkt::core::player::PlayerList mySeatsList;
    pkt::core::player::PlayerList myActivePlayerList;  // used seats
    pkt::core::player::PlayerList myRunningPlayerList; // nonfolded and nonallin active players

    // start variables
    int myStartQuantityPlayers;
    int myStartCash;
    int myStartSmallBlind;
    int myGameId;

    // running variables
    int myCurrentSmallBlind;
    int myCurrentHandId{0};
    unsigned myDealerPosition{0};
    int myLastHandBlindsRaised{1};
    int myLastTimeBlindsRaised{0};
    GameData myGameData;
    StartData myStartData;
    std::list<int> myBlindsList;
};

} // namespace pkt::core
