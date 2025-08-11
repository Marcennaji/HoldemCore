// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Game.h"

#include "EngineFactory.h"
#include "Exception.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"
#include "model/EngineError.h"
#include "model/StartData.h"

#include <iostream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Game::Game(const GameEvents& events, std::shared_ptr<EngineFactory> factory, const PlayerList& playersList,
           const GameData& gameData, const StartData& startData)
    : myEngineFactory(factory), myEvents(events), myGameData(gameData), myStartData(startData)
{
    myDealerPlayerId = startData.startDealerPlayerId;

    // determine dealer position
    PlayerListConstIterator playerI = playersList->begin();
    PlayerListConstIterator playerEnd = playersList->end();

    while (playerI != playerEnd)
    {
        if ((*playerI)->getId() == myDealerPlayerId)
        {
            break;
        }
        ++playerI;
    }
    if (playerI == playerEnd)
    {
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);
    }

    // create board
    myCurrentBoard = myEngineFactory->createBoard(myDealerPlayerId);

    // create players lists
    mySeatsList.reset(new std::list<std::shared_ptr<Player>>);
    myRunningPlayersList.reset(new std::list<std::shared_ptr<Player>>);

    // Create a deep copy of playersList for mySeatsList
    for (const auto& player : *playersList)
    {
        mySeatsList->push_back(player);
    }

    // Create a separate deep copy of playersList for myRunningPlayersList
    for (const auto& player : *playersList)
    {
        myRunningPlayersList->push_back(player);
    }

    myCurrentBoard->setSeatsList(mySeatsList);
    myCurrentBoard->setRunningPlayersList(myRunningPlayersList);

    GlobalServices::instance().rankingStore()->updateRankingPlayedGames(mySeatsList);
}

Game::~Game()
{
    myRunningPlayersList->clear();
    mySeatsList->clear();
    mySeatsList->clear();
}

void Game::startNewHand()
{
    for (auto& player : *mySeatsList)
    {
        player->setAction(ActionType::None);
    }

    myRunningPlayersList->clear();
    (*myRunningPlayersList) = (*mySeatsList);

    myCurrentHand = myEngineFactory->createHand(myEngineFactory, myCurrentBoard, mySeatsList, myRunningPlayersList,
                                                myGameData, myStartData);

    bool nextDealerFound = false;

    auto dealerPositionIt = getPlayerListIteratorById(myCurrentHand->getSeatsList(), myDealerPlayerId);
    if (dealerPositionIt == mySeatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }

    for (int i = 0; i < mySeatsList->size(); i++)
    {

        ++dealerPositionIt;
        if (dealerPositionIt == mySeatsList->end())
        {
            dealerPositionIt = mySeatsList->begin();
        }
        auto playerIterator = getPlayerListIteratorById(myCurrentHand->getSeatsList(), (*dealerPositionIt)->getId());

        if (playerIterator != mySeatsList->end())
        {
            nextDealerFound = true;
            myDealerPlayerId = (*playerIterator)->getId();
            break;
        }
    }
    if (!nextDealerFound)
    {
        throw Exception(__FILE__, __LINE__, EngineError::NextDealerNotFound);
    }

    myCurrentHand->start();
}

} // namespace pkt::core
