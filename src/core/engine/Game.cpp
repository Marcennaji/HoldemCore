// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Game.h"

#include "EngineFactory.h"
#include "Exception.h"
#include "core/interfaces/persistence/IHandAuditStore.h"
#include "core/interfaces/persistence/IRankingStore.h"
#include "model/EngineError.h"
#include "model/StartData.h"

#include <iostream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Game::Game(const GameEvents& events, std::shared_ptr<EngineFactory> factory, const PlayerList& playerList,
           const GameData& gameData, const StartData& startData, int gameId, IRankingStore* l,
           IPlayersStatisticsStore* ps, IHandAuditStore* handAuditStore)
    : myFactory(factory), myEvents(events), myRankingStore(l), myPlayersStatisticsStore(ps),
      myHandAuditStore(handAuditStore), startQuantityPlayers(startData.numberOfPlayers), startCash(gameData.startMoney),
      startSmallBlind(gameData.firstSmallBlind), myGameID(gameId), currentSmallBlind(gameData.firstSmallBlind),
      currentHandID(0), dealerPosition(0), lastHandBlindsRaised(1), lastTimeBlindsRaised(0), myGameData(gameData)
{
    dealerPosition = startData.startDealerPlayerId;

    // determine dealer position
    PlayerListConstIterator player_i = playerList->begin();
    PlayerListConstIterator player_end = playerList->end();

    while (player_i != player_end)
    {
        if ((*player_i)->getID() == dealerPosition)
            break;
        ++player_i;
    }
    if (player_i == player_end)
        throw Exception(__FILE__, __LINE__, EngineError::DEALER_NOT_FOUND);

    // create board
    currentBoard = myFactory->createBoard(dealerPosition);

    // create player lists
    seatsList.reset(new std::list<std::shared_ptr<Player>>);
    activePlayerList.reset(new std::list<std::shared_ptr<Player>>);
    runningPlayerList.reset(new std::list<std::shared_ptr<Player>>);

    (*runningPlayerList) = (*playerList);
    (*activePlayerList) = (*playerList);
    (*seatsList) = (*playerList);

    currentBoard->setPlayerLists(seatsList, activePlayerList, runningPlayerList);

    myRankingStore->updateRankingPlayedGames(activePlayerList);
}

Game::~Game()
{
    runningPlayerList->clear();
    activePlayerList->clear();
    seatsList->clear();
}

std::shared_ptr<IHand> Game::getCurrentHand()
{
    return currentHand;
}

const std::shared_ptr<IHand> Game::getCurrentHand() const
{
    return currentHand;
}

void Game::initHand()
{

    size_t i;
    PlayerListConstIterator it_c;
    PlayerListIterator it;

    currentHandID++;

    // set player action none
    for (it = seatsList->begin(); it != seatsList->end(); ++it)
    {
        (*it)->setAction(PLAYER_ACTION_NONE);
    }

    // set player with empty cash inactive
    it = activePlayerList->begin();
    while (it != activePlayerList->end())
    {

        if ((*it)->getCash() == 0)
        {
            (*it)->setActiveStatus(false);
            it = activePlayerList->erase(it);
        }
        else
        {
            ++it;
        }
    }

    runningPlayerList->clear();
    (*runningPlayerList) = (*activePlayerList);

    // create Hand
    currentHand = myFactory->createHand(myFactory, currentBoard, myRankingStore, myPlayersStatisticsStore,
                                        myHandAuditStore, seatsList, activePlayerList, runningPlayerList, currentHandID,
                                        startQuantityPlayers, dealerPosition, currentSmallBlind, startCash);

    bool nextDealerFound = false;
    PlayerListConstIterator dealerPositionIt = currentHand->getSeatIt(dealerPosition);
    if (dealerPositionIt == seatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SEAT_NOT_FOUND);
    }

    for (i = 0; i < seatsList->size(); i++)
    {

        ++dealerPositionIt;
        if (dealerPositionIt == seatsList->end())
            dealerPositionIt = seatsList->begin();

        it_c = currentHand->getActivePlayerIt((*dealerPositionIt)->getID());
        if (it_c != activePlayerList->end())
        {
            nextDealerFound = true;
            dealerPosition = (*it_c)->getID();
            break;
        }
    }
    if (!nextDealerFound)
    {
        throw Exception(__FILE__, __LINE__, EngineError::NEXT_DEALER_NOT_FOUND);
    }
}

void Game::startHand()
{
    if (myEvents.onNextBettingRoundInitializeGui)
        myEvents.onNextBettingRoundInitializeGui();

    currentHand->start();
}

std::shared_ptr<Player> Game::getPlayerByUniqueId(unsigned id)
{
    std::shared_ptr<Player> tmpPlayer;
    PlayerListIterator i = getSeatsList()->begin();
    PlayerListIterator end = getSeatsList()->end();
    while (i != end)
    {
        if ((*i)->getID() == id)
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
        throw Exception(__FILE__, __LINE__, EngineError::CURRENT_PLAYER_NOT_FOUND);
    return tmpPlayer;
}

} // namespace pkt::core
