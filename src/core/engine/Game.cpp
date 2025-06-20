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

Game::Game(const GameEvents& events, std::shared_ptr<EngineFactory> factory, const PlayerList& playerList,
           const GameData& gameData, const StartData& startData, int gameId)
    : myFactory(factory), myEvents(events), myStartQuantityPlayers(startData.numberOfPlayers),
      myStartCash(gameData.startMoney), myStartSmallBlind(gameData.firstSmallBlind), myGameId(gameId),
      myCurrentSmallBlind(gameData.firstSmallBlind), myCurrentHandId(0), myDealerPosition(0), myLastHandBlindsRaised(1),
      myLastTimeBlindsRaised(0), myGameData(gameData), myStartData(startData)
{
    myDealerPosition = startData.startDealerPlayerId;

    // determine dealer position
    PlayerListConstIterator player_i = playerList->begin();
    PlayerListConstIterator player_end = playerList->end();

    while (player_i != player_end)
    {
        if ((*player_i)->getId() == myDealerPosition)
            break;
        ++player_i;
    }
    if (player_i == player_end)
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);

    // create board
    myCurrentBoard = myFactory->createBoard(myDealerPosition);

    // create player lists
    mySeatsList.reset(new std::list<std::shared_ptr<Player>>);
    myActivePlayerList.reset(new std::list<std::shared_ptr<Player>>);
    myRunningPlayerList.reset(new std::list<std::shared_ptr<Player>>);

    (*myRunningPlayerList) = (*playerList);
    (*myActivePlayerList) = (*playerList);
    (*mySeatsList) = (*playerList);

    myCurrentBoard->setPlayerLists(mySeatsList, myActivePlayerList, myRunningPlayerList);

    GlobalServices::instance().rankingStore()->updateRankingPlayedGames(myActivePlayerList);
}

Game::~Game()
{
    myRunningPlayerList->clear();
    myActivePlayerList->clear();
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
    PlayerListConstIterator it_c;
    PlayerListIterator it;

    myCurrentHandId++;

    // set player action none
    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->setAction(PlayerActionNone);
    }

    // set player with empty cash inactive
    it = myActivePlayerList->begin();
    while (it != myActivePlayerList->end())
    {

        if ((*it)->getCash() == 0)
        {
            (*it)->setActiveStatus(false);
            it = myActivePlayerList->erase(it);
        }
        else
        {
            ++it;
        }
    }

    myRunningPlayerList->clear();
    (*myRunningPlayerList) = (*myActivePlayerList);

    // create Hand
    myCurrentHand = myFactory->createHand(myFactory, myCurrentBoard, mySeatsList, myActivePlayerList,
                                          myRunningPlayerList, myCurrentHandId, myGameData, myStartData);

    bool nextDealerFound = false;
    PlayerListConstIterator dealerPositionIt = myCurrentHand->getSeatIt(myDealerPosition);
    if (dealerPositionIt == mySeatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }

    for (i = 0; i < mySeatsList->size(); i++)
    {

        ++dealerPositionIt;
        if (dealerPositionIt == mySeatsList->end())
            dealerPositionIt = mySeatsList->begin();

        it_c = myCurrentHand->getActivePlayerIt((*dealerPositionIt)->getId());
        if (it_c != myActivePlayerList->end())
        {
            nextDealerFound = true;
            myDealerPosition = (*it_c)->getId();
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
        myEvents.onNextBettingRoundInitializeGui();

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
        throw Exception(__FILE__, __LINE__, EngineError::CurrentPlayerNotFound);
    return tmpPlayer;
}

} // namespace pkt::core
