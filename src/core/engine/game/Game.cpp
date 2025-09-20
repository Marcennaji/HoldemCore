// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Game.h"

#include "EngineFactory.h"
#include "Exception.h"
#include "core/player/Helpers.h"
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
    if (!factory || !board || !seatsList || seatsList->empty())
        throw Exception(__FILE__, __LINE__, EngineError::MissingParameter);

    // Acting players list starts identical to seats list
    myActingPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>(*mySeatsList);

    // Validate dealer exists
    auto it = getPlayerListIteratorById(mySeatsList, dealerId);
    if (it == mySeatsList->end())
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);
}

void Game::startNewHand()
{
    myCurrentHand = myEngineFactory->createHand(myEngineFactory, myCurrentBoard, mySeatsList, myActingPlayersList,
                                                myGameData, myStartData);
    findNextDealer();
    myCurrentHand->initialize();
    myCurrentHand->runGameLoop();
}

void Game::findNextDealer()
{
    bool nextDealerFound = false;
    auto dealerPos = getPlayerListIteratorById(myCurrentHand->getSeatsList(), myDealerPlayerId);

    // Current dealer must exist in the seats list
    if (dealerPos == myCurrentHand->getSeatsList()->end())
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);

    for (size_t i = 0; i < mySeatsList->size(); ++i)
    {
        ++dealerPos;
        if (dealerPos == myCurrentHand->getSeatsList()->end())
            dealerPos = myCurrentHand->getSeatsList()->begin();

        auto playerIt = getPlayerListIteratorById(myCurrentHand->getSeatsList(), (*dealerPos)->getId());
        if (playerIt != myCurrentHand->getSeatsList()->end())
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
