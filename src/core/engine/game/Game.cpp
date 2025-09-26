// HoldemCore — Texas Hold'em simulator
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
    // Reset the board to preflop state (clear all community cards)
    myCurrentBoard->setBoardCards(BoardCards());
    
    myActingPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>(*mySeatsList);

    // Determine dealer for THIS hand before creating the Hand.
    // For the very first hand, use the dealer provided at Game construction.
    // For subsequent hands, rotate to the next eligible dealer first.
    if (myCurrentHand) {
        findNextDealer();
    }

    StartData handStart = myStartData;
    handStart.startDealerPlayerId = myDealerPlayerId;

    myCurrentHand = myEngineFactory->createHand(myEngineFactory, myCurrentBoard, mySeatsList, myActingPlayersList,
                                                myGameData, handStart);
    myCurrentHand->initialize();
    myCurrentHand->runGameLoop();
}

void Game::findNextDealer()
{
    bool nextDealerFound = false;
    // Work off the stable seats list; dealer must be among seated players
    auto dealerPos = getPlayerListIteratorById(mySeatsList, myDealerPlayerId);

    if (dealerPos == mySeatsList->end())
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);

    for (size_t i = 0; i < mySeatsList->size(); ++i) {
        ++dealerPos;
        if (dealerPos == mySeatsList->end())
            dealerPos = mySeatsList->begin();

        auto playerIt = getPlayerListIteratorById(mySeatsList, (*dealerPos)->getId());
        if (playerIt != mySeatsList->end()) {
            myDealerPlayerId = (*playerIt)->getId();
            nextDealerFound = true;
            break;
        }
    }

    if (!nextDealerFound)
        throw Exception(__FILE__, __LINE__, EngineError::NextDealerNotFound);
}

void Game::handlePlayerAction(const PlayerAction& action)
{
    if (myCurrentHand) {
        myCurrentHand->handlePlayerAction(action);
    }
}

} // namespace pkt::core
