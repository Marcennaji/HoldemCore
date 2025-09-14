// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "BoardFsm.h"

#include "core/interfaces/IHand.h"

#include <core/services/GlobalServices.h>
#include "Exception.h"
#include "PotFsm.h"
#include "core/engine/HandFsm.h"
#include "core/player/PlayerFsm.h"
#include "model/EngineError.h"

#include <algorithm>

namespace pkt::core
{
using namespace pkt::core::player;

BoardFsm::BoardFsm(unsigned dp, const GameEvents& events) : myDealerPlayerId(dp), myEvents(events)
{
    myCards[0] = myCards[1] = myCards[2] = myCards[3] = myCards[4] = 0;
}

BoardFsm::~BoardFsm()
{
}

void BoardFsm::setSeatsListFsm(PlayerFsmList seats)
{
    mySeatsList = seats;
}
void BoardFsm::setActingPlayersListFsm(PlayerFsmList actingPlayers)
{
    myActingPlayersList = actingPlayers;
}

void BoardFsm::distributePot(HandFsm& hand)
{
    int totalPot = 0;

    for (auto& player : *hand.getSeatsList())
    {
        totalPot += player->getCurrentHandActions().getHandTotalBetAmount();
    }
    PotFsm pot(totalPot, mySeatsList, myDealerPlayerId);
    pot.distribute();
    myWinners = pot.getWinners();

    if (myEvents.onHandCompleted)
        myEvents.onHandCompleted(myWinners, totalPot);
}

void BoardFsm::determinePlayerNeedToShowCards()
{

    myPlayerNeedToShowCards.clear();

    // in All In Condition everybody have to show the cards
    if (myAllInCondition)
    {
        for (auto itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
        {
            if ((*itC)->getLastAction().type != ActionType::Fold)
            {
                myPlayerNeedToShowCards.push_back((*itC)->getId());
            }
        }
    }

    else
    {

        // all winners have to show their cards

        std::list<std::pair<int, int>> level;

        PlayerFsmListConstIterator lastActionPlayerIt;
        PlayerFsmListConstIterator itC;

        // search lastActionPlayer
        for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
        {
            if ((*itC)->getId() == myLastActionPlayerId && (*itC)->getLastAction().type != ActionType::Fold)
            {
                lastActionPlayerIt = itC;
                break;
            }
        }

        if (itC == mySeatsList->end())
        {
            for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
            {
                if ((*itC)->getLastAction().type != ActionType::Fold)
                {
                    lastActionPlayerIt = itC;
                    break;
                }
            }
        }

        // the player who has done the last action has to show his cards first
        myPlayerNeedToShowCards.push_back((*lastActionPlayerIt)->getId());

        std::pair<int, int> levelTmp;
        // get position and cardsValue of the player who show his cards first
        levelTmp.first = (*lastActionPlayerIt)->getHandRanking();
        levelTmp.second = (*lastActionPlayerIt)->getCashAtHandStart() - (*lastActionPlayerIt)->getCash();

        level.push_back(levelTmp);

        std::list<std::pair<int, int>>::iterator levelIt;
        std::list<std::pair<int, int>>::iterator nextLevelIt;

        itC = lastActionPlayerIt;
        ++itC;

        for (unsigned i = 0; i < mySeatsList->size(); i++)
        {

            if (itC == mySeatsList->end())
            {
                itC = mySeatsList->begin();
            }

            if ((*itC)->getLastAction().type != ActionType::Fold)
            {

                for (levelIt = level.begin(); levelIt != level.end(); ++levelIt)
                {
                    if ((*itC)->getHandRanking() > (*levelIt).first)
                    {
                        nextLevelIt = levelIt;
                        ++nextLevelIt;
                        if (nextLevelIt == level.end())
                        {
                            myPlayerNeedToShowCards.push_back((*itC)->getId());
                            levelTmp.first = (*itC)->getHandRanking();
                            levelTmp.second = (*itC)->getCashAtHandStart() - (*itC)->getCash();
                            level.push_back(levelTmp);
                            break;
                        }
                    }
                    else
                    {
                        if ((*itC)->getHandRanking() == (*levelIt).first)
                        {
                            nextLevelIt = levelIt;
                            ++nextLevelIt;

                            if (nextLevelIt == level.end() ||
                                (*itC)->getCashAtHandStart() - (*itC)->getCash() > (*nextLevelIt).second)
                            {
                                myPlayerNeedToShowCards.push_back((*itC)->getId());
                                if ((*itC)->getCashAtHandStart() - (*itC)->getCash() > (*levelIt).second)
                                {
                                    (*levelIt).second = (*itC)->getCashAtHandStart() - (*itC)->getCash();
                                }
                            }
                            break;
                        }
                        else
                        {
                            if ((*itC)->getCashAtHandStart() - (*itC)->getCash() > (*levelIt).second)
                            {
                                myPlayerNeedToShowCards.push_back((*itC)->getId());
                                levelTmp.first = (*itC)->getHandRanking();
                                levelTmp.second = (*itC)->getCashAtHandStart() - (*itC)->getCash();

                                level.insert(levelIt, levelTmp);

                                break;
                            }
                        }
                    }
                }
            }

            ++itC;
        }

        level.clear();
    }

    // sort and unique the list
    myPlayerNeedToShowCards.sort();
    myPlayerNeedToShowCards.unique();
}
void BoardFsm::setCards(int* theValue)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        myCards[i] = theValue[i];
    }
}
void BoardFsm::getCards(int* theValue)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        theValue[i] = myCards[i];
    }
}

void BoardFsm::setAllInCondition(bool theValue)
{
    myAllInCondition = theValue;
}
void BoardFsm::setLastActionPlayerId(unsigned theValue)
{
    myLastActionPlayerId = theValue;
}

std::list<unsigned> BoardFsm::getWinners() const
{
    return myWinners;
}
void BoardFsm::setWinners(const std::list<unsigned>& w)
{
    myWinners = w;
}

std::list<unsigned> BoardFsm::getPlayerNeedToShowCards() const
{
    return myPlayerNeedToShowCards;
}
void BoardFsm::setPlayerNeedToShowCards(const std::list<unsigned>& p)
{
    myPlayerNeedToShowCards = p;
}
int BoardFsm::getPot(const HandFsm& hand) const
{
    int totalPot = 0;

    for (auto& player : *hand.getSeatsList())
    {
        totalPot += player->getCurrentHandActions().getHandTotalBetAmount();
    }
    return totalPot;
}
int BoardFsm::getSets(const HandFsm& hand) const
{
    int total = 0;
    GameState currentRound = hand.getGameState();

    for (auto& player : *hand.getSeatsList())
    {
        total += player->getCurrentHandActions().getRoundTotalBetAmount(currentRound);
    }
    return total;
}
} // namespace pkt::core
