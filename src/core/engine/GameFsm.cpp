// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "GameFsm.h"

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

GameFsm::GameFsm(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
                 PlayerFsmList seatsList, unsigned dealerId, const GameData& gameData, const StartData& startData)
    : myEngineFactory(factory), myEvents(events), myCurrentBoard(board), mySeatsList(seatsList),
      myDealerPlayerId(dealerId), myGameData(gameData), myStartData(startData)
{
    // Acting players list starts identical to seats list
    myActingPlayersList = std::make_shared<std::list<std::shared_ptr<PlayerFsm>>>(*mySeatsList);

    // Validate dealer exists
    auto it = getPlayerFsmListIteratorById(mySeatsList, dealerId);
    if (it == mySeatsList->end())
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);
}

void GameFsm::startNewHand()
{
    myCurrentHand = myEngineFactory->createHandFsm(myEngineFactory, myCurrentBoard, mySeatsList, myActingPlayersList,
                                                   myGameData, myStartData);
    findNextDealer();
    myCurrentHand->initialize();
    myCurrentHand->runGameLoop();
}

void GameFsm::findNextDealer()
{
    bool nextDealerFound = false;
    auto dealerPos = getPlayerFsmListIteratorById(myCurrentHand->getSeatsList(), myDealerPlayerId);

    if (dealerPos == mySeatsList->end())
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);

    for (size_t i = 0; i < mySeatsList->size(); ++i)
    {
        ++dealerPos;
        if (dealerPos == mySeatsList->end())
            dealerPos = mySeatsList->begin();

        auto playerIt = getPlayerFsmListIteratorById(myCurrentHand->getSeatsList(), (*dealerPos)->getId());
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
