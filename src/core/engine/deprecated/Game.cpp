// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Game.h"

#include "EngineFactory.h"
#include "Exception.h"
#include "core/engine/deprecated/Helpers.h"
#include "core/services/GlobalServices.h"
#include "model/EngineError.h"
#include "model/StartData.h"

#include <iostream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Game::Game(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
           PlayerList seatsList, unsigned dealerId, const GameData& gameData, const StartData& startData)
    : myEngineFactory(factory), myEvents(events), myCurrentBoard(board), mySeatsList(seatsList),
      myDealerPlayerId(dealerId), myGameData(gameData), myStartData(startData)
{
    // Acting players list starts identical to seats list
    myActingPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>(*mySeatsList);

    // Validate dealer exists
    auto it = getPlayerListIteratorById(mySeatsList, dealerId);
    if (it == mySeatsList->end())
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);
}

Game::~Game()
{
    myActingPlayersList->clear();
    mySeatsList->clear();
}

void Game::startNewHand()
{
    resetPlayerActions();
    resetActingPlayers();
    createNewHand();
    findNextDealer();
    myCurrentHand->start();
}

void Game::resetPlayerActions()
{
    for (auto& player : *mySeatsList)
        player->setLastAction({player->getId(), ActionType::None, 0});
}

void Game::resetActingPlayers()
{
    myActingPlayersList->clear();
    *myActingPlayersList = *mySeatsList;
}

void Game::createNewHand()
{
    myCurrentHand = myEngineFactory->createHand(myEngineFactory, myCurrentBoard, mySeatsList, myActingPlayersList,
                                                myGameData, myStartData);
}

void Game::findNextDealer()
{
    bool nextDealerFound = false;
    auto dealerPos = getPlayerListIteratorById(myCurrentHand->getSeatsList(), myDealerPlayerId);

    if (dealerPos == mySeatsList->end())
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);

    for (size_t i = 0; i < mySeatsList->size(); ++i)
    {
        ++dealerPos;
        if (dealerPos == mySeatsList->end())
            dealerPos = mySeatsList->begin();

        auto playerIt = getPlayerListIteratorById(myCurrentHand->getSeatsList(), (*dealerPos)->getId());
        if (playerIt != mySeatsList->end())
        {
            myDealerPlayerId = (*playerIt)->getId();
            nextDealerFound = true;
            break;
        }
    }

    if (!nextDealerFound)
        throw Exception(__FILE__, __LINE__, EngineError::NextDealerNotFound);
}

} // namespace pkt::core
