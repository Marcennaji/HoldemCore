// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Game.h"

#include "EngineFactory.h"
#include "Exception.h"
#include "core/services/GlobalServices.h"
#include "model/EngineError.h"
#include "model/StartData.h"

#include <iostream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Game::Game(const GameEvents& events, std::shared_ptr<EngineFactory> factory, const PlayerList& playersList,
           const GameData& gameData, const StartData& startData, int gameId)
    : myFactory(factory), myEvents(events), myStartQuantityPlayers(startData.numberOfPlayers),
      myStartCash(gameData.startMoney), myStartSmallBlind(gameData.firstSmallBlind), myGameId(gameId),
      myCurrentSmallBlind(gameData.firstSmallBlind), myGameData(gameData), myStartData(startData)
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
    myCurrentBoard = myFactory->createBoard(myDealerPlayerId);

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

std::shared_ptr<IHand> Game::getCurrentHand()
{
    return myCurrentHand;
}

const std::shared_ptr<IHand> Game::getCurrentHand() const
{
    return myCurrentHand;
}

void Game::initHand()
{

    size_t i;
    PlayerListConstIterator itC;
    PlayerListIterator it;

    myCurrentHandId++;

    // set player action none
    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->setAction(PlayerActionNone);
    }

    myRunningPlayersList->clear();
    (*myRunningPlayersList) = (*mySeatsList);

    // create Hand
    myCurrentHand = myFactory->createHand(myFactory, myCurrentBoard, mySeatsList, myRunningPlayersList, myCurrentHandId,
                                          myGameData, myStartData);

    bool nextDealerFound = false;
    PlayerListConstIterator dealerPositionIt = myCurrentHand->getPlayerSeatFromId(myDealerPlayerId);
    if (dealerPositionIt == mySeatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }

    for (i = 0; i < mySeatsList->size(); i++)
    {

        ++dealerPositionIt;
        if (dealerPositionIt == mySeatsList->end())
        {
            dealerPositionIt = mySeatsList->begin();
        }

        itC = myCurrentHand->getPlayerSeatFromId((*dealerPositionIt)->getId());
        if (itC != mySeatsList->end())
        {
            nextDealerFound = true;
            myDealerPlayerId = (*itC)->getId();
            break;
        }
    }
    if (!nextDealerFound)
    {
        throw Exception(__FILE__, __LINE__, EngineError::NextDealerNotFound);
    }
}

void Game::startHand()
{
    if (myEvents.onNextBettingRoundInitializeGui)
    {
        myEvents.onNextBettingRoundInitializeGui();
    }

    myCurrentHand->start();
}

std::shared_ptr<Player> Game::getPlayerByUniqueId(unsigned id)
{
    std::shared_ptr<Player> tmpPlayer;
    PlayerListIterator i = getSeatsList()->begin();
    PlayerListIterator end = getSeatsList()->end();
    while (i != end)
    {
        if ((*i)->getId() == id)
        {
            tmpPlayer = *i;
            break;
        }
        ++i;
    }
    return tmpPlayer;
}

std::shared_ptr<Player> Game::getCurrentPlayer()
{
    std::shared_ptr<Player> tmpPlayer =
        getPlayerByUniqueId(getCurrentHand()->getCurrentBettingRound()->getCurrentPlayersTurnId());
    if (!tmpPlayer.get())
    {
        throw Exception(__FILE__, __LINE__, EngineError::CurrentPlayerNotFound);
    }
    return tmpPlayer;
}

} // namespace pkt::core
